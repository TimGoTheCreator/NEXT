#!/usr/bin/env python3
"""
Big Bang Physical Expansion & Fragmenting Cosmic Web Benchmark (50,000 Particles)
Physics:
1. Particles start in a dense primordial sphere (R_0 ~ 3.0 kpc/Mpc).
2. Pure Physical Hubble Flow: v_Hubble = H_0 * r (Linear Outward Radial Blast).
3. Superimposed scale-dependent primordial density ripples.
4. As the universe expands outward into the void, local overdensities turn around
   under self-gravity to form spinning galaxy clusters within the expanding fireball!
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
# Big Bang Initial Condition Parameters
# =============================================================================
N_PARTICLES = 50000     # 50,000 Dark Matter / Stellar Particles
R_INITIAL = 3.5         # Initial compact radius of the dense primordial core
H_EXPANSION = 0.45      # Outward Hubble expansion rate (v = H * r)
TOTAL_MASS = 12.0       # Total gravitating mass (controls gravitational turnaround)

print("=" * 80)
print("  NEXT - Physical Big Bang & Cosmic Web Fragmentation Initial Conditions")
print("================================================================================")
print(f"  • Total Particles:       {N_PARTICLES:,}")
print(f"  • Primordial Core Size:  R_0 = {R_INITIAL:.1f} (High Density)")
print(f"  • Total Gravitating Mass: M_tot = {TOTAL_MASS:.1f}")
print(f"  • Hubble Expansion Rate: H_0 = {H_EXPANSION:.2f} (Linear Radial Blast)")
print("================================================================================")

np.random.seed(42)

# 1. Distribute particles in dense sphere with smooth radial profile + random fluctuations
# Uniform distribution in sphere with subtle center concentration
u = np.random.uniform(0.0, 1.0, N_PARTICLES)
r = R_INITIAL * (u**(1.0 / 3.0))

costheta = np.random.uniform(-1.0, 1.0, N_PARTICLES)
sintheta = np.sqrt(1.0 - costheta**2)
phi = np.random.uniform(0.0, 2.0 * np.pi, N_PARTICLES)

x = r * sintheta * np.cos(phi)
y = r * sintheta * np.sin(phi)
z = r * costheta

# 2. Multi-Scale Density Perturbations (Creates Clumping & Fragmentation)
k1, k2, k3 = 1.2, 2.8, 5.5
density_perturbation = (
    0.18 * np.sin(k1 * x) * np.cos(k1 * y) +
    0.12 * np.cos(k2 * y) * np.sin(k2 * z) +
    0.08 * np.sin(k3 * z) * np.cos(k3 * x)
)

# Apply spatial clump displacement
x += density_perturbation * (x / (r + 1e-4)) * 0.4
y += density_perturbation * (y / (r + 1e-4)) * 0.4
z += density_perturbation * (z / (r + 1e-4)) * 0.4

# 3. Pure Physical Hubble Expansion: v_vector = H_0 * r_vector
vx = H_EXPANSION * x
vy = H_EXPANSION * y
vz = H_EXPANSION * z

# 4. Superimpose Peculiar Velocities (Perturbations that trigger local rotational spin)
vx += np.random.normal(0.0, 0.04, N_PARTICLES) + 0.05 * (-y / (r + 1e-3)) * density_perturbation
vy += np.random.normal(0.0, 0.04, N_PARTICLES) + 0.05 * ( x / (r + 1e-3)) * density_perturbation
vz += np.random.normal(0.0, 0.04, N_PARTICLES)

# 5. Build Particle System
particle_mass = TOTAL_MASS / N_PARTICLES
bigbang_system = ns.System()

for i in range(N_PARTICLES):
    bigbang_system.add(
        x=x[i], y=y[i], z=z[i],
        vx=vx[i], vy=vy[i], vz=vz[i],
        mass=particle_mass,
        ptype=4 # Stellar/Luminous particles for beautiful visualization
    )

output_file = "bigbang_50k.txt"
bigbang_system.save(output_file)
print(f"\n[SUCCESS] Generated {output_file} ({N_PARTICLES:,} particles) in examples/BigBangExplosion/!")
