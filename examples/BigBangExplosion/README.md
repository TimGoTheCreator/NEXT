# Physical Big Bang Explosion & Cosmic Fragmentation (50,000 Particles)

A physical-space expanding universe simulation where particles physically blast outward from an ultra-dense primordial fireball under a linear Hubble expansion law ($\vec{v} = H_0 \vec{r}$).

## Physics Overview:
1. **Initial State**: 50,000 particles packed into a dense primordial core ($R_0 \approx 3.5$).
2. **Hubble Outward Blast**: Every particle is propelled radially with outward velocity proportional to its distance from the center.
3. **Turnaround & Fragmenting Galaxies**: While the bulk universe expands outwards by $10\times$ in radius, local gravitational overdensities turn around and collapse, fragmenting into hundreds of spinning proto-galaxies and cosmic web filaments!

---

## How to Run:

### 1. Generate Initial Conditions:
```powershell
py -3.13 bigbang.py
```

### 2. Run High-Speed Simulation on GPU:
```powershell
..\..\build\next.exe bigbang_50k.txt 12 0.04 25 hdf5-single 300 --cuda --output bigbang.h5
```

### 3. View the Exploding & Fragmenting Universe in 3D:
```powershell
py -3.13 ..\..\tools\renderer.py bigbang.h5
```
