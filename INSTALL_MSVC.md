# 🖥️ Building NEXT on Windows (MSVC)

## Prerequisites

- Git
- CMake ≥ 3.10
- Visual Studio 2022 with C++ workload
- vcpkg
- HDF5 (installed via vcpkg)
- Python 3

## Steps

1. **Clone the repository**

```powershell
git clone https://github.com/TimGoTheCreator/NEXT.git
cd NEXT
```

2. **Install vcpkg and HDF5**

```powershell
git clone https://github.com/microsoft/vcpkg.git C:/vcpkg
C:/vcpkg/bootstrap-vcpkg.bat
C:/vcpkg/vcpkg install hdf5[hl]:x64-windows
```

3. **Build NEXT**

```powershell
mkdir build
cd build
cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake ..
cmake --build . --config Release
cd ..
```

4. **Run a built-in example simulation**

```powershell
cd examples/TwoBodies
python two_body.py
..\..\next.exe two_body.txt 8 0.01 0.1 vtu
```

5. **View results**

Open the `.vtu` output in ParaView.
