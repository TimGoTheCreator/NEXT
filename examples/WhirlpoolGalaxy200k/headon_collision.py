import os
import sys

# Add nextsim to python path
nextsim_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "..", "nextsim"))
if nextsim_dir not in sys.path:
    sys.path.insert(0, nextsim_dir)

import nextinsim as ns

# =============================================================================
# Head-On Galaxy Collision Benchmark: 30,000 vs 20,000 Particles
# =============================================================================
TOTAL_N = 50000

# 1. Primary Galaxy G1 (30,000 particles)
N_DISK_1  = 15000   # 15,000 disk stars
N_BULGE_1 = 3000    # 3,000 bulge stars
N_HALO_1  = 12000   # 12,000 dark matter halo particles
MASS_G1   = 4.5

# 2. Secondary Galaxy G2 (20,000 particles)
N_DISK_2  = 10000   # 10,000 disk stars
N_BULGE_2 = 2000    # 2,000 bulge stars
N_HALO_2  = 8000    # 8,000 dark matter halo particles
MASS_G2   = 2.5

print("=" * 75)
print("  NEXT - Generating Head-On Galaxy Collision (30k vs 20k)")
print("===========================================================================")
print(f"  • Galaxy 1 (Primary):   {N_DISK_1 + N_BULGE_1 + N_HALO_1:,} particles (M = {MASS_G1})")
print(f"  • Galaxy 2 (Secondary): {N_DISK_2 + N_BULGE_2 + N_HALO_2:,} particles (M = {MASS_G2})")
print("  • Trajectory:           Direct Head-On Collision Course along X-axis")
print("=" * 75)

# Primary Galaxy in self-consistent equilibrium (centered at X = -15 kpc moving +X)
g1 = ns.EquilibriumSpiralGalaxy(
    N_disk=N_DISK_1,
    N_bulge=N_BULGE_1,
    N_halo=N_HALO_1,
    R_d=3.5,
    z_d=0.25,
    mass_disk=1.0,
    mass_bulge=0.3,
    bulge_scale=0.7,
    mass_halo=3.2,
    halo_scale=9.0,
    Toomre_Q=1.2,
    spiral_arms=2,
    pitch_angle_deg=15.0,
    spiral_amplitude=0.08
)
g1.translate(dx=-15.0, dy=0.0, dz=0.0)
g1.boost(vx=0.25, vy=0.0, vz=0.0)

# Secondary Galaxy in self-consistent equilibrium (centered at X = +15 kpc moving -X)
g2 = ns.EquilibriumSpiralGalaxy(
    N_disk=N_DISK_2,
    N_bulge=N_BULGE_2,
    N_halo=N_HALO_2,
    R_d=2.8,
    z_d=0.22,
    mass_disk=0.7,
    mass_bulge=0.2,
    bulge_scale=0.5,
    mass_halo=1.6,
    halo_scale=7.0,
    Toomre_Q=1.3,
    spiral_arms=2,
    pitch_angle_deg=18.0,
    spiral_amplitude=0.08
)
# Tilt G2 slightly (30 deg) for realistic ring and splash feature formation on head-on impact
g2.rotate(angle_deg=35.0, axis='y')
g2.translate(dx=15.0, dy=0.0, dz=0.0)
g2.boost(vx=-0.35, vy=0.0, vz=0.0)

# Merge both systems
headon_collision = g1 + g2

# Export to NEXT IC format
headon_collision.save("headon_30k_20k.txt")
print("\n[SUCCESS] Generated headon_30k_20k.txt (50,000 total particles)!")
