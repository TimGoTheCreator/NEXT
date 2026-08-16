import os
import sys
import math
import random

# Add nextsim to python path
nextsim_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "..", "nextsim"))
if nextsim_dir not in sys.path:
    sys.path.insert(0, nextsim_dir)

import nextinsim as ns

# =============================================================================
# M51 Whirlpool Galaxy + NGC 5195 Satellite Flyby (50,000 Particles)
# Based on Toomre & Toomre (1972) and Salo & Laurikainen (2000)
# =============================================================================
TOTAL_N = 50000

# 1. Primary Spiral Galaxy (M51 / NGC 5194) - 42,000 particles
N_DISK  = 18000   # Luminous disk stars (PartType0/4)
N_BULGE =  4000   # Compact central bulge
N_HALO  = 20000   # Dark matter halo (PartType1)

# 2. Companion Dwarf Galaxy (NGC 5195) - 8,000 particles
N_COMPANION = 8000
MASS_MAIN = 5.2       # Disk(1.0) + Bulge(0.2) + Halo(4.0)
MASS_COMP = 1.7       # ~1/3 mass ratio (Toomre & Toomre 1972)

print("=" * 70)
print("  NEXT - Generating M51 Whirlpool Grand-Design Spiral Galaxy")
print("  (Prograde Satellite Tidal Resonance - Toomre & Toomre 1972 / Salo 2000)")
print("=" * 70)
print(f"  • Primary M51 Disk:     {N_DISK:,} stars (R_d = 4.0, z_d = 0.25)")
print(f"  • Primary Bulge & Halo: {N_BULGE + N_HALO:,} particles (M_main = {MASS_MAIN})")
print(f"  • Companion NGC 5195:   {N_COMPANION:,} particles (M_comp = {MASS_COMP}, Mass Ratio 1:3)")
print("  • Orbital Trajectory:   Prograde inclined flyby (Pericenter ~ 2.2 R_d)")
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
    spiral_arms=0,        # Pure unperturbed disk - spiral arms will be tidally excited by companion!
    spiral_amplitude=0.0
)

# Generate companion dwarf satellite (Hernquist sphere)
companion = ns.Hernquist(
    N=N_COMPANION,
    scale=1.2,
    total_mass=MASS_COMP,
    dm_fraction=0.85
)

# Set companion onto exact prograde inclined flyby orbit
# Pericenter passage excites the m=2 Grand-Design Whirlpool spiral arms and bridge!
X_0, Y_0, Z_0 = 24.0, -16.0, 6.0
VX_0, VY_0, VZ_0 = -0.32, 0.42, -0.12

companion.translate(dx=X_0, dy=Y_0, dz=Z_0)
companion.boost(vx=VX_0, vy=VY_0, vz=VZ_0)

# Combine systems
whirlpool_system = m51 + companion

# Export Initial Conditions
whirlpool_system.save("whirlpool.txt")
print("\n[SUCCESS] Generated whirlpool.txt (50,000 particles)! Ready to simulate.")
