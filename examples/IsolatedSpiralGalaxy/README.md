# Isolated Equilibrium Spiral Galaxy (100,000 Particles)

An isolated multi-component spiral galaxy generated in dynamical and virial equilibrium.

## Astrophysical Components:
- **Exponential Stellar Disk**: 30,000 stars (R_d = 4.0, z_d = 0.25)
- **Central Bulge**: 5,000 stars (R_b = 0.8)
- **Dark Matter Halo**: 65,000 particles (M_h = 5.0, R_h = 12.0)
- **Stability**: Toomre Q = 1.3 with a 2-arm spiral density perturbation.

---

## How to Run:

### 1. Generate Initial Conditions:
```powershell
py -3.13 spiral_galaxy.py
```

### 2. Run Simulation in NEXT:
```powershell
..\..\build\next.exe spiral_galaxy.txt 12 0.05 20 hdf5-single 300
```

### 3. View the Rotating Galaxy:
```powershell
py -3.13 ..\..\tools\renderer.py simulation.h5
```
