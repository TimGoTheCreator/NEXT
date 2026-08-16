import os
import sys

# Add nextsim to python path
nextsim_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "..", "nextsim"))
if nextsim_dir not in sys.path:
    sys.path.insert(0, nextsim_dir)

import nextinsim as ns

# =============================================================================
# M51 Whirlpool Galaxy - 25,000 Fast Particles Long-Duration Run
# Designed for deep cosmic time evolution (500+ Dumps)
# =============================================================================
TOTAL_N = 25000

# 1. Primary Galaxy (M51) - 21,000 particles
N_DISK  = 10000   # Luminous disk stars (PartType0/4)
N_BULGE =  2000   # Central bulge
N_HALO  =  9000   # Dark matter halo (PartType1)

# 2. Companion Dwarf Satellite (NGC 5195) - 4,000 particles
N_COMPANION = 4000
MASS_MAIN = 5.2
MASS_COMP = 1.7

print("=" * 70)
print("  NEXT - Generating M51 Whirlpool 25k (Fast Long-Duration Setup)")
print("======================================================================")
print(f"  • Primary M51 Disk:     {N_DISK:,} stars (R_d = 4.0, z_d = 0.25)")
print(f"  • Primary Bulge & Halo: {N_BULGE + N_HALO:,} particles (M_main = {MASS_MAIN})")
print(f"  • Companion NGC 5195:   {N_COMPANION:,} particles (M_comp = {MASS_COMP})")
print("  • Orbital Trajectory:   Wide Prograde Arc (L_z = +10.92, R_p ~ 11.5 kpc)")
print("=" * 70)

# Generate primary galaxy (in equilibrium, no pre-baked spiral arms)
m51 = ns.EquilibriumSpiralGalaxy(
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
    spiral_arms=0,
    spiral_amplitude=0.0
)

# Generate companion dwarf satellite (Hernquist sphere)
companion = ns.Hernquist(
    N=N_COMPANION,
    scale=1.2,
    total_mass=MASS_COMP,
    dm_fraction=0.85
)

# Wide prograde grazing orbit (L_z = +10.92)
X_0, Y_0, Z_0 = 0.0, -26.0, 5.0
VX_0, VY_0, VZ_0 = 0.42, 0.10, -0.02

companion.translate(dx=X_0, dy=Y_0, dz=Z_0)
companion.boost(vx=VX_0, vy=VY_0, vz=VZ_0)

# Combine systems
whirlpool_25k = m51 + companion

# Export Initial Conditions
whirlpool_25k.save("whirlpool_25k.txt")
print("\n[SUCCESS] Generated whirlpool_25k.txt (25,000 particles)! Ready for long run.")
