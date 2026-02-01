# NEXT - Newtonian EXact Trajectories

### NEXT supports four operating modes:
FP32, FP64, SIMD32, SIMD64, AVX512_32, AVX512_64

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

### NEXT uses a CMake configuration. 
```bash
mkdir build
cd build
cmake .. 
cmake --build . 
cd ..
```