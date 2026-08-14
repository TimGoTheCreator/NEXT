#!/usr/bin/env python3
"""
===============================================================================
Milky Way & Andromeda (M31) Galaxy Collision IC Generator (NEXTInSim)
===============================================================================
Simulates the realistic future encounter and merger of the Milky Way and 
Andromeda (M31) galaxies using multi-component equilibrium profiles:
  - Milky Way: Exponential disk, Hernquist bulge, and Dark Matter halo.
  - Andromeda (M31): ~1.5x more massive, larger scale radius, inclined at 77°.
  - Orbit: Parabolic / hyperbolic approach with realistic impact parameter.

Compatible with: NEXT (text), GADGET-4, SWIFT, and AREPO (HDF5).
"""

import os
import sys

# Ensure project root is in Python path to import NEXTInSim
ROOT_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
if ROOT_DIR not in sys.path:
    sys.path.insert(0, ROOT_DIR)

from NEXTInSim.nextinsim import EquilibriumSpiralGalaxy, GalaxyMerger, System


def generate_milkyway_andromeda_collision(
    N_total=10000,
    separation=20.0,
    impact_parameter=4.0,
    approach_velocity=2.0,
    output_txt="andromeda_mw_collision.txt",
    output_hdf5="andromeda_mw_collision.hdf5"
):
    print("=" * 65)
    print("  Generating Milky Way - Andromeda (M31) Collision Initial Conditions")
    print("=" * 65)

    # Particle budget: ~40% for MW (4,000), ~60% for Andromeda (6,000)
    N_mw = int(N_total * 0.40)
    N_m31 = N_total - N_mw

    # -------------------------------------------------------------------------
    # 1. Milky Way Galaxy Model (4,000 particles)
    # -------------------------------------------------------------------------
    print(f"[1/3] Building Milky Way ({N_mw:,} particles)...")
    mw = EquilibriumSpiralGalaxy(
        N_disk=int(N_mw * 0.40),
        N_bulge=int(N_mw * 0.10),
        N_halo=int(N_mw * 0.50),
        R_d=3.0,                  # MW disk scale length (~3 kpc)
        z_d=0.3,                  # MW disk scale height (~300 pc)
        mass_disk=1.0,            # MW disk mass (~5e10 M_sun)
        mass_bulge=0.2,           # MW bulge mass (~1e10 M_sun)
        bulge_scale=0.7,          # MW bulge radius (~0.7 kpc)
        mass_halo=8.0,            # MW dark matter halo mass (~8e11 M_sun)
        halo_scale=15.0,          # MW halo scale radius (~15 kpc)
        Toomre_Q=1.2,
        spiral_arms=2,
        pitch_angle_deg=12.0,
        spiral_amplitude=0.12,
        G=1.0
    )

    # -------------------------------------------------------------------------
    # 2. Andromeda (M31) Galaxy Model (6,000 particles)
    # -------------------------------------------------------------------------
    print(f"[2/3] Building Andromeda / M31 ({N_m31:,} particles)...")
    m31 = EquilibriumSpiralGalaxy(
        N_disk=int(N_m31 * 0.40),
        N_bulge=int(N_m31 * 0.10),
        N_halo=int(N_m31 * 0.50),
        R_d=5.5,                  # M31 disk scale length (~5.5 kpc)
        z_d=0.4,                  # M31 disk scale height (~400 pc)
        mass_disk=1.6,            # M31 disk mass (~8e10 M_sun)
        mass_bulge=0.4,           # M31 bulge mass (~2e10 M_sun)
        bulge_scale=1.2,          # M31 bulge radius (~1.2 kpc)
        mass_halo=12.0,           # M31 dark matter halo mass (~1.2e12 M_sun)
        halo_scale=22.0,          # M31 halo scale radius (~22 kpc)
        Toomre_Q=1.2,
        spiral_arms=2,
        pitch_angle_deg=14.0,
        spiral_amplitude=0.15,
        G=1.0
    )

    # Tilt Andromeda's galactic plane to match real-world 77° inclination
    print("      Applying 3D inclination & orientation to Andromeda disk...")
    m31.rotate(angle_deg=77.0, axis='x')
    m31.rotate(angle_deg=35.0, axis='z')

    # -------------------------------------------------------------------------
    # 3. Setup Orbital Collision Dynamics
    # -------------------------------------------------------------------------
    print(f"[3/3] Setting up collision orbital trajectory (separation={separation}, b={impact_parameter}, v_rel={approach_velocity})...")
    merger = GalaxyMerger(
        primary_sys=mw,
        secondary_sys=m31,
        separation=separation,
        impact_parameter=impact_parameter,
        relative_velocity=approach_velocity
    )

    # -------------------------------------------------------------------------
    # 4. Save Initial Conditions
    # -------------------------------------------------------------------------
    out_dir = os.path.dirname(os.path.abspath(__file__))
    txt_path = os.path.join(out_dir, output_txt)
    merger.save(txt_path)

    try:
        hdf5_path = os.path.join(out_dir, output_hdf5)
        merger.save_gadget(hdf5_path, box_size=separation * 4.0)
    except Exception as e:
        print(f"Note: HDF5 export skipped ({e})")

    print("=" * 65)
    print(f"SUCCESS: Andromeda - Milky Way IC generated ({len(merger):,} particles total)!")
    print(f"Output: {txt_path}")
    print("=" * 65)


if __name__ == "__main__":
    generate_milkyway_andromeda_collision(
        N_total=10000,
        separation=20.0,
        impact_parameter=4.0,
        approach_velocity=2.0,
        output_txt="andromeda_mw_collision.txt"
    )
