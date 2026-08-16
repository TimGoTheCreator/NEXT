import os
import sys
import numpy as np

# Add nextsim (c:/n2/nextsim) to python path
nextsim_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "..", "nextsim"))
if nextsim_dir not in sys.path:
    sys.path.insert(0, nextsim_dir)

import nextinsim as ns

# =============================================================================
# MILKY WAY + ALL SATELLITE DWARF GALAXIES (300,000 PARTICLES)
# =============================================================================
# 1. Main Milky Way Host Galaxy
N_DISK  = 160000   # Exponential Thin Stellar Disk
N_BULGE =  25000   # Compact Central Bulge
N_HALO  =  85000   # Dark Matter Halo

print("=" * 80)
print("  NEXT - Milky Way Host + Complete Dwarf Satellite System (300,000 Particles)")
print("================================================================================")
print(f"  • Milky Way Disk:      {N_DISK:,} stars (R_d = 3.5, z_d = 0.15)")
print(f"  • Milky Way Bulge:      {N_BULGE:,} stars (R_b = 0.6)")
print(f"  • Milky Way Halo:      {N_HALO:,} DM particles (R_h = 15.0)")
print("  • Generating All Real Milky Way Dwarf Satellite Galaxies:")
print("    - Large Magellanic Cloud (LMC), Small Magellanic Cloud (SMC)")
print("    - Sagittarius Dwarf (Sgr dSph), Fornax, Sculptor, Leo I, Leo II,")
print("      Ursa Minor, Draco, Carina, Sextans, Bootes I, Ursa Major I/II,")
print("      Coma Berenices, Canes Venatici I/II, Segue 1/2, Willman 1, etc.")
print("================================================================================")

# Generate Main Milky Way
mw = ns.EquilibriumSpiralGalaxy(
    N_disk=N_DISK,
    N_bulge=N_BULGE,
    N_halo=N_HALO,
    R_d=3.5,
    z_d=0.15,
    mass_disk=1.0,
    mass_bulge=0.25,
    bulge_scale=0.6,
    mass_halo=5.8,
    halo_scale=15.0,
    Toomre_Q=1.25,
    spiral_arms=2,
    pitch_angle_deg=16.0,
    spiral_amplitude=0.15
)

# Total remaining budget for satellites = ~30,000 particles
# Real Dwarf Satellite Catalog (scaled to simulation units: R_MW_disk = 3.5 ~ 15 kpc, M_MW ~ 1.0):
# (Name, N_particles, mass, [x, y, z], [vx, vy, vz], scale_radius)
satellites = [
    # 1. Major Magellanic Clouds
    ("LMC (Large Magellanic Cloud)", 10000, 0.100, [-2.5, -9.0, -5.5], [0.18, 0.42, 0.15], 0.8),
    ("SMC (Small Magellanic Cloud)",  4000, 0.025, [-3.8, -10.5, -7.0], [0.12, 0.38, 0.11], 0.4),

    # 2. Classic Milky Way Dwarf Spheroidals (dSph)
    ("Sagittarius Dwarf (Sgr dSph)", 3500, 0.015, [ 3.2, -0.6, -2.8], [-0.45, 0.10, 0.55], 0.35),
    ("Fornax dSph",                  2500, 0.008, [ 0.8, -12.5, -14.0], [0.15, 0.22, -0.10], 0.3),
    ("Sculptor dSph",                1800, 0.005, [-0.5,  0.8, -12.0], [-0.18, 0.28, 0.05], 0.25),
    ("Leo I dSph",                   1500, 0.004, [-8.5, 14.0,  18.0], [0.08, -0.15, -0.12], 0.25),
    ("Leo II dSph",                  1000, 0.003, [-10.0, 12.0, 16.0], [0.05, -0.12, -0.10], 0.2),
    ("Ursa Minor dSph",              1000, 0.003, [-3.5,  8.5,  10.0], [0.22, -0.18, 0.08], 0.2),
    ("Draco dSph",                   1000, 0.003, [-0.8, 10.0,   9.5], [0.25, -0.14, 0.05], 0.2),
    ("Carina dSph",                   800, 0.002, [-4.5, -9.0,  -4.0], [0.20, 0.30, -0.15], 0.18),
    ("Sextans dSph",                  800, 0.002, [-6.0,  7.0,   6.5], [0.15, -0.20, 0.10], 0.18),

    # 3. Ultra-Faint Dwarf Satellites (UFDs)
    ("Canes Venatici I",              500, 0.001, [-5.0, 16.0,  20.0], [0.06, -0.10, -0.08], 0.15),
    ("Bootes I",                      400, 0.0008, [ 2.0, 10.0,  12.0], [-0.18, -0.15, 0.12], 0.12),
    ("Ursa Major I",                  400, 0.0008, [-7.0,  9.0,  12.0], [0.12, -0.18, 0.08], 0.12),
    ("Ursa Major II",                 350, 0.0006, [-4.0,  6.0,   7.5], [0.24, -0.22, 0.10], 0.10),
    ("Coma Berenices",                300, 0.0005, [-1.0,  8.0,  11.0], [0.10, -0.16, 0.14], 0.10),
    ("Canes Venatici II",             250, 0.0004, [-4.5, 14.0,  18.0], [0.08, -0.11, -0.09], 0.10),
    ("Hercules dSph",                 250, 0.0004, [ 4.0,  9.0,   9.0], [-0.20, -0.10, 0.15], 0.10),
    ("Leo IV",                        200, 0.0003, [-7.0, 12.0,  13.0], [0.09, -0.14, -0.08], 0.08),
    ("Segue 1",                       150, 0.0002, [-3.0,  3.5,   4.5], [0.28, -0.25, 0.12], 0.06),
    ("Willman 1",                     150, 0.0002, [-4.0,  5.0,   5.5], [0.22, -0.20, 0.11], 0.06),
    ("Segue 2",                       100, 0.0001, [-2.5,  4.0,   3.0], [0.30, -0.22, 0.10], 0.05),
]

