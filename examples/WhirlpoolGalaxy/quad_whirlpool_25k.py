import os
import sys
import math

# Add nextsim to python path
nextsim_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "..", "nextsim"))
if nextsim_dir not in sys.path:
    sys.path.insert(0, nextsim_dir)

import nextinsim as ns

# =============================================================================
# 4-Companion Orbiting Dance: 1 Giant Galaxy + 4 Rotating Mini-Spiral Dwarfs
# North, South, East, West Satellite Ring (25,000 Total Particles)
# =============================================================================
TOTAL_N = 25000

# 1. Primary Central Spiral Galaxy (17,000 particles)
N_DISK_MAIN  = 8000   # Luminous disk stars
N_BULGE_MAIN = 2000   # Central bulge
N_HALO_MAIN  = 7000   # Dark matter halo
MASS_MAIN    = 5.2

# 2. 4 Mini-Spiral Satellites (2,000 particles each -> 8,000 total)
N_SAT = 4
N_PART_PER_SAT = 2000
MASS_SAT = 0.25       # Realistic lightweight mass (~5% of main galaxy)

print("=" * 75)
print("  NEXT - 4-Satellite Galactic Dance (North, South, East, West)")
print("  1 Central Giant Spiral + 4 Mini Rotating Disk Satellites (25,000 Particles)")
print("=" * 75)
print(f"  • Primary Central Galaxy: {N_DISK_MAIN + N_BULGE_MAIN + N_HALO_MAIN:,} particles (M = {MASS_MAIN})")
print(f"  • 4 Satellite Mini-Disks: {N_SAT * N_PART_PER_SAT:,} particles total (4 x {N_PART_PER_SAT:,}, M = {MASS_SAT} each)")
print("  • Orbital Config:         4 Orthogonal Prograde Grazing Orbits (R = 24 kpc)")
print("=" * 75)

# Generate Central Primary Galaxy (with EquilibriumSpiralGalaxy)
main_galaxy = ns.EquilibriumSpiralGalaxy(
    N_disk=N_DISK_MAIN,
    N_bulge=N_BULGE_MAIN,
    N_halo=N_HALO_MAIN,
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

# Orbital parameters for the 4 satellites:
# Orbit radius R = 24.0 kpc, circular velocity v_circ ~ sqrt(G * M_tot / R) ~ sqrt(5.2 / 24) ~ 0.465
ORBIT_R = 24.0
V_ORBIT = 0.44

# Positions and tangential velocities for (North, East, South, West):
# Rotating counter-clockwise in harmonic resonance
sat_configs = [
    {"name": "North (Top)",    "pos": (0.0,  ORBIT_R,  3.0), "vel": (-V_ORBIT,  0.0, -0.02), "rot_x": 30.0},
    {"name": "East (Right)",   "pos": ( ORBIT_R, 0.0, -2.0), "vel": ( 0.0,  V_ORBIT,  0.02), "rot_x": 45.0},
    {"name": "South (Bottom)", "pos": (0.0, -ORBIT_R, -3.0), "vel": ( V_ORBIT,  0.0,  0.02), "rot_x": 60.0},
    {"name": "West (Left)",    "pos": (-ORBIT_R, 0.0,  2.0), "vel": ( 0.0, -V_ORBIT, -0.02), "rot_x": 45.0},
]

combined_system = main_galaxy

for sat_info in sat_configs:
    # Each satellite is its own tiny Equilibrium Spiral Galaxy!
    sat_disk = ns.EquilibriumSpiralGalaxy(
        N_disk=int(N_PART_PER_SAT * 0.40),    # 800 stars
        N_bulge=int(N_PART_PER_SAT * 0.10),   # 200 bulge stars
        N_halo=int(N_PART_PER_SAT * 0.50),    # 1000 DM halo particles
        R_d=1.2,
        z_d=0.15,
        mass_disk=MASS_SAT * 0.3,
        mass_bulge=MASS_SAT * 0.1,
        bulge_scale=0.3,
        mass_halo=MASS_SAT * 0.6,
        halo_scale=2.5,
        Toomre_Q=1.2,
        spiral_arms=2,
        pitch_angle_deg=15.0,
        spiral_amplitude=0.08
    )

    # Give each mini-galaxy an authentic inclined tilt
    sat_disk.rotate(sat_info["rot_x"], axis='x')
    
    # Translate and boost into orbiting trajectory
    px, py, pz = sat_info["pos"]
    vx, vy, vz = sat_info["vel"]
    sat_disk.translate(dx=px, dy=py, dz=pz)
    sat_disk.boost(vx=vx, vy=vy, vz=vz)

    # Add to combined multi-galaxy system
    combined_system = combined_system + sat_disk

# Export Initial Conditions
output_file = "quad_whirlpool_25k.txt"
combined_system.save(output_file)
print(f"\n[SUCCESS] Generated {output_file} (25,000 particles with 4 orbiting mini-spirals)!")
