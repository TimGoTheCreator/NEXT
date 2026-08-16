# Experimental CUDA Gravity Engine for NEXT 🚀⚡

This folder contains experimental GPU-accelerated gravitational solvers for the **NEXT** N-body simulation engine.

---

## 📂 Architecture Overview:

1. **[`cuda_gravity.h`](file:///c:/n2/next/src/cuda/cuda_gravity.h)**:
   - Interface header declaring `is_cuda_available()` and `compute_gravity_cuda(...)`.
   - Defines `GPUNode`: 32-byte cache-aligned linear node struct with quadrupole moments and stackless sibling pointers.

2. **[`cuda_gravity.cu`](file:///c:/n2/next/src/cuda/cuda_gravity.cu)**:
   - **`kernel_direct_gravity_tiled`**: High-throughput shared-memory tiled gravity solver running at full warp parallelism.
   - **`kernel_octree_gravity`**: Stackless GPU tree traversal kernel with Multipole Acceptance Criteria (MAC).
   - Graceful fallback stub when compiled on systems without NVIDIA `nvcc`.

---

## 🛠️ How to Enable in CMake:
When configuring CMake on a machine with the NVIDIA CUDA Toolkit installed:

```powershell
cmake -B build -DNEXT_ENABLE_CUDA=ON
cmake --build build --config Release
```
