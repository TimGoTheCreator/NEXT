# 🐧 Building NEXT on Linux (Ubuntu)

## Prerequisites

- Git
- CMake ≥ 3.10
- GCC or Clang
- OpenMP
- HDF5 development libraries
- Python 3 (for input generation)

## Steps

1. **Clone the repository**

```bash
git clone https://github.com/TimGoTheCreator/NEXT.git
cd NEXT
```

2. **Install dependencies**

```bash
sudo apt-get update
sudo apt-get install -y libomp-dev libhdf5-dev python3
```

3. **Build NEXT**

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
cd ..
```

4. **Run a built-in example simulation**

```bash
cd examples/TwoBodies
python two_body.py
../../next two_body.txt 8 0.001 0.1 vtu
```

5. **View results**

Open the `.vtu` output in ParaView.
