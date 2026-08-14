# Chaotic 3-Way Galaxy Merger: The Cosmic Triple Dance

This example simulates a bound 3-body collision between three comparable spiral galaxies (Alpha, Beta, Gamma) arranged in a triangular configuration with out-of-plane 3D orientations.

---

## The Physics: Multi-Pass 3-Body Dynamics
- **Inward Radial Momentum (v_r = 1.8)**: Pulls all 3 galaxies into the central gravitational potential.
- **Tangential Angular Momentum (v_t = 0.85)**: Provides sufficient orbital angular momentum to prevent single-pass collapse, triggering 4 to 8 close pericentric slingshots and orbital exchanges.
- **Non-Coplanar Geometry**: As the 3 galactic cores pass in 3D, gravitational perturbations generate crossing tidal ribbons, bridge filaments, and stellar shells before coalescing into a single remnant.

---

## 1. Generate the 3-Way Merger IC
```powershell
python threeway_merger.py
```
This generates `threeway_merger.txt` with 45,000 particles (15,000 per galaxy).

---

## 2. Run the Simulation with NEXT
```powershell
..\..\build\next.exe threeway_merger.txt 12 0.02 25 hdf5-single 120
```

---

## 3. Visualize in ParaView
1. Open **ParaView** -> Open `simulation.xdmf`.
2. Select **XDMF Reader** -> Click **Apply**.
3. Add a **Threshold Filter** on `Mass <= 0.0003` to hide the dark matter halo.
4. Set representation to **Point Gaussian** (Radius = 0.05).
5. Color by **Velocity** (Preset: `Black-Body Radiation` or `Plasma`).
6. Advance temporal frames along the animation toolbar.
