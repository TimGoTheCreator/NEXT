import os
import sys

# Add nextsim to python path
nextsim_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "..", "nextsim"))
if nextsim_dir not in sys.path:
    sys.path.insert(0, nextsim_dir)

import nextinsim as ns

# =============================================================================
# M51 Whirlpool Galaxy - 100,000 Particle High-Resolution Stress Test
# =============================================================================
TOTAL_N = 100000

# 1. Primary M51 Giant Spiral (80,000 particles)
N_DISK  = 40000   # 40,000 luminous disk stars
N_BULGE =  8000   # 8,000 central bulge stars
N_HALO  = 32000   # 32,000 dark matter halo particles
MASS_MAIN = 5.2

# 2. Companion Dwarf Galaxy NGC 5195 (20,000 particles)
N_COMPANION = 20000
MASS_COMP = 1.7

print("=" * 75)
print("  NEXT - Generating M51 Whirlpool 100,000 Particles (Stress Test)")
print("===========================================================================")
print(f"  • Primary M51 Disk:     {N_DISK:,} stars (R_d = 4.0, z_d = 0.25)")
print(f"  • Primary Bulge & Halo: {N_BULGE + N_HALO:,} particles (M_main = {MASS_MAIN})")
print(f"  • Companion NGC 5195:   {N_COMPANION:,} particles (M_comp = {MASS_COMP})")
print("  • Orbital Trajectory:   Wide Prograde Grazing Arc (R_p ~ 11.5 kpc)")
print("=" * 75)

# Primary galaxy in self-consistent equilibrium
m51_100k = ns.EquilibriumSpiralGalaxy(
    N_disk=N_DISK,
    N_bulge=N_BULGE,
    N_halo=N_HALO,
    R_d=4.0,
    z_d=0.25,
    mass_disk=1.0,
    mass_bulge=0.2,
    bulge_scale=0.8,
    mass_halo=4.0,
    halo_scale=10.0,
    Toomre_Q=1.3,
    spiral_arms=2,
    pitch_angle_deg=18.0,
    spiral_amplitude=0.10
)

# Companion satellite (Hernquist sphere with 85% DM halo)
companion_100k = ns.Hernquist(
    N=N_COMPANION,
    scale=1.2,
    total_mass=MASS_COMP,
    dm_fraction=0.85
)

# Wide grazing orbit
X_0, Y_0, Z_0 = 0.0, -26.0, 5.0
VX_0, VY_0, VZ_0 = 0.42, 0.10, -0.02

companion_100k.translate(dx=X_0, dy=Y_0, dz=Z_0)
companion_100k.boost(vx=VX_0, vy=VY_0, vz=VZ_0)

# Merge systems
whirlpool_100k = m51_100k + companion_100k

# Save Initial Conditions
whirlpool_100k.save("whirlpool_100k.txt")
print("\n[SUCCESS] Generated whirlpool_100k.txt (100,000 particles)! Ready for run.")
