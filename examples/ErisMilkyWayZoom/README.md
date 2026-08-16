# Eris & ErisBH Cosmological Milky Way Zoom-In Simulation

A high-fidelity replica of the landmark **Eris Simulation (Guedes et al. 2011)** and **ErisBH (Bellovary & Governato)** cosmological zoom-in initial conditions.

---

## Scientific Architecture:

1. **Multi-Scale Nested Grids**:
   - **Parent Volume**: 90 Mpc/h comoving cosmological box for real tidal gravitational torques.
   - **Lagrangian Zoom-in Core**: Ultra-high-resolution region targeting the exact matter volume that collapses into a **Milky Way-sized disk galaxy ($M_{\text{vir}} \approx 8 \times 10^{11} M_\odot$)** with a quiet late-merging history.
2. **Cosmological Foundation**:
   - Starting redshift: $z_{\text{init}} = 90.0$ (Scale factor $a = 0.010989$).
   - Matter Power Spectrum: Eisenstein & Hu (1998) transfer function with Baryon Acoustic Oscillations.
   - 2nd-order Lagrangian Perturbation Theory (2LPT) & Fourier Zeldovich displacement.
3. **ErisBH Black Hole Physics**:
   - Seeds an intermediate-mass central black hole particle ($M_{\text{BH}} \sim 10^5 M_\odot$, PartType5) at the potential minimum.

---

## How to Run:

### 1. Generate Eris Zoom-in Initial Conditions:
```powershell
py -3.13 eris_zoom.py
```

### 2. Run High-Speed Cosmological Simulation on GPU with TreePM:
```powershell
..\..\build\next.exe eris_zoom_ic.txt 12 0.02 20 hdf5-single 300 --cuda --cosmology --z-start 90.0 --treepm --pm-grid 128 --output eris_milkyway.h5
```

### 3. View the Evolving Milky Way Formation & Black Hole in 3D:
```powershell
py -3.13 ..\..\tools\renderer.py eris_milkyway.h5
```
