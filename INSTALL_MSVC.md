# Building NEXT on Windows (MSVC)

This guide covers building **NEXT** on Windows using **Microsoft Visual Studio (MSVC)** and **CMake**.

---

## Prerequisites

- **Visual Studio 2022** (with *Desktop development with C++* workload)
- **CMake** (>= 3.10)
- **Git**
- **Python 3** (for initial condition generation)
- **HDF5** (installed via MSYS2, vcpkg, or official installer)

---

## Build Steps

### 1. Clone the Repository

```powershell
git clone https://github.com/TimGoTheCreator/NEXT.git
cd NEXT
```

### 2. Configure & Build with CMake

Open **Developer PowerShell for VS 2022** or standard PowerShell:

```powershell
# Configure CMake
cmake -B build -S . -G "Visual Studio 17 2022" -A x64

# Build Release binary
cmake --build build --config Release
```

---

## Running an Example Simulation

```powershell
cd examples/TwoBodies
python two_body.py
../../next.exe two_body.txt 8 0.001 0.1 vtu 50
```

---

## Viewing Simulation Results

Open the resulting `dump_*.vtu` files in **ParaView** or **VisIt** to visualize particle dynamics.
