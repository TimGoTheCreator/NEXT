# Compiling NEXT

NEXT uses the **CMake** build system for cross-platform compilation.

---

## Basic Compilation Procedure

From the repository root directory, run:

```bash
# Configure build
cmake -B build -S .

# Build Release binary
cmake --build build --config Release
```

Upon successful compilation, the executable binary is placed in the project root directory:
- **Linux / macOS:** `next`
- **Windows:** `next.exe`

---

## Build Options

Build parameters can be configured via CMake flags:

```bash
# Enable 32-bit floating point precision (default is 64-bit FP64)
cmake -B build -S . -DNEXT_FP32=ON -DNEXT_FP64=OFF

# Enable MPI support for distributed memory clusters
cmake -B build -S . -DNEXT_MPI=ON
```
