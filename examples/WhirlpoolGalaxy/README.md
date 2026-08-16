# M51 Whirlpool Galaxy & NGC 5195 Companion Flyby (50,000 Particles) 🌌🌀

A realistic astrophysical simulation of the **Whirlpool Galaxy (M51 / NGC 5194)** excited by the prograde tidal flyby of its companion dwarf galaxy **(NGC 5195)**, based on the landmark research of **Toomre & Toomre (1972)** and **Salo & Laurikainen (2000)**.

---

## 🧬 Astrophysical Mechanics:
1. **Unperturbed Equilibrium Disk**: M51 starts as a smooth, stable circular disk with no artificial pre-baked spiral arms.
2. **Prograde Tidal Resonance**: The companion satellite ($M_c \approx \frac{1}{3} M_{\text{main}}$) approaches on an inclined prograde orbit.
3. **Grand-Design Spiral Arms**: As the satellite grazes past the pericenter ($r_p \approx 9.0$), its tidal pull synchronizes with the disk's orbital rotation ($\Omega(R)$), triggering maximum **Swing Amplification & Lindblad Resonances**.
4. **Tidal Bridge & Counter-Tail**: A luminous stellar bridge reaches out connecting toward the companion while a sweeping, symmetric spiral arm unfurls on the opposite side!

---

## 🚀 How to Run:

### 1. Generate Initial Conditions:
```powershell
py -3.13 whirlpool.py
```

### 2. Run Simulation in NEXT:
```powershell
..\..\build\next.exe whirlpool.txt 12 0.08 25 hdf5-single 250
```

### 3. View the Grand-Design Whirlpool Arms in 3D:
```powershell
py -3.13 ..\..\tools\renderer.py simulation.h5
```
