# Local Group Trio: Milky Way + Andromeda (M31) + Triangulum (M33)

This example models the full 3-body Local Group galactic interaction:
1. **Milky Way**: Primary barred/spiral galaxy (M = 9.2, R_d = 3.0).
2. **Andromeda (M31)**: Giant tilted spiral galaxy (M = 14.0, R_d = 5.5, 77 degrees inclination).
3. **Triangulum (M33)**: Compact pure-disk companion spiral galaxy (M = 1.35, R_d = 1.5, 54 degrees inclination) orbiting Andromeda before entering the gravitational wake of the merger.

---

## 1. Generate the Initial Conditions

### Option A: Realistic Gaia/Hubble Astrometric Infall
```powershell
python triangulum_collision.py
```
Generates `andromeda_triangulum_mw.txt` with authentic 3D phase-space coordinates derived from Gaia DR2 and Hubble Space Telescope data.

### Option B: Zoomed-In Pericenter Encounter (Instant Collision Action)
```powershell
python triangulum_3b_collision.py
```
Generates `andromeda_triangulum_3b.txt` pre-advanced to first pericenter.

---

## 2. Run the Simulation with NEXT
```powershell
..\..\build\next.exe andromeda_triangulum_3b.txt 12 0.02 25 hdf5-single 150
```

---

## 3. Visualize in ParaView
1. Open **ParaView**.
2. Open `simulation.xdmf` (Select **XDMF Reader**).
3. Click **Apply**.
4. Apply a **Threshold Filter** on `Mass <= 0.0003` to isolate stellar components.
5. Set representation to **Point Gaussian** (Radius = 0.05).
6. Color by **Velocity** (Preset: `Black-Body Radiation` or `Plasma`).
7. Advance frames along the animation toolbar.
