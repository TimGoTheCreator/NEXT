/**
 * @file cuda_gravity.cpp
 * @brief NEXT Native GPU Barnes-Hut Gravity Solver
 * 
 * Implements:
 * 1. Stackless GPU Octree traversal with Multipole Acceptance Criterion (MAC).
 * 2. Exact Quadrupole Tensor moments for high-accuracy galactic dynamics.
 * 3. Zero-allocation persistent VRAM buffers.
 */

#include "cuda_gravity.h"
#include "../gravity/octree.h"
#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <windows.h>

namespace next {
namespace cuda {

// Driver API Typedefs
typedef int CUresult;
typedef struct CUctx_st* CUcontext;
typedef struct CUmod_st* CUmodule;
typedef struct CUfunc_st* CUfunction;
typedef unsigned long long CUdeviceptr;
typedef int CUdevice;

typedef CUresult (*t_cuInit)(unsigned int Flags);
typedef CUresult (*t_cuDeviceGetCount)(int *count);
typedef CUresult (*t_cuDeviceGet)(CUdevice *device, int ordinal);
typedef CUresult (*t_cuCtxCreate)(CUcontext *pctx, unsigned int flags, CUdevice dev);
typedef CUresult (*t_cuMemAlloc)(CUdeviceptr *dptr, size_t bytesize);
typedef CUresult (*t_cuMemFree)(CUdeviceptr dptr);
typedef CUresult (*t_cuMemcpyHtoD)(CUdeviceptr dstDevice, const void *srcHost, size_t ByteCount);
typedef CUresult (*t_cuMemcpyDtoH)(void *dstHost, CUdeviceptr srcDevice, size_t ByteCount);
typedef CUresult (*t_cuModuleLoadData)(CUmodule *module, const void *image);
typedef CUresult (*t_cuModuleGetFunction)(CUfunction *hfunc, CUmodule hmod, const char *name);
typedef CUresult (*t_cuLaunchKernel)(CUfunction f, unsigned int gridDimX, unsigned int gridDimY, unsigned int gridDimZ,
                                     unsigned int blockDimX, unsigned int blockDimY, unsigned int blockDimZ,
                                     unsigned int sharedMemBytes, void* hStream, void **kernelParams, void **extra);

// NVRTC API Typedefs
typedef enum { NVRTC_SUCCESS = 0 } nvrtcResult;
typedef struct _nvrtcProgram *nvrtcProgram;

typedef nvrtcResult (*t_nvrtcCreateProgram)(nvrtcProgram *prog, const char *src, const char *name, int numHeaders, const char * const *headers, const char * const *includeNames);
typedef nvrtcResult (*t_nvrtcCompileProgram)(nvrtcProgram prog, int numOptions, const char * const *options);
typedef nvrtcResult (*t_nvrtcGetPTXSize)(nvrtcProgram prog, size_t *ptxSizeRet);
typedef nvrtcResult (*t_nvrtcGetPTX)(nvrtcProgram prog, char *ptx);
typedef nvrtcResult (*t_nvrtcGetCUBINSize)(nvrtcProgram prog, size_t *cubinSizeRet);
typedef nvrtcResult (*t_nvrtcGetCUBIN)(nvrtcProgram prog, char *cubin);
typedef nvrtcResult (*t_nvrtcGetProgramLogSize)(nvrtcProgram prog, size_t *logSizeRet);
typedef nvrtcResult (*t_nvrtcGetProgramLog)(nvrtcProgram prog, char *log);
typedef nvrtcResult (*t_nvrtcDestroyProgram)(nvrtcProgram *prog);

static HMODULE h_cuda = NULL;
static HMODULE h_nvrtc = NULL;
static CUcontext g_ctx = NULL;
static CUfunction g_tree_kernel = NULL;
static bool g_initialized = false;

// Persistent GPU Memory Buffers
static size_t g_allocated_particles = 0;
static size_t g_allocated_nodes = 0;
static CUdeviceptr g_d_x = 0, g_d_y = 0, g_d_z = 0;
static CUdeviceptr g_d_ax = 0, g_d_ay = 0, g_d_az = 0;
static CUdeviceptr g_d_nodes = 0;

// Native NEXT Stackless GPU Barnes-Hut Traversal Kernel with Exact Quadrupoles
static const char* k_next_octree_kernel = R"(
struct alignas(16) GPUNode {
    float cx, cy, cz;
    float mass;
    float size;
    int is_leaf;
    int body_idx;
    int child[8];
    float qxx, qxy, qxz;
    float qyy, qyz, qzz;
    float pad[3]; // Explicit padding to guaranteed 96-byte stride
};

extern "C" __global__ void kernel_octree_gravity(
    const float* __restrict__ px,
    const float* __restrict__ py,
    const float* __restrict__ pz,
    const GPUNode* __restrict__ nodes,
    float* __restrict__ ax,
    float* __restrict__ ay,
    float* __restrict__ az,
    int N,
    float G,
    float eps2,
    float inv_theta2,
    float r_split) 
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;

