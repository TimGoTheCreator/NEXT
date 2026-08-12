# Building NEXT on Windows (MinGW / MSYS2)

This guide covers building NEXT on Windows using MinGW-w64 GCC via MSYS2.

---

## Prerequisites

- **MSYS2** (MinGW-w64 environment)
- **CMake** (>= 3.10)
- **Git**
- **Python 3**

---

## Installation & Build Steps

### 1. Install Dependencies in MSYS2 UCRT64 / MinGW64 Terminal

```bash
pacman -Syu --noconfirm
pacman -S --noconfirm mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja mingw-w64-ucrt-x86_64-hdf5 python
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
python two_body.py
../../next.exe two_body.txt 8 0.001 0.1 vtu 50
```
