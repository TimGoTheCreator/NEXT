# Cosmological Structure Formation & Cosmic Web (32,768 Particles)

Comoving N-body cosmological box simulation with expanding FLRW metric ($\Lambda\text{CDM}$).

## Initial Conditions Physics:
- **Transfer Function**: Eisenstein & Hu (1998) Cold Dark Matter fitting formula.
- **Power Spectrum**: $P(k) = A \cdot k^{n_s} \cdot T(k)^2$ with scalar spectral index $n_s = 0.9667$.
- **Displacements & Peculiar Velocities**: Exact Fourier-space Zeldovich approximation starting at redshift $z = 50$.

---

## How to Run:

### 1. Generate Initial Conditions:
```powershell
py -3.13 cosmology_box.py
```

### 2. Run Cosmological Simulation in NEXT:
```powershell
..\..\build\next.exe cosmology_32k.txt 12 0.02 20 hdf5-single 300 --cosmology --z-start 50.0 --treepm --pm-grid 128 --output cosmos.h5
```

### 3. View the Expanding Cosmic Web in 3D:
```powershell
py -3.13 ..\..\tools\renderer.py cosmos.h5
```