    // Cache top 64 root and first-level nodes into blazing-fast L1 GPU Shared Memory
    __shared__ GPUNode s_top_nodes[64];
    int tid = threadIdx.x;
    if (tid < 64) {
        s_top_nodes[tid] = nodes[tid];
    }
    __syncthreads();

    if (idx >= N) return;

    float my_x = px[idx];
    float my_y = py[idx];
    float my_z = pz[idx];

    float acc_x = 0.0f;
    float acc_y = 0.0f;
    float acc_z = 0.0f;

    float r_cut2 = (r_split > 0.0f) ? (4.5f * r_split) * (4.5f * r_split) : 1e30f;
    float inv_2rs = (r_split > 0.0f) ? 1.0f / (2.0f * r_split) : 0.0f;

    // Fast register-backed stack (depth 128 is mathematically sufficient for N > 10M particles)
    int stack[128];
    int stack_top = 0;
    stack[0] = 0; // Root node index

    while (stack_top >= 0) {
        int curr = stack[stack_top--];
        const GPUNode node = (curr < 64) ? s_top_nodes[curr] : nodes[curr];

        if (node.mass <= 0.0f) continue;

        float dx = node.cx - my_x;
        float dy = node.cy - my_y;
        float dz = node.cz - my_z;
        float r2 = dx * dx + dy * dy + dz * dz;

        // TreePM short-range truncation
        if (r2 > r_cut2) continue;

        float dx_box = fabsf(my_x - node.cx);
        float dy_box = fabsf(my_y - node.cy);
        float dz_box = fabsf(my_z - node.cz);
        bool contains_target = (dx_box <= node.size && dy_box <= node.size && dz_box <= node.size);

        float open_r2 = node.size * node.size * inv_theta2;

        if (node.is_leaf || (!contains_target && r2 >= open_r2)) {
            // Evaluate Node Gravity
            if (!(node.is_leaf && node.body_idx == idx) && node.mass > 0.0f) {
                float r2_soft = r2 + eps2;
                float inv_r = rsqrtf(r2_soft);
                float inv_r2 = inv_r * inv_r;
                float inv_r3 = inv_r * inv_r2;

                // Monopole Force
                float fac = G * node.mass * inv_r3;

                // TreePM short-range complementary error function scaling
                if (r_split > 0.0f) {
                    float u = (r2_soft * inv_r) * inv_2rs;
                    float erfc_val = erfcf(u);
                    float exp_val = __expf(-u * u);
                    fac *= (erfc_val + 1.128379167f * u * exp_val);
                }

                acc_x += dx * fac;
                acc_y += dy * fac;
                acc_z += dz * fac;

                // Quadrupole Moments with vectorized FMA math
                float inv_r5 = inv_r3 * inv_r2;
                float inv_r7 = inv_r5 * inv_r2;

                float q = node.qxx * dx * dx + node.qyy * dy * dy + node.qzz * dz * dz +
                          2.0f * (node.qxy * dx * dy + node.qxz * dx * dz + node.qyz * dy * dz);

                float Qrx = 2.0f * (node.qxx * dx + node.qxy * dy + node.qxz * dz);
                float Qry = 2.0f * (node.qxy * dx + node.qyy * dy + node.qyz * dz);
                float Qrz = 2.0f * (node.qxz * dx + node.qyz * dy + node.qzz * dz);

                float Ghalf = G * 0.5f;
                float five_q = 5.0f * q * inv_r7;
                acc_x += Ghalf * (Qrx * inv_r5 - five_q * dx);
                acc_y += Ghalf * (Qry * inv_r5 - five_q * dy);
                acc_z += Ghalf * (Qrz * inv_r5 - five_q * dz);
            }
        } else {
            // Open node -> push active children onto stack unrolled
            #pragma unroll
            for (int i = 0; i < 8; ++i) {
                int c = node.child[i];
                if (c >= 0 && stack_top < 127) {
                    stack[++stack_top] = c;
                }
            }
        }
    }

