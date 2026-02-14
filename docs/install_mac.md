# 🍏 Building NEXT on macOS

## Prerequisites

- Git
- CMake ≥ 3.10
- Clang
- OpenMP
- HDF5
- Python 3

## Steps

1. **Clone the repository**

```bash
git clone https://github.com/TimGoTheCreator/NEXT.git
cd NEXT
```

2. **Install dependencies using Homebrew**

```bash
brew update
brew install llvm libomp hdf5 python
```

3. **Build NEXT**

```bash
mkdir build
cd build
cmake .. -DCMAKE_C_COMPILER=/opt/homebrew/opt/llvm/bin/clang -DCMAKE_CXX_COMPILER=/opt/homebrew/opt/llvm/bin/clang++
cmake --build . --config Release
cd ..
```

4. **Run a built-in example simulation**

```bash
cd examples/TwoBodies
python two_body.py
../../next two_body.txt 8 0.01 0.1 vtu
```

5. **View results**

Open the `.vtu` output in ParaView.
