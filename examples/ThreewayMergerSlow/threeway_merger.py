#!/usr/bin/env python3
"""
===============================================================================
3-Way Tucked Sandwich Galaxy Collision (Slow-Approach Sub-Parabolic Orbit)
===============================================================================
Center-of-mass approach velocities are cut in half (50% speed):
  - Galaxy 1: vx = +0.60, vz = +0.075 (approaching from left)
  - Galaxy 2: vx = -0.60, vz = -0.075 (approaching from right)
  - Galaxy 3: vy = -0.40 (tucked plunging galaxy)

Physics Effect:
  Halving the approach speed allows mutual self-gravity to capture the 3 galaxies 
  into a graceful, swirling gravitational dance. Instead of shooting past each other, 
  they wrap around one another, stretching immense tidal spiral bridges and merging 
  into a massive unified galaxy!

Usage:
  python threeway_merger.py [N_particles]
"""

import os
import sys

# Ensure nextsim repository is in sys.path
ROOT_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "..", "nextsim"))
if ROOT_DIR not in sys.path:
    sys.path.insert(0, ROOT_DIR)

from nextinsim import EquilibriumSpiralGalaxy, System, Particle


def generate_threeway_merger_slow(
    N_total=200000,
    output_txt="threeway_merger.txt",
    output_hdf5="threeway_merger.hdf5"
):
    print("=" * 75)
    print("  Generating 3-Way Tucked Collision (50% SLOW APPROACH VELOCITIES)")
    print("  Graceful Gravitational Capture & Sweeping Tidal Spiral Dance")
    print("=" * 75)

    # Particle budget: Equal particles per galaxy
    N_per_galaxy = N_total // 3
    N_disk = int(N_per_galaxy * 0.45)
    N_bulge = int(N_per_galaxy * 0.10)
    N_halo = N_per_galaxy - N_disk - N_bulge

    # -------------------------------------------------------------------------
    # 1. Galaxy 1 (Left): at x = -10.0, z = -1.2 -> moving +X (Halved: vx = 0.60)
    # -------------------------------------------------------------------------
    print(f"[1/3] Building Galaxy 1 (Left, Slow +X approach) ({N_per_galaxy:,} particles)...")
    g1 = EquilibriumSpiralGalaxy(
        N_disk=N_disk, N_bulge=N_bulge, N_halo=N_halo,
        R_d=3.0, z_d=0.3, mass_disk=1.0, mass_bulge=0.25, bulge_scale=0.8,
        mass_halo=7.0, halo_scale=16.0, Toomre_Q=1.2,
        spiral_arms=2, pitch_angle_deg=14.0, spiral_amplitude=0.15, G=1.0
    )
    g1.rotate(angle_deg=20.0, axis='x')
    g1.translate(dx=-10.0, dy=0.0, dz=-1.2)
    g1.boost(vx=0.60, vy=0.0, vz=0.075)  # Cut in half

    # -------------------------------------------------------------------------
    # 2. Galaxy 2 (Right): at x = +10.0, z = +1.2 -> moving -X (Halved: vx = -0.60)
    # -------------------------------------------------------------------------
    print(f"[2/3] Building Galaxy 2 (Right, Slow -X approach) ({N_per_galaxy:,} particles)...")
    g2 = EquilibriumSpiralGalaxy(
        N_disk=N_disk, N_bulge=N_bulge, N_halo=N_halo,
        R_d=3.0, z_d=0.3, mass_disk=1.0, mass_bulge=0.25, bulge_scale=0.8,
        mass_halo=7.0, halo_scale=16.0, Toomre_Q=1.2,
        spiral_arms=2, pitch_angle_deg=14.0, spiral_amplitude=0.15, G=1.0
    )
    g2.rotate(angle_deg=65.0, axis='x')
    g2.rotate(angle_deg=25.0, axis='z')
    g2.translate(dx=10.0, dy=0.0, dz=1.2)
    g2.boost(vx=-0.60, vy=0.0, vz=-0.075)  # Cut in half

    # -------------------------------------------------------------------------
    # 3. Galaxy 3 (Tucked In): at x = 0.0, y = 6.0 -> plunging -Y (Halved: vy = -0.40)
    # -------------------------------------------------------------------------
    print(f"[3/3] Building Galaxy 3 (Tucked Inside, Slow -Y plunge) ({N_per_galaxy:,} particles)...")
    g3 = EquilibriumSpiralGalaxy(
        N_disk=N_disk, N_bulge=N_bulge, N_halo=N_halo,
        R_d=3.0, z_d=0.3, mass_disk=1.0, mass_bulge=0.25, bulge_scale=0.8,
        mass_halo=7.0, halo_scale=16.0, Toomre_Q=1.2,
        spiral_arms=2, pitch_angle_deg=14.0, spiral_amplitude=0.15, G=1.0
    )
    g3.rotate(angle_deg=-45.0, axis='y')
    g3.rotate(angle_deg=45.0, axis='z')
    g3.translate(dx=0.0, dy=6.0, dz=0.0)
    g3.boost(vx=0.0, vy=-0.40, vz=0.0)  # Cut in half

    # -------------------------------------------------------------------------
    # 4. Merge all 3 into unified 3-body system
    # -------------------------------------------------------------------------
    threeway_system = g1 + g2 + g3

    out_dir = os.path.dirname(os.path.abspath(__file__))
    txt_path = os.path.join(out_dir, output_txt)
    threeway_system.save(txt_path)

    try:
        hdf5_path = os.path.join(out_dir, output_hdf5)
        threeway_system.save_gadget(hdf5_path, box_size=80.0)
    except Exception as e:
        print(f"Note: HDF5 export skipped ({e})")

    print("=" * 75)
    print(f"SUCCESS: Slow 3-Way Collision IC created ({len(threeway_system):,} particles total)!")
    print(f"Output: {txt_path}")
    print("=" * 75)


if __name__ == "__main__":
    n_part = int(sys.argv[1]) if len(sys.argv) > 1 else 200000
    generate_threeway_merger_slow(
        N_total=n_part,
        output_txt="threeway_merger.txt"
    )
