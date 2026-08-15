<p align="center">
  <img src="https://timgothecreator.github.io/images/hero_banner_transparent.png" alt="NEXT" width="100%">
</p>



# NEXT: Newtonian EXact Trajectories

[![Build Status](https://img.shields.io/github/actions/workflow/status/TimGoTheCreator/NEXT/cmake-multi-platform.yml?branch=main)](https://github.com/TimGoTheCreator/NEXT/actions)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3%2B-blue.svg)](LICENSE)
[![Standard](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![Vectorization](https://img.shields.io/badge/SIMD-AVX--512%20%2F%20AVX2-red.svg)](CMakeLists.txt)
[![Parallelism](https://img.shields.io/badge/Parallel-OpenMP%20%2F%20MPI-orange.svg)](CMakeLists.txt)

A high-performance, multithreaded and distributed astrophysical N-body simulation framework designed for collisionless gravitational dynamics, galactic structure evolution, and multi-body galaxy mergers.

---

## 1. Overview

NEXT is an N-body code engineered for large-scale gravitational simulations. It implements a hierarchical spatial octree algorithm with multipole expansions to achieve O(N log N) computational complexity, coupled with a second-order symplectic Kick-Drift-Kick (KDK) leapfrog integrator. The architecture is optimized for modern multi-core microprocessors with SIMD vector units, OpenMP shared-memory threading, and non-blocking asynchronous I/O streaming.

---

## 2. Numerical Methods and Architecture

### 2.1 Gravitational Force Evaluation
- **Hierarchical Octree:** Implements a Barnes-Hut spatial tree with configurable multipole opening criterion (theta = 0.7).
- **Plummer Softening Kernel:** Branchless evaluation of softened gravitational acceleration:
  ```text
  a_i = -G * sum_j [ m_j * (r_i - r_j) / (|r_i - r_j|^2 + eps^2)^(3/2) ]
  ```
- **Division-Avoiding Algebra:** Force kernels are optimized by calculating inverse square-root distances and eliminating redundant floating-point divisions.

### 2.2 Time Integration
- **Symplectic KDK Leapfrog:** Second-order symplectic time-reversible integrator preserving phase-space volume and minimizing secular energy drift over extended integration times.

### 2.3 Memory Layout and Hardware Vectorization
- **Structure-of-Arrays (SoA):** Particle phase-space coordinates, velocities, masses, and types are stored in contiguous, cache-aligned arrays to maximize L1/L2 data cache throughput.
- **Hardware SIMD Execution:** Native instruction compilation targets AVX-512, AVX2, FMA3, and BMI2 instruction sets on modern x86-64 microarchitectures (AMD Zen 4/5, Intel Skylake-X/Sapphire Rapids).

### 2.4 Asynchronous Parallel I/O
- **Non-Blocking I/O Worker:** Simulation snapshot writes to disk are offloaded to background threads (async_writer), preventing I/O serialization from stalling computational physics threads.
- **Single-File HDF5 Container:** Consolidated simulation.h5 storage format with accompanying XML Data Format (.xdmf) sidecars for direct ingestion into visualization packages.

---

## 3. Prerequisites and Compilation

### 3.1 Requirements
- C++20 compliant compiler: GCC 11+, Clang 14+, or MSVC 2022+
- CMake 3.16 or higher
- OpenMP 4.5+ runtime libraries
- HDF5 C development libraries (optional, required for HDF5 output)

### 3.2 Building from Source

```bash
# Clone the repository
git clone https://github.com/TimGoTheCreator/NEXT.git
cd NEXT

# Configure Release build with native hardware vectorization
cmake -B build -S . \
  -DCMAKE_BUILD_TYPE=Release \
  -DNEXT_NATIVE_ARCH=ON \
  -DNEXT_ENABLE_LTO=ON \
  -DNEXT_FP32=ON

# Compile executable
cmake --build build --config Release --parallel
```

### 3.3 CMake Configuration Flags

| Option | Default | Description |
|:---|:---|:---|
| `CMAKE_BUILD_TYPE` | `Release` | Build type (`Release`, `Debug`, `RelWithDebInfo`) |
| `NEXT_NATIVE_ARCH` | `OFF` | Enables `-march=native` architecture-specific optimizations (AVX-512/AVX2) |
| `NEXT_ENABLE_LTO` | `OFF` | Enables Link-Time Optimization (`-flto`) for interprocedural optimizations |
| `NEXT_FP32` | `OFF` | Switches floating-point precision from 64-bit double (`FP64`) to 32-bit single (`FP32`) |
| `NEXT_BUILD_MPI` | `OFF` | Enables MPI support for distributed multi-node execution |

Detailed platform-specific guides:
- [Windows (MSVC)](INSTALL_MSVC.md)
- [Windows (MinGW / MSYS2)](INSTALL_MinGW.md)
- [Linux (Ubuntu / Debian / Fedora / RHEL)](INSTALL_Linux.md)
- [macOS (Darwin / Clang)](INSTALL_Darwin.md)

---

## 4. Execution and Command-Line Interface

```bash
next <input.txt> <threads> <dt> <dump_steps> <format> [max_dumps]
```

### 4.1 Positional Parameters

1. `input.txt` *(string)*: Path to the input ASCII particle table (`x y z vx vy vz mass type`).
2. `threads` *(integer)*: Number of OpenMP worker threads allocated for tree construction and force evaluation.
3. `dt` *(float)*: Base numerical integration time step.
4. `dump_steps` *(integer)*: Frequency of snapshot data outputs in integration steps.
5. `format` *(string)*: Output file format selector:
   - `hdf5-single`: Consolidated HDF5 file (`simulation.h5`) with XDMF temporal collection (`simulation.xdmf`).
   - `hdf5` / `hdf5-multi`: Individual HDF5 files per dump (`dump_0001.h5`).
   - `vtu`: Unstructured XML VTK format.
   - `vtk`: Legacy VTK polydata format.
6. `[max_dumps]` *(integer, optional)*: Maximum number of snapshot outputs before program termination.

### 4.2 Execution Example

```bash
# Execute simulation using 12 OpenMP threads, dt=0.02, dumping every 25 steps to HDF5-single for 120 snapshots:
./next ic.txt 12 0.02 25 hdf5-single 120
```

---

## 5. Simulation Examples

Initial condition generators and reference simulation setups are provided in the `examples/` directory:

- **`examples/ThreewayMerger/`**: Non-coplanar 3-body collision between three equal-mass disk galaxies configured for multi-pass dynamical friction and core coalescence.
- **`examples/AndromedaCollision/`**: Direct pericentric encounter between the Milky Way and Andromeda (M31) with realistic mass ratios and orbital parameters.
- **`examples/AndromedaWithTriangulum/`**: Three-body Local Group interaction (Milky Way, M31, M33) initialized with Gaia DR2 and Hubble Space Telescope astrometry.
- **`examples/FigureEight/`**: Periodic 3-body choreography (Moore 1993).
- **`examples/ColdCollapseGalaxy/`**: Gravitational relaxation and virialization of a spherical cold cloud.

---

## 6. Post-Processing and Morphological Analysis

The repository includes quantitative morphological analysis tools in `tools/`:

```bash
# Compute azimuthal Fourier harmonics (m=1,2,4), bar strength, and phase angle:
python tools/bardetector.py simulation.h5 --plot bar_analysis.png --dpi 250
```

### Diagnostic Output:
- **Fourier Surface Density Decomposition:** Evaluates relative azimuthal harmonics A_m(R) / A_0(R) across radial cylindrical bins.
- **Bar Quantification:** Identifies peak bar amplitude S_bar = max(A_2(R)), semi-major bar radius R_bar, and position angle phi_bar.
- **Temporal Tracking:** Computes bar growth, pattern speed Omega_p = d(phi_bar)/dt, and saturation across snapshot series.
- **Kinematics:** Calculates rotation velocity profile V_rot(R), velocity dispersion tensor sigma(R), and V/sigma ratio.

---

## 7. Visualization Pipeline

1. Launch **ParaView** and open `simulation.xdmf`.
2. Select **XDMF Reader** in the reader prompt and click **Apply**.
3. Apply a **Threshold Filter** on `Mass <= 0.0003` to isolate stellar components from the dark matter halo.
4. Set representation to **Point Gaussian** (Shader preset: `Sphere`, Radius = 0.05).
5. Map coloring to **`Velocity`** with a scientific colormap (`Black-Body Radiation` or `Plasma`).
6. Advance temporal frames using the animation toolbar.

---

## 8. License and Citation

NEXT is distributed under the terms of the **GNU General Public License v3.0 or later (GPLv3+)**. See the [LICENSE](LICENSE) file for complete terms.

When utilizing NEXT in published scientific research, please cite the software using the metadata provided in [CITATION.cff](CITATION.cff).