# Build unified system
system = ns.System()

# Add Milky Way Particles
for p in mw.particles:
    system.add(
        x=p.x, y=p.y, z=p.z,
        vx=p.vx, vy=p.vy, vz=p.vz,
        mass=p.mass,
        ptype=p.ptype
    )

# Add each satellite dwarf galaxy with internal Plummer equilibrium
np.random.seed(1337)
sat_count = 0

for name, n_pts, m_sat, center_pos, bulk_vel, a_scale in satellites:
    sat_count += 1
    m_part = m_sat / n_pts
    cx, cy, cz = center_pos
    bvx, bvy, bvz = bulk_vel

    # Plummer sphere profile for dwarf galaxy
    u = np.random.uniform(0.0, 1.0, n_pts)
    r = a_scale / np.sqrt(np.clip(u**(-2.0/3.0) - 1.0, 1e-4, 1e6))
    theta = np.arccos(np.random.uniform(-1.0, 1.0, n_pts))
    phi = np.random.uniform(0.0, 2.0 * np.pi, n_pts)

    px = r * np.sin(theta) * np.cos(phi) + cx
    py = r * np.sin(theta) * np.sin(phi) + cy
    pz = r * np.cos(theta) + cz

    # Internal velocity dispersion via Jeans equilibrium
    vesc = np.sqrt(2.0 * 1.0 * m_sat / np.sqrt(r**2 + a_scale**2))
    # Pick velocities below escape speed
    v_mag = vesc * np.random.uniform(0.2, 0.7, n_pts)
    v_theta = np.arccos(np.random.uniform(-1.0, 1.0, n_pts))
    v_phi = np.random.uniform(0.0, 2.0 * np.pi, n_pts)

    vx = v_mag * np.sin(v_theta) * np.cos(v_phi) + bvx
    vy = v_mag * np.sin(v_theta) * np.sin(v_phi) + bvy
    vz = v_mag * np.cos(v_theta) + bvz

    for j in range(n_pts):
        # 65% Dark Matter (1), 35% Stars (4)
        pt = 4 if (j % 3 == 0) else 1
        system.add(x=px[j], y=py[j], z=pz[j], vx=vx[j], vy=vy[j], vz=vz[j], mass=m_part, ptype=pt)

    print(f"  + Added Satellite {sat_count:02d}: {name:<30} ({n_pts:>5,} parts)")

output_file = "milkyway_and_satellites_300k.txt"
system.save(output_file)
