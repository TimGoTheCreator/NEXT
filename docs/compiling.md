# Compiling NEXT

NEXT uses a simple **CMake** build system.  
To compile (assuming you are in the project root), run:

    mkdir build
    cd build
    cmake ..
    cmake --build .
    cd ..

After this, you will have access to the compiled executable:
```bash
- **Linux/macOS** → `next`
- **Windows** → `next.exe`
```
