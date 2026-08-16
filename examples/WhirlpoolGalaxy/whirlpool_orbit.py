import os
import sys

# Add nextsim to python path
nextsim_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "..", "nextsim"))
if nextsim_dir not in sys.path:
    sys.path.insert(0, nextsim_dir)

import nextinsim as ns

# =============================================================================
# M51 Whirlpool Galaxy + Looping Grazing Satellite Orbit (50,000 Particles)
# High Angular Momentum Prograde Flyby (Pericenter outside disk core: R_p ~ 11 kpc)
# =============================================================================
TOTAL_N = 50000

# 1. Primary Spiral Galaxy (M51 / NGC 5194) - 42,000 particles
N_DISK  = 18000   # Luminous disk stars (PartType0/4)
N_BULGE =  4000   # Compact central bulge
N_HALO  = 20000   # Dark matter halo (PartType1)

# 2. Companion Dwarf Galaxy (NGC 5195) - 8,000 particles
N_COMPANION = 8000
MASS_MAIN = 5.2       # Disk(1.0) + Bulge(0.2) + Halo(4.0)
MASS_COMP = 1.7       # ~1/3 mass ratio

print("=" * 70)
print("  NEXT - Generating M51 Whirlpool Wide Grazing Satellite Orbit")
print("  (High Angular Momentum L_z = 11.0, Pericenter R_p ~ 11.5 kpc)")
print("=" * 70)
print(f"  • Primary M51 Disk:     {N_DISK:,} stars (R_d = 4.0, z_d = 0.25)")
print(f"  • Primary Bulge & Halo: {N_BULGE + N_HALO:,} particles (M_main = {MASS_MAIN})")
print(f"  • Companion NGC 5195:   {N_COMPANION:,} particles (M_comp = {MASS_COMP})")
print("  • Orbital Trajectory:   Wide Prograde Arc (Sweeps around disk edge, NO PLUNGE)")
print("=" * 70)

# Generate primary galaxy (in equilibrium)
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
# Starts at Y = -26.0, moves with tangential velocity VX = +0.42
# Sweeps past the disk edge at R ~ 11.5 kpc and loops around to the top
X_0, Y_0, Z_0 = 0.0, -26.0, 5.0
VX_0, VY_0, VZ_0 = 0.42, 0.10, -0.02

companion.translate(dx=X_0, dy=Y_0, dz=Z_0)
companion.boost(vx=VX_0, vy=VY_0, vz=VZ_0)

# Combine systems
whirlpool_orbit_system = m51 + companion

# Export Initial Conditions
whirlpool_orbit_system.save("whirlpool_orbit.txt")
print("\n[SUCCESS] Generated whirlpool_orbit.txt (50,000 particles)! Ready to simulate.")