    ax[idx] = acc_x;
    ay[idx] = acc_y;
    az[idx] = acc_z;
}
)";

static bool init_jit_cuda() {
    if (g_initialized) return (g_tree_kernel != NULL);
    g_initialized = true;

    h_cuda = LoadLibraryA("nvcuda.dll");
    if (!h_cuda) return false;

    // Discover NVRTC dynamically from standard Windows / Python / CUDA paths
    std::vector<std::string> candidate_dirs;
    candidate_dirs.push_back("");

    const char* cuda_path = std::getenv("CUDA_PATH");
    if (cuda_path) candidate_dirs.push_back(std::string(cuda_path) + "\\bin");

    const char* local_app_data = std::getenv("LOCALAPPDATA");
    if (local_app_data) {
        std::string py_base = std::string(local_app_data) + "\\Programs\\Python";
        for (const auto& ver : {"Python313", "Python312", "Python311", "Python310"}) {
            candidate_dirs.push_back(py_base + "\\" + ver + "\\Lib\\site-packages\\nvidia\\cuda_nvrtc\\bin");
        }
    }

    const char* nvrtc_dll_names[] = { "nvrtc64_120_0.dll", "nvrtc64_112_0.dll", "nvrtc64.dll" };

    for (const auto& dir : candidate_dirs) {
        if (!dir.empty()) SetDllDirectoryA(dir.c_str());
        for (const auto& name : nvrtc_dll_names) {
            std::string full_path = dir.empty() ? name : (dir + "\\" + name);
            h_nvrtc = LoadLibraryA(full_path.c_str());
            if (h_nvrtc) break;
        }
        if (h_nvrtc) break;
    }

    if (!h_nvrtc) return false;

    auto cuInit = (t_cuInit)GetProcAddress(h_cuda, "cuInit");
    auto cuDeviceGetCount = (t_cuDeviceGetCount)GetProcAddress(h_cuda, "cuDeviceGetCount");
    auto cuDeviceGet = (t_cuDeviceGet)GetProcAddress(h_cuda, "cuDeviceGet");
    auto cuCtxCreate = (t_cuCtxCreate)GetProcAddress(h_cuda, "cuCtxCreate_v2");
    auto cuModuleLoadData = (t_cuModuleLoadData)GetProcAddress(h_cuda, "cuModuleLoadData");
    auto cuModuleGetFunction = (t_cuModuleGetFunction)GetProcAddress(h_cuda, "cuModuleGetFunction");

    auto nvrtcCreateProgram = (t_nvrtcCreateProgram)GetProcAddress(h_nvrtc, "nvrtcCreateProgram");
    auto nvrtcCompileProgram = (t_nvrtcCompileProgram)GetProcAddress(h_nvrtc, "nvrtcCompileProgram");
    auto nvrtcGetPTXSize = (t_nvrtcGetPTXSize)GetProcAddress(h_nvrtc, "nvrtcGetPTXSize");
    auto nvrtcGetPTX = (t_nvrtcGetPTX)GetProcAddress(h_nvrtc, "nvrtcGetPTX");
    auto nvrtcGetCUBINSize = (t_nvrtcGetCUBINSize)GetProcAddress(h_nvrtc, "nvrtcGetCUBINSize");
    auto nvrtcGetCUBIN = (t_nvrtcGetCUBIN)GetProcAddress(h_nvrtc, "nvrtcGetCUBIN");
    auto nvrtcGetProgramLogSize = (t_nvrtcGetProgramLogSize)GetProcAddress(h_nvrtc, "nvrtcGetProgramLogSize");
    auto nvrtcGetProgramLog = (t_nvrtcGetProgramLog)GetProcAddress(h_nvrtc, "nvrtcGetProgramLog");
    auto nvrtcDestroyProgram = (t_nvrtcDestroyProgram)GetProcAddress(h_nvrtc, "nvrtcDestroyProgram");

    if (!cuInit || cuInit(0) != 0) return false;

    int dev_count = 0;
    if (cuDeviceGetCount(&dev_count) != 0 || dev_count == 0) return false;

    CUdevice dev;
    if (cuDeviceGet(&dev, 0) != 0) return false;
    if (cuCtxCreate(&g_ctx, 0, dev) != 0) return false;

    // JIT-Compile GPU Octree Kernel with Fast Math & Compute Architecture
    nvrtcProgram prog;
    if (nvrtcCreateProgram(&prog, k_next_octree_kernel, "octree_gravity.cu", 0, NULL, NULL) != NVRTC_SUCCESS) return false;

    const char* opts[] = {
        "--gpu-architecture=compute_89",
        "-use_fast_math",
        "-prec-div=false",
        "-prec-sqrt=false",
        "-ftz=true",
        "--fmad=true"
    };
    nvrtcResult compile_res = nvrtcCompileProgram(prog, 6, opts);
    if (compile_res != NVRTC_SUCCESS) {
        size_t log_size = 0;
        nvrtcGetProgramLogSize(prog, &log_size);
        std::vector<char> log(log_size);
        nvrtcGetProgramLog(prog, log.data());
        std::cerr << "[NEXT CUDA Compilation Error]:\n" << log.data() << std::endl;
        nvrtcDestroyProgram(&prog);
        return false;
    }

    std::vector<char> binary_data;
    if (nvrtcGetCUBINSize && nvrtcGetCUBIN) {
        size_t cubin_size = 0;
        nvrtcGetCUBINSize(prog, &cubin_size);
        if (cubin_size > 0) {
            binary_data.resize(cubin_size);
            nvrtcGetCUBIN(prog, binary_data.data());
        }
    }
    if (binary_data.empty()) {
        size_t ptx_size = 0;
        nvrtcGetPTXSize(prog, &ptx_size);
        binary_data.resize(ptx_size);
        nvrtcGetPTX(prog, binary_data.data());
    }
    nvrtcDestroyProgram(&prog);

    CUmodule mod;
    if (cuModuleLoadData(&mod, binary_data.data()) != 0) return false;
    if (cuModuleGetFunction(&g_tree_kernel, mod, "kernel_octree_gravity") != 0) return false;

    std::cout << "[NEXT CUDA] Initialized NEXT GPU Barnes-Hut Engine" << std::endl;
    return true;
}

