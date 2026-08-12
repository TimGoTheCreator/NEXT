# Building NEXT on Linux

This guide covers building NEXT on Linux platforms (Ubuntu, Debian, Fedora, RHEL).

---

## Prerequisites

- **Git**
- **CMake** (>= 3.10)
- **GCC / Clang** (with OpenMP support)
- **HDF5** development libraries
- **Python 3**

---

## Installation & Build Steps

### 1. Install System Dependencies

#### Ubuntu / Debian:
```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake git libomp-dev libhdf5-dev python3
```

#### Fedora / RHEL:
```bash
sudo dnf install -y gcc-c++ cmake git libomp-devel hdf5-devel python3
```

---

### 2. Build NEXT

```bash
git clone https://github.com/TimGoTheCreator/NEXT.git
cd NEXT

cmake -B build -S .
cmake --build build --config Release
```

---

### 3. Run Example Simulation

```bash
cd examples/TwoBodies
python3 two_body.py
../../next two_body.txt 8 0.001 0.1 vtu 50
```
