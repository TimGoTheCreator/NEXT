#!/usr/bin/env python3
"""
Hyper-Dense Primordial Big Bang Shockwave & Global Fragmentation (100,000 Particles)
Physics:
1. Singularity Core: 100,000 particles packed into a superdense micro-core (R_0 ~ 1.2).
2. Ultra-Relativistic Hubble Blast: Supersonic radial explosion profile v_r(r) ~ H0 * r.
3. High-Density Power Spectrum (Sedov-Taylor Blast + 3D Multi-Harmonic Turbulence):
   - Overdense turbulent shock ripples trigger simultaneous fragmentation across the expanding shell.
   - Gravitational instability condenses hundreds of filamentary star clusters and rotating knots
     as the shockwave expands from R = 1.2 to R = 60.0!
"""

import os
import sys
import numpy as np

# Add nextsim to python path
nextsim_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "nextsim"))
if nextsim_dir not in sys.path:
    sys.path.insert(0, nextsim_dir)

import nextinsim as ns

# =============================================================================
# Primordial Fireball Parameters
# =============================================================================
N_PARTICLES = 100000    # 100,000 high-resolution particles
R_CORE = 1.2            # Ultra-dense micro-core radius (the primordial fireball)
H_BLAST = 1.65          # High-speed Hubble detonation rate
TOTAL_MASS = 8.0        # Gravitating mass

print("=" * 80)
print("  NEXT - Hyper-Dense Primordial Big Bang Detonation (100,000 Particles)")
print("================================================================================")
print(f"  • Particle Resolution:   {N_PARTICLES:,} particles")
print(f"  • Primordial Fireball:   R_0 = {R_CORE:.2f} (Extreme Initial Density)")
print(f"  • Shockwave Expansion:   H_0 = {H_BLAST:.2f} (Supersonic Blast Wave)")
print("================================================================================")

np.random.seed(42)

# 1. Distribute particles into a dense spherical fireball with r^-1 density cusp
u = np.random.uniform(0.001, 1.0, N_PARTICLES)
r = R_CORE * (u**(1.0 / 2.0)) # Cuspy center

costheta = np.random.uniform(-1.0, 1.0, N_PARTICLES)
sintheta = np.sqrt(1.0 - costheta**2)
phi = np.random.uniform(0.0, 2.0 * np.pi, N_PARTICLES)

x = r * sintheta * np.cos(phi)
y = r * sintheta * np.sin(phi)
z = r * costheta

# 2. 3D Multi-Harmonic Turbulent Density Ripple Field (10 octaves of turbulence)
turb_phase_x = np.zeros(N_PARTICLES)
turb_phase_y = np.zeros(N_PARTICLES)
turb_phase_z = np.zeros(N_PARTICLES)

for k_freq, weight in [(3.0, 0.25), (7.0, 0.18), (14.0, 0.12), (28.0, 0.08), (56.0, 0.04)]:
    turb_phase_x += weight * np.sin(k_freq * (y + z)) * np.cos(k_freq * x)
    turb_phase_y += weight * np.sin(k_freq * (z + x)) * np.cos(k_freq * y)
    turb_phase_z += weight * np.sin(k_freq * (x + y)) * np.cos(k_freq * z)

# Apply turbulent displacement
r_safe = r + 1e-4
x += turb_phase_x * (x / r_safe) * 0.15
y += turb_phase_y * (y / r_safe) * 0.15
z += turb_phase_z * (z / r_safe) * 0.15

# 3. Super-Hubble Relativistic Shock Blast (v = H_0 * r)
vx = H_BLAST * x
vy = H_BLAST * y
vz = H_BLAST * z

# 4. Tangential Curl / Vorticity (Tidal Torque that makes clumps spin into real disks)
curl_x = (turb_phase_y - turb_phase_z) * 0.45
curl_y = (turb_phase_z - turb_phase_x) * 0.45
curl_z = (turb_phase_x - turb_phase_y) * 0.45

vx += curl_x + np.random.normal(0.0, 0.02, N_PARTICLES)
vy += curl_y + np.random.normal(0.0, 0.02, N_PARTICLES)
vz += curl_z + np.random.normal(0.0, 0.02, N_PARTICLES)

# 5. Export Initial Conditions
particle_mass = TOTAL_MASS / N_PARTICLES
bigbang_system = ns.System()

for i in range(N_PARTICLES):
    bigbang_system.add(
        x=x[i], y=y[i], z=z[i],
        vx=vx[i], vy=vy[i], vz=vz[i],
        mass=particle_mass,
        ptype=4
    )

output_file = "bigbang_100k.txt"
bigbang_system.save(output_file)
print(f"\n[SUCCESS] Generated {output_file} (100,000 particles) for high-density detonation!")
