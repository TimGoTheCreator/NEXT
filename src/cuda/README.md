# Experimental CUDA Gravity Engine for NEXT

This directory contains GPU-accelerated gravitational solvers for the **NEXT** N-body simulation framework.

---

## Architecture Overview:

1. **[`cuda_gravity.h`](file:///c:/n2/next/src/cuda/cuda_gravity.h)**:
   - Interface header declaring `is_cuda_available()` and `compute_gravity_cuda(...)`.
   - Defines `GPUNode`: 96-byte cache-aligned linear node struct with quadrupole moments and stackless traversal pointers.

2. **[`cuda_gravity.cpp`](file:///c:/n2/next/src/cuda/cuda_gravity.cpp)**:
   - **`kernel_octree_gravity`**: GPU Barnes-Hut octree traversal kernel with Multipole Acceptance Criteria (MAC) and quadrupole moment corrections.
   - NVRTC runtime compilation and CUDA driver API execution with automatic fallback to host CPU OpenMP solver.

---

## How to Enable in CMake:
When configuring CMake on a machine with NVIDIA GPU hardware:

```powershell
cmake -B build -DNEXT_ENABLE_CUDA=ON
cmake --build build --config Release
```
