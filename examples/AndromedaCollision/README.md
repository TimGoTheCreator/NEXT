# Milky Way & Andromeda (M31) Collision Simulation

This example sets up and simulates the future collision and merger of the Milky Way and Andromeda (M31) galaxies.

---

## 1. Generate Initial Conditions
Generate the multi-component initial conditions:
```powershell
python andromeda_collision.py
```
This generates `andromeda_mw_collision.txt` with:
- **Milky Way**: Exponential disk (R_d = 3.0), Hernquist bulge, and Dark Matter halo.
- **Andromeda (M31)**: ~1.5x more massive (R_d = 5.5), tilted at 77 degrees inclination.
- **Orbit**: Parabolic approach trajectory (separation D = 80.0, impact parameter b = 15.0).

---

## 2. Run the Collision Simulation with NEXT
Run the simulation in high-performance single-file HDF5 mode:
```powershell
..\..\build\next.exe andromeda_mw_collision.txt 12 0.02 25 hdf5-single 120
```

### Parameters:
- `12`: 12 CPU threads (OpenMP acceleration).
- `0.02`: Time step dt.
- `25`: Dump interval in physics steps.
- `hdf5-single`: Fast single-file output (`simulation.h5` + `simulation.xdmf`).
- `120`: Total dump count before auto-exit.

---

## 3. Visualize in ParaView
1. Open **ParaView**.
2. Open `simulation.xdmf` (Select **XDMF Reader**).
3. Click **Apply**.
4. Apply a **Threshold Filter** on `Mass <= 0.0003` to isolate stellar disks from the dark matter halo.
5. Set representation to **Point Gaussian** (Radius = 0.05).
6. Change Coloring to **Velocity** (Preset: `Black-Body Radiation` or `Plasma`).
7. Play the animation slider to watch the collision, tidal bridges, and merger progression.
