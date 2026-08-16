#pragma once
#include <vector>
#include "../struct/particle.h"

struct Octree;

namespace next {
namespace cuda {

/**
 * @brief High-performance linear tree node layout matching PKDGRAV3 / Bonsai.
 */
struct alignas(16) GPUNode {
    float cx, cy, cz;      // Center of mass
    float mass;            // Total mass
    float size;            // Half-size / opening radius
    int is_leaf;           // 1 if leaf, 0 if internal
    int body_idx;          // Particle index if leaf, -1 otherwise
    int child[8];          // Indices to 8 octant children (-1 if empty)

    // Quadrupole moment tensor (symmetric)
    float qxx, qxy, qxz;
    float qyy, qyz, qzz;
    float pad[3];          // Explicit padding to 96-byte stride matching GPU kernel
};

/**
 * @brief Checks if CUDA driver and NVRTC JIT runtime are available.
 */
bool is_cuda_available();

/**
 * @brief Computes N-body gravity on GPU using PKDGRAV3-grade stackless Barnes-Hut with quadrupoles.
 */
void compute_gravity_cuda(const struct ::Octree* root,
                           const ParticleSystem& ps,
                           float G,
                           float eps,
                           float theta,
                           std::vector<float>& ax,
                           std::vector<float>& ay,
                           std::vector<float>& az);

} // namespace cuda
} // namespace next