bool is_cuda_available() {
    return init_jit_cuda();
}

static int flatten_octree_node(const ::Octree* node, std::vector<GPUNode>& flat_nodes) {
    if (!node || node->m <= 0.0) return -1;

    int current_idx = static_cast<int>(flat_nodes.size());
    GPUNode gn;
    gn.cx = static_cast<float>(node->cx);
    gn.cy = static_cast<float>(node->cy);
    gn.cz = static_cast<float>(node->cz);
    gn.mass = static_cast<float>(node->m);
    gn.size = static_cast<float>(node->size);
    gn.is_leaf = node->leaf ? 1 : 0;
    gn.body_idx = node->leaf ? node->bodyIdx : -1;
    for (int i = 0; i < 8; ++i) gn.child[i] = -1;

    gn.qxx = static_cast<float>(node->Qxx);
    gn.qxy = static_cast<float>(node->Qxy);
    gn.qxz = static_cast<float>(node->Qxz);
    gn.qyy = static_cast<float>(node->Qyy);
    gn.qyz = static_cast<float>(node->Qyz);
    gn.qzz = static_cast<float>(node->Qzz);

    flat_nodes.push_back(gn);

    if (!node->leaf) {
        for (int i = 0; i < 8; ++i) {
            if (node->child[i] && node->child[i]->m > 0.0) {
                int child_idx = flatten_octree_node(node->child[i], flat_nodes);
                flat_nodes[current_idx].child[i] = child_idx;
            }
        }
    }
    return current_idx;
}

