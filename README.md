# NEXT - Newtonian EXact Trajectories

![Build](https://img.shields.io/github/actions/workflow/status/TimGoTheCreator/NEXT/cmake-multi-platform.yml?branch=main)
![License: GPL v3](https://img.shields.io/badge/License-GPLv3%2B-blue.svg)
![C++17](https://img.shields.io/badge/C%2B%2B-17-brightgreen.svg)
![OpenMP](https://img.shields.io/badge/OpenMP-enabled-orange.svg)
![MPI](https://img.shields.io/badge/MPI-supported-purple.svg)
![HDF5](https://img.shields.io/badge/HDF5-supported-blue.svg)

## Multi-Threaded & Distributed Simulation Tool for Solving the N-Body Problem
Powered by Barnes-Hut Octree O(N log N), Symplectic KDK Leapfrog Integration, OpenMP, and MPI.

---

### Key Features
- **O(N log N) Barnes-Hut Octree:** Scalable spatial tree hierarchy with multipole expansions.
- **Symplectic KDK Leapfrog Integrator:** Preserves energy stability over extended numerical integration.
- **Division-Avoiding Algebra:** Optimized force calculations (3–6x speedup by minimizing costly float divisions).
- **Hybrid Parallelism:** **OpenMP** multi-threading (multi-core) + **MPI** (distributed computing cluster).
- **Precision Modes:** Configurable compile-time switching between `FP32` and `FP64`.
- **Multi-Format Output:** Native support for **VTK**, **VTU** (ParaView), and **HDF5**.
- **Python IC Builder:** Legacy initial condition scripts (`tools/icbuilder.py`).

---

### Building NEXT

```bash
# Configure build directory
cmake -B build -S .

# Build executable (Release configuration)
cmake --build build --config Release
```

For detailed platform-specific installation instructions, see:
- [Windows (MSVC)](INSTALL_MSVC.md)
- [Windows (MinGW / MSYS2)](INSTALL_MinGW.md)
- [Linux (Ubuntu / Debian / Fedora)](INSTALL_Linux.md)
- [macOS](INSTALL_Darwin.md)

---

### CLI Usage & Demo

```bash
# Generate initial conditions
cd examples/TwoBodies
python two_body.py

# Run NEXT simulation
../../next two_body.txt <threads> <dt> <dump_interval> <vtk|vtu|hdf5> [max_steps]

# Example: Run 8 threads, dt=0.001, dump every 0.1s, output VTU, for 100 max steps
../../next two_body.txt 8 0.001 0.1 vtu 100
```

#### Command Arguments Explained:
1. `input.txt` - Path to initial condition particle file
2. `threads` - Number of CPU threads (OpenMP)
3. `dt` - Baseline time step
4. `dump_interval` - Simulation time interval between output dumps
5. `format` - Output format (`vtk`, `vtu`, or `hdf5`)
6. `[max_steps]` - *(Optional)* Maximum simulation steps before auto-exit

---

### Media & Previews

- **[Preview the figure-eight simulation](https://timgothecreator.github.io/videos/f8th.mp4)**
