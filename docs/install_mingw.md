# 🖥️ Building NEXT on Windows (MinGW)

## Prerequisites

- Git
- CMake ≥ 3.10
- MinGW-w64 GCC
- MSYS2 environment
- HDF5 via MSYS2
- Python 3
- (Optional) Chocolatey, for automated MSYS2 install

## Steps

1. **Clone the repository**

```bash
git clone https://github.com/TimGoTheCreator/NEXT.git
cd NEXT
```

2. **Install dependencies via MSYS2**

```bash
choco install msys2 -y
C:/tools/msys64/usr/bin/pacman -Syu --noconfirm
C:/tools/msys64/usr/bin/pacman -S --noconfirm mingw-w64-x86_64-hdf5 python
```

3. **Build NEXT**

```bash
mkdir build
cd build
cmake -G "MinGW Makefiles" -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ ..
cmake --build . --config Release
cd ..
```

4. **Run a built-in example simulation**

```bash
cd examples/TwoBodies
python two_body.py
```

#### If the copy command worked (copies to project root by default in CMake):
../../next.exe two_body.txt 8 0.001 0.1 vtu

#### If your executable is in build/
../../build/next.exe two_body.txt 8 0.001 0.1 vtu

5. **View results**

Open the `.vtu` output in ParaView.
