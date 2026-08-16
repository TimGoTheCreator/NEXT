import os
import sys

# Add nextsim (c:/n2/nextsim) to python path
nextsim_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "..", "nextsim"))
if nextsim_dir not in sys.path:
    sys.path.insert(0, nextsim_dir)

import nextinsim as ns

# =============================================================================
# Isolated Equilibrium Spiral Galaxy (50,000 Particles)
# =============================================================================
N_DISK  = 15000   # Luminous exponential disk stars (PartType0/4)
N_BULGE =  2500   # Compact spherical central bulge
N_HALO  = 32500   # Massive spherical Dark Matter halo (PartType1)

print("=" * 65)
print("  NEXT - Generating Isolated Equilibrium Spiral Galaxy (50,000)")
print("=" * 65)
print(f"  • Stellar Disk:        {N_DISK:,} stars (R_d = 4.0, z_d = 0.25)")
print(f"  • Central Bulge:        {N_BULGE:,} stars (R_b = 0.8)")
print(f"  • Dark Matter Halo:    {N_HALO:,} particles (M_halo = 5.0, R_h = 12.0)")
print("  • Toomre Q Stability:   1.3 (Stable against bar collapse)")
print("  • Spiral Pattern:       2-Arm Grand Design Wave")
print("=" * 65)

galaxy = ns.EquilibriumSpiralGalaxy(
    N_disk=N_DISK,
    N_bulge=N_BULGE,
    N_halo=N_HALO,
    R_d=4.0,
    z_d=0.25,
    mass_disk=1.0,
    mass_bulge=0.2,
    bulge_scale=0.8,
    mass_halo=5.0,
    halo_scale=12.0,
    Toomre_Q=1.3,
    spiral_arms=2,
    pitch_angle_deg=18.0,
    spiral_amplitude=0.12
)

# Export for NEXT simulation engine
galaxy.save("spiral_galaxy.txt")
print("\n[SUCCESS] Generated spiral_galaxy.txt! Ready to simulate.")
