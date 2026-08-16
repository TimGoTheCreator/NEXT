# M51 Whirlpool Galaxy & NGC 5195 Companion Flyby (50,000 Particles)

Astrophysical simulation of the **Whirlpool Galaxy (M51 / NGC 5194)** excited by the prograde tidal flyby of its companion dwarf galaxy **(NGC 5195)**, based on the models of Toomre & Toomre (1972) and Salo & Laurikainen (2000).

---

## Astrophysical Mechanics:
1. **Unperturbed Equilibrium Disk**: M51 starts as a smooth, stable circular disk in dynamical equilibrium.
2. **Prograde Tidal Resonance**: The companion satellite approaches on an inclined prograde orbit.
3. **Grand-Design Spiral Arms**: As the satellite passes pericenter, tidal forces synchronize with disk rotation, triggering swing amplification and Lindblad resonances.
4. **Tidal Bridge & Counter-Tail**: A stellar bridge forms connecting toward the companion alongside an extended tidal counter-arm on the opposite side.

---

## How to Run:

### 1. Generate Initial Conditions:
```powershell
py -3.13 whirlpool.py
```

### 2. Run Simulation in NEXT:
```powershell
..\..\build\next.exe whirlpool.txt 12 0.08 25 hdf5-single 250
```

### 3. View the Result in 3D:
```powershell
py -3.13 ..\..\tools\renderer.py simulation.h5
```
