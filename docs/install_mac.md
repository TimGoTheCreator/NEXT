# Building NEXT on macOS

This guide covers building NEXT on macOS using Homebrew.

---

## Prerequisites

- **Homebrew**
- **Git**
- **CMake** (>= 3.10)
- **Python 3**

---

## Installation & Build Steps

### 1. Install System Dependencies

```bash
brew update
brew install cmake libomp hdf5 python
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
