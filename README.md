<p align="center">
  <img src="https://timgothecreator.github.io/images/hero_banner_transparent.png" alt="NEXT" width="90%">
</p>

# NEXT: Newtonian EXact Trajectories

[![Build Status](https://img.shields.io/github/actions/workflow/status/TimGoTheCreator/NEXT/cmake-multi-platform.yml?branch=main)](https://github.com/TimGoTheCreator/NEXT/actions)
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3%2B-blue.svg)](LICENSE)
[![Standard](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![CUDA Acceleration](https://img.shields.io/badge/GPU-NVIDIA%20CUDA%20%2F%20NVRTC-green.svg)](src/cuda/cuda_gravity.cpp)
[![Vectorization](https://img.shields.io/badge/SIMD-AVX--512%20%2F%20AVX2-red.svg)](CMakeLists.txt)
[![Parallelism](https://img.shields.io/badge/Parallel-OpenMP%20%2F%20MPI-orange.svg)](CMakeLists.txt)

A high-performance, multithreaded, GPU-accelerated, and distributed astrophysical N-body simulation framework designed for collisionless gravitational dynamics, galactic structure evolution, TreePM cosmological setups, and multi-body galaxy collisions.

---

## 1. Key Features

- **Native NVIDIA GPU CUDA Barnes-Hut Solver:** Stackless GPU Octree traversal with exact Quadrupole tensor moments and zero-allocation persistent VRAM buffers (capable of 1,000,000+ particle simulations in real time).
- **Hybrid TreePM Gravity (Tree-Particle Mesh):** Long-range Fast Fourier Transform (FFT) Particle-Mesh (PM) solver seamlessly coupled with short-range Barnes-Hut tree forces using complementary error functions ($\text{erfc}$).
- **Exact Jeans-Equilibrium Initial Conditions (`nextinsim`):** High-precision multi-component spiral galaxy generator incorporating self-consistent Miyamoto-Nagai potentials, Hernquist dark matter halos, Toomre $Q$ dispersions, and asymmetric drift corrections.
- **Hardware SIMD Execution:** Native instruction compilation supporting AVX-512, AVX2, FMA3, and BMI2.
- **3D Real-Time Astrophysics Viewport (`renderer.py`):** Interactive GPU/Pygame particle visualizer with zero-lag snapshot pre-caching, multi-species color mapping (stars, dark matter, satellites), and 4K SPH rendering.
- **4-Panel Master Galactic Bar & Morphology Diagnosis Suite (`bardetector.py`):** Automated Fourier $A_2(R)$ harmonic decomposition, bar length/orientation tracking, and kinematic $V/\sigma$ disciness classification.
- **Asynchronous Parallel I/O:** Non-blocking background worker writes consolidated single-file HDF5 containers (`.h5`) with XML Data Format (`.xdmf`) sidecars for ParaView.

---

## 2. Numerical Architecture

### 2.1 Gravitational Force Evaluation
- **Hierarchical Octree & Quadrupole Tensor:** Implements Barnes-Hut spatial octrees with configurable multipole opening criterion ($\theta = 0.5\text{--}0.7$) and exact quadrupole moments:
  $$\Phi(\mathbf{r}) = -\frac{G M}{r} - \frac{G}{2 r^5} \mathbf{r}^T \mathbf{Q} \mathbf{r}$$
- **TreePM Split Formulation:**
  $$\mathbf{a}_{\text{short}}(\mathbf{r}) = -\frac{G m \mathbf{r}}{(r^2 + \epsilon^2)^{3/2}} \left[ \operatorname{erfc}\left(\frac{r}{2 r_s}\right) + \frac{r}{r_s \sqrt{\pi}} e^{-r^2 / 4 r_s^2} \right]$$

### 2.2 Time Integration
- **Symplectic KDK Leapfrog:** Second-order time-reversible symplectic integrator preserving phase-space volume and minimizing secular energy drift:
  $$\mathbf{v}^{n+1/2} = \mathbf{v}^n + \frac{\Delta t}{2} \mathbf{a}^n$$
  $$\mathbf{x}^{n+1} = \mathbf{x}^n + \Delta t \mathbf{v}^{n+1/2}$$
  $$\mathbf{v}^{n+1} = \mathbf{v}^{n+1/2} + \frac{\Delta t}{2} \mathbf{a}^{n+1}$$

---

## 3. Compilation

### 3.1 Requirements
- C++20 compliant compiler (GCC 11+, Clang 14+, or MSVC 2022+)
- CMake 3.16 or higher
- NVIDIA CUDA Toolkit / NVRTC (optional, for `--cuda` GPU acceleration)
- OpenMP 4.5+ runtime libraries
- HDF5 development libraries

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

---

## 4. Usage and CLI

```bash
next <input.txt> <threads> <dt> <dump_steps> <format> [max_dumps] [--cuda] [--treepm]
```

### 4.1 CLI Arguments

| Parameter | Type | Description |
| :--- | :--- | :--- |
| `<input.txt>` | `string` | Path to initial condition file |
| `<threads>` | `int` | CPU OpenMP worker thread count |
| `<dt>` | `float` | Integration timestep (e.g. `0.05`) |
| `<dump_steps>` | `int` | Timesteps between output dumps (e.g. `25`) |
| `<format>` | `string` | Output format: `hdf5-single`, `csv`, or `none` |
| `[max_dumps]` | `int` | Maximum snapshot count before termination (e.g. `300`) |
| `[--cuda]` | `flag` | Enables Native NVIDIA GPU CUDA Barnes-Hut acceleration |
| `[--treepm]` | `flag` | Enables TreePM hybrid periodic particle-mesh solver |
| `[--output]` | `string` | Custom output filename (e.g. `--output simulation.h5`) |

---

## 5. Ecosystem & Analysis Tools

### 5.1 Python Initial Condition Generator (`nextinsim`)
```python
import nextinsim as ns

# Generate a 100,000 particle Jeans-equilibrium spiral galaxy
galaxy = ns.JeansSpiralGalaxy(
    N_disk=70000,
    N_bulge=10000,
    N_halo=20000,
    R_d=3.5,
    z_d=0.15,
    mass_disk=1.0,
    mass_bulge=0.20,
    mass_halo=6.0,
    Toomre_Q=1.10,
    spiral_arms=2
)
galaxy.save("spiral_100k.txt")
```

### 5.2 3D Real-Time Viewport & Video Renderer
```bash
# Launch interactive 3D Viewport
python tools/renderer.py simulation.h5
```

### 5.3 Galactic Bar & Morphology Diagnosis Suite
```bash
# Compute Fourier A_2(R) modes, bar length, and S_bar(t) evolution
python tools/bardetector.py simulation.h5 --all
```

---

## 6. Simulation Examples

Initial condition generators and reference setups are provided in the `examples/` directory:

- **`examples/Galaxy1M/`**: 1,000,000-particle grand-design isolated spiral galaxy with full Jeans equilibrium.
- **`examples/MilkyWayAndromeda/`**: 200,000-particle full-resolution Gaia DR3 tilted collision of the Milky Way and M31.
- **`examples/IsolatedSpiralGalaxy/`**: 300,000-particle Milky Way model populated with 22 real dwarf satellite galaxies.
- **`examples/PureSpiralGalaxy/`**: 50,000-to-100,000 particle tidal flyby and swing-amplification benchmarks.
- **`examples/CosmologyBox/`**: Hybrid TreePM periodic cosmological box with long-range FFT gravity.
- **`examples/ThreewayMerger/`**: Non-coplanar 3-body collision between three equal-mass disk galaxies.

---

## 7. ParaView Visualization Pipeline

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