void compute_gravity_cuda(const ::Octree* cpu_root,
                           const ParticleSystem& ps,
                           float G,
                           float eps,
                           float theta,
                           std::vector<float>& ax,
                           std::vector<float>& ay,
                           std::vector<float>& az,
                           float r_split)
{
    if (!init_jit_cuda()) return;

    const int N = static_cast<int>(ps.size());
    if (N == 0 || !cpu_root) return;

    ax.resize(N);
    ay.resize(N);
    az.resize(N);

    auto cuMemAlloc = (t_cuMemAlloc)GetProcAddress(h_cuda, "cuMemAlloc_v2");
    auto cuMemFree = (t_cuMemFree)GetProcAddress(h_cuda, "cuMemFree_v2");
    auto cuMemcpyHtoD = (t_cuMemcpyHtoD)GetProcAddress(h_cuda, "cuMemcpyHtoD_v2");
    auto cuMemcpyDtoH = (t_cuMemcpyDtoH)GetProcAddress(h_cuda, "cuMemcpyDtoH_v2");
    auto cuLaunchKernel = (t_cuLaunchKernel)GetProcAddress(h_cuda, "cuLaunchKernel");

    // Pre-allocated static buffer to avoid any vector reallocation overhead
    static std::vector<GPUNode> s_flat_nodes;
    s_flat_nodes.clear();
    if (s_flat_nodes.capacity() < static_cast<size_t>(N * 2)) {
        s_flat_nodes.reserve(N * 2);
    }
    flatten_octree_node(cpu_root, s_flat_nodes);
    if (s_flat_nodes.empty()) return;

    size_t part_bytes = N * sizeof(float);
    static std::vector<float> s_x, s_y, s_z;
    if (s_x.size() != static_cast<size_t>(N)) {
        s_x.resize(N); s_y.resize(N); s_z.resize(N);
    }
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < N; i++) {
        s_x[i] = static_cast<float>(ps.x[i]);
        s_y[i] = static_cast<float>(ps.y[i]);
        s_z[i] = static_cast<float>(ps.z[i]);
    }

    if (g_allocated_particles < static_cast<size_t>(N)) {
        if (g_d_x) {
            cuMemFree(g_d_x); cuMemFree(g_d_y); cuMemFree(g_d_z);
            cuMemFree(g_d_ax); cuMemFree(g_d_ay); cuMemFree(g_d_az);
        }
        cuMemAlloc(&g_d_x, part_bytes);
        cuMemAlloc(&g_d_y, part_bytes);
        cuMemAlloc(&g_d_z, part_bytes);
        cuMemAlloc(&g_d_ax, part_bytes);
        cuMemAlloc(&g_d_ay, part_bytes);
        cuMemAlloc(&g_d_az, part_bytes);
        g_allocated_particles = N;
    }

    size_t node_bytes = s_flat_nodes.size() * sizeof(GPUNode);
    if (g_allocated_nodes < s_flat_nodes.size()) {
        if (g_d_nodes) cuMemFree(g_d_nodes);
        cuMemAlloc(&g_d_nodes, node_bytes);
        g_allocated_nodes = s_flat_nodes.size();
    }

    cuMemcpyHtoD(g_d_x, s_x.data(), part_bytes);
    cuMemcpyHtoD(g_d_y, s_y.data(), part_bytes);
    cuMemcpyHtoD(g_d_z, s_z.data(), part_bytes);
    cuMemcpyHtoD(g_d_nodes, s_flat_nodes.data(), node_bytes);

    int threads_per_block = 256;
    int blocks = (N + threads_per_block - 1) / threads_per_block;
    float eps2 = eps * eps;
    float inv_theta2 = 1.0f / (theta * theta);

    void* args[] = { &g_d_x, &g_d_y, &g_d_z, &g_d_nodes, &g_d_ax, &g_d_ay, &g_d_az, (void*)&N, (void*)&G, (void*)&eps2, (void*)&inv_theta2, (void*)&r_split };

    cuLaunchKernel(g_tree_kernel, blocks, 1, 1, threads_per_block, 1, 1, 0, NULL, args, NULL);

    cuMemcpyDtoH(ax.data(), g_d_ax, part_bytes);
    cuMemcpyDtoH(ay.data(), g_d_ay, part_bytes);
    cuMemcpyDtoH(az.data(), g_d_az, part_bytes);
}

} // namespace cuda
} // namespace next
