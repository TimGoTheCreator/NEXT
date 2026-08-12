# Building NEXT on macOS

This guide covers building **NEXT** on macOS using **Clang** / **Apple Clang** or **GCC** via **Homebrew**.

---

## Prerequisites

- **Homebrew**
- **Git**
- **CMake** (>= 3.10)
- **Python 3**

---

## Build Steps

### 1. Install Dependencies using Homebrew

```bash
brew update
brew install cmake libomp hdf5 python
```

### 2. Clone & Build NEXT

```bash
git clone https://github.com/TimGoTheCreator/NEXT.git
cd NEXT

cmake -B build -S .
cmake --build build --config Release
```

---

## Running an Example Simulation

```bash
cd examples/TwoBodies
python3 two_body.py
../../next two_body.txt 8 0.001 0.1 vtu 50
```
