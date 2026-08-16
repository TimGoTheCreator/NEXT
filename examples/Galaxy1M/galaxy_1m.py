#!/usr/bin/env python3
"""
===================================================================================
1,000,000 PARTICLE (1 MILLION) ULTRA-RESOLUTION SPIRAL GALAXY BENCHMARK
===================================================================================
Simulates a full 1 Million particle self-gravitating galaxy using NEXT CUDA Engine:
- 700,000 Disk Stars
- 100,000 Bulge Stars
- 200,000 Dark Matter Halo Particles
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
print("  NEXT - 1,000,000 (1 Million) Particle Grand-Design Spiral Galaxy")
print("================================================================================")
print("  • Disk Stars: 700,000 Particles (ptype=4)")
print("  • Bulge Stars: 100,000 Particles (ptype=2)")
print("  • Dark Matter: 200,000 Particles (ptype=1)")
print("  • Physics: Exact Jeans Equilibrium + Swing Amplification (Toomre Q = 1.10)")
print("================================================================================")

galaxy = ns.JeansSpiralGalaxy(
    N_disk=700000,
    N_bulge=100000,
    N_halo=200000,
    R_d=3.5,
    z_d=0.15,
    mass_disk=1.0,
    mass_bulge=0.20,
    bulge_scale=0.6,
    mass_halo=7.0,
    halo_scale=16.0,
    Toomre_Q=1.10,
    spiral_arms=2,
    pitch_angle_deg=14.0,
    spiral_amplitude=0.25,
    seed=1000000
)

output_file = "galaxy_1m.txt"
galaxy.save(output_file)

print(f"\n[SUCCESS] Generated {output_file} ({len(galaxy.particles):,} particles total)!")
print("================================================================================")
