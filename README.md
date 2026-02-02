# NEXT - Newtonian EXact Trajectories

![Build](https://img.shields.io/github/actions/workflow/status/TimGoTheCreator/NEXT/cmake-multi-platform.yml?branch=main)
![License: GPL v3](https://img.shields.io/badge/License-GPLv3%2B-blue.svg)
![C++20](https://img.shields.io/badge/C%2B%2B-20-brightgreen.svg)
![OpenMP](https://img.shields.io/badge/OpenMP-enabled-orange.svg)

NEXT is a Multi-Threaded Simulation tool written in C/C++, solving the N-Body Problem
using the Barnes-Hut Algorithm at a time of O(N log N) 

### Initial Conditions
NEXT includes a modular python script to generate
initial conditions, it outputs simple
.txt files that NEXT can load directly.

Supported IC types include:
- Plummer sphere (cold)
- Hernquist sphere
- Exponential disk galaxy
- Composite galaxy (bulge + disk + halo)
- Two‑galaxy mergers
- Uniform sphere
- Binary system
- Three‑body figure‑eight choreography
- Four‑body cross choreography
- Random solar systems (with optional moons)

The generator script is located in tools/icbuilder.py


### NEXT supports six operating modes:
FP32, FP64, SIMD32, SIMD64, AVX512_32, AVX512_64

### NEXT Multi-threading
NEXT Uses OpenMP, with the "#pragma omp parallel for" method

### NEXT uses a 3D Newtonian gravity formulation with:
reduced division count (3-6x faster on avg.) 
symmetric-force application
SIMD-Friendly algebra

### NEXT particle representation and I/O
Particles store:
```cpp
position (x, y, z);
velocity (vx, vy, vz);
mass (m);
```

### Building NEXT
```bash
mkdir build
cd build
cmake .. 
cmake --build . --config Release
cd ..
```

### Running a Demo
```bash
cd examples/TwoBodies
python two_body.py
../../next two_body.txt 0.1
```

### NEXT Mathematics
NEXT uses Division-avoiding algebra optimizations to make the simulation run faster.

NEXT also uses the Kick-Drift-Kick Leapfrog method which is implemented like this:

"

- Compute velocities with 0.5 of dt
- Compute Positions with full dt
- Compute velocities with 0.5 dt

"

### NEXT Data Output
NEXT uses the .VTK format to output data.
More formats like HDF5 are planned. 