#!/usr/bin/env python3
"""
===================================================================================
CLOSE PROGRADE TIDAL FLYBY (50,000 PARTICLES) - M51 / WHIRLPOOL GALAXY ANALOG
===================================================================================
1. Primary Grand-Design Spiral (35,000 particles, M_tot = 6.0, R_d = 3.5).
2. Very Massive Fat Perturber (15,000 particles, M_tot = 18.0, 3x heavier!).
3. Close Grazing Flyby:
   - Starts at separation X = 28.0, Y = 14.0, Z = 3.0
   - Inbound prograde velocity: VX = -1.6, VY = 0.5, VZ = 0.2
   - Grazes just outside the disk edge (pericenter r_p ~ 11.0 kpc) at resonance,
     producing massive tidal bridges and giant sweeping counter-tails!
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
print("  NEXT - Close Grazing Tidal Flyby Encounter (50,000 Particles)")
print("================================================================================")
print("  • Primary Spiral: 25k disk + 3k bulge + 7k halo (M_tot = 6.0)")
print("  • Massive Fat Companion: 10k disk + 2k bulge + 3k halo (M_tot = 18.0, 3x MASS)")
print("  • Grazing Pericenter: ~11 kpc (passes right past disk edge at resonance!)")
print("================================================================================")

# 1. Primary Target Spiral Galaxy (35,000 particles)
primary = ns.JeansSpiralGalaxy(
    N_disk=25000,
    N_bulge=3000,
    N_halo=7000,
    R_d=3.5,
    z_d=0.15,
    mass_disk=1.0,
    mass_bulge=0.15,
    bulge_scale=0.5,
    mass_halo=5.0,
    halo_scale=14.0,
    Toomre_Q=1.05,
    spiral_arms=2,
    pitch_angle_deg=16.0,
    spiral_amplitude=0.20,
    seed=42
)

# 2. Very Massive, Fat Companion Perturber (15,000 particles, 3x mass of primary)
perturber = ns.JeansSpiralGalaxy(
    N_disk=10000,
    N_bulge=2000,
    N_halo=3000,
    R_d=4.5,
    z_d=0.40,            # Very fat disk
    mass_disk=3.0,       # Heavy stellar mass
    mass_bulge=1.0,      # Dense core
    bulge_scale=1.2,
    mass_halo=14.0,      # Huge DM halo (enormous tidal force!)
    halo_scale=20.0,
    Toomre_Q=1.40,
    spiral_arms=0,
    seed=1337
)

# 3. Setup Close Grazing Flyby Trajectory (Pericenter ~ 11 kpc, Prograde)
perturber.translate(dx=28.0, dy=14.0, dz=3.0)
perturber.boost(vx=-1.6, vy=0.5, vz=0.2)

# Combine both into one simulation system
encounter = primary + perturber

output_file = "tidal_flyby_50k.txt"
encounter.save(output_file)

print(f"\n[SUCCESS] Exported {output_file} ({len(encounter.particles):,} particles total)!")
print("================================================================================")
