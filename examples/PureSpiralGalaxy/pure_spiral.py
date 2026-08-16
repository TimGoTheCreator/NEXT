#!/usr/bin/env python3
"""
===================================================================================
NATURAL SELF-CONSISTENT SPIRAL GALAXY (100,000 PARTICLES)
===================================================================================
Real N-body spiral arms:
- Smooth full 2D exponential stellar disk (not a single line / wireframe).
- Self-consistent Miyamoto-Nagai / Hernquist / Halo gravitational equilibrium.
- Jeans asymmetric drift + Toomre Q dispersion.
- Smooth global m=2 swing-amplification density wave perturbation.
===================================================================================
"""

import os
import sys

# Add nextsim (c:/n2/nextsim) to python path
nextsim_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "..", "nextsim"))
if nextsim_dir not in sys.path:
    sys.path.insert(0, nextsim_dir)

import nextinsim as ns

print("=" * 80)
print("  NEXT - Natural Spiral Galaxy Initial Conditions (100,000 Particles)")
print("================================================================================")
print("  • 70,000 Disk Stars (Full 2D Exponential Disk)")
print("  • 10,000 Bulge Stars (Hernquist Core)")
print("  • 20,000 Dark Matter Halo Particles")
print("  • Physics: Swing-Amplification m=2 Mode (Toomre Q = 1.15)")
print("================================================================================")

galaxy = ns.EquilibriumSpiralGalaxy(
    N_disk=70000,
    N_bulge=10000,
    N_halo=20000,
    R_d=3.5,
    z_d=0.15,
    mass_disk=1.0,
    mass_bulge=0.20,
    bulge_scale=0.6,
    mass_halo=6.0,
    halo_scale=15.0,
    Toomre_Q=1.15,
    spiral_arms=2,
    pitch_angle_deg=20.0,
    spiral_amplitude=0.30
)

output_file = "pure_spiral_100k.txt"
galaxy.save(output_file)
print(f"\n[SUCCESS] Exported {output_file} ({len(galaxy.particles):,} particles)!")
print("================================================================================")
