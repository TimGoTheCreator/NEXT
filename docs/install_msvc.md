# Building NEXT on Windows (MSVC)

This guide covers building NEXT on Windows using Microsoft Visual Studio (MSVC) and CMake.

---

## Prerequisites

- **Visual Studio 2022** (with *Desktop development with C++* workload)
- **CMake** (>= 3.10)
- **Git**
- **Python 3**
- **HDF5** (installed via MSYS2, vcpkg, or installer)

---

## Installation & Build Steps

### 1. Configure & Build with CMake

Open **Developer PowerShell for VS 2022**:

```powershell
git clone https://github.com/TimGoTheCreator/NEXT.git
cd NEXT

cmake -B build -S . -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

---

### 2. Run Example Simulation

```powershell
cd examples/TwoBodies
python two_body.py
../../next.exe two_body.txt 8 0.001 0.1 vtu 50
```
