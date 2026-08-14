#!/usr/bin/env python3
"""
===============================================================================
Local Group Real-World 3-Body Simulation: Milky Way, Andromeda (M31), Triangulum (M33)
===============================================================================
Based on observational astrometry from Hubble Space Telescope (HST) & Gaia:
  - van der Marel et al. (2012, 2019)
  - McConnachie (2012)
  - Patel, Besla & van der Marel (2017)

Galactocentric 3D Phase-Space Coordinates (MW center at origin):
  - Milky Way:  r = (0, 0, 0) kpc,           v = (0, 0, 0) km/s
  - Andromeda:  r = (-379, 612, -283) kpc,   v = (+66, -76, +45) km/s  [d_MW = 774 kpc]
  - Triangulum: r = (-476, 491, -413) kpc,   v = (+43, +101, +139) km/s [d_M31 = 202 kpc]

Compatible with: NEXT (text), GADGET-4, SWIFT, and AREPO (HDF5).
"""

import os
import sys
import math

# Ensure project root is in Python path to import NEXTInSim
ROOT_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
if ROOT_DIR not in sys.path:
    sys.path.insert(0, ROOT_DIR)

from NEXTInSim.nextinsim import EquilibriumSpiralGalaxy, System, Particle


def generate_real_local_group(
    N_total=30000,
    unit_scale_kpc=10.0,       # 1 simulation position unit = 10 kpc
    unit_velocity_kms=100.0,   # 1 simulation velocity unit = 100 km/s
    output_txt="andromeda_triangulum_mw.txt",
    output_hdf5="andromeda_triangulum_mw.hdf5"
):
    print("=" * 75)
    print("  Local Group 3-Body IC: Milky Way + Andromeda (M31) + Triangulum (M33)")
    print("  Based on Real Hubble & Gaia Astrometry (van der Marel et al. 2012/2019)")
    print("=" * 75)

    # Particle distribution:
    # - Milky Way (MW):   35% (e.g. 10,500)
    # - Andromeda (M31):  55% (e.g. 16,500)
    # - Triangulum (M33): 10% (e.g.  3,000)
    N_mw = int(N_total * 0.35)
    N_m31 = int(N_total * 0.55)
    N_m33 = N_total - N_mw - N_m31

    # -------------------------------------------------------------------------
    # 1. Milky Way (MW) - Galactocentric Origin (0, 0, 0)
    # -------------------------------------------------------------------------
    print(f"[1/3] Building Milky Way ({N_mw:,} particles at origin)...")
    mw = EquilibriumSpiralGalaxy(
        N_disk=int(N_mw * 0.40),
        N_bulge=int(N_mw * 0.10),
        N_halo=int(N_mw * 0.50),
        R_d=3.0 / unit_scale_kpc,          # R_d = 3 kpc (~0.30 sim units)
        z_d=0.3 / unit_scale_kpc,          # z_d = 300 pc (~0.03 sim units)
        mass_disk=1.0,                     # M_disk ~ 5e10 M_sun
        mass_bulge=0.2,                    # M_bulge ~ 1e10 M_sun
        bulge_scale=0.7 / unit_scale_kpc,
        mass_halo=8.0,                     # M_halo ~ 8e11 M_sun
        halo_scale=15.0 / unit_scale_kpc,
        Toomre_Q=1.2,
        spiral_arms=2,
        pitch_angle_deg=12.0,
        spiral_amplitude=0.12,
        G=1.0
    )
    # Milky Way sits at the origin (0, 0, 0) with 0 net velocity

    # -------------------------------------------------------------------------
    # 2. Andromeda (M31) - Real Galactocentric Coordinates
    # -------------------------------------------------------------------------
    print(f"[2/3] Building Andromeda / M31 ({N_m31:,} particles at d=774 kpc)...")
    m31 = EquilibriumSpiralGalaxy(
        N_disk=int(N_m31 * 0.40),
        N_bulge=int(N_m31 * 0.10),
        N_halo=int(N_m31 * 0.50),
        R_d=5.5 / unit_scale_kpc,          # R_d = 5.5 kpc (~0.55 sim units)
        z_d=0.4 / unit_scale_kpc,          # z_d = 400 pc
        mass_disk=1.6,                     # M_disk ~ 8e10 M_sun
        mass_bulge=0.4,                    # M_bulge ~ 2e10 M_sun
        bulge_scale=1.2 / unit_scale_kpc,
        mass_halo=12.0,                    # M_halo ~ 1.2e12 M_sun
        halo_scale=22.0 / unit_scale_kpc,
        Toomre_Q=1.2,
        spiral_arms=2,
        pitch_angle_deg=14.0,
        spiral_amplitude=0.15,
        G=1.0
    )

    # Real 3D Disk Orientation: 77° inclination, 35° position angle
    m31.rotate(angle_deg=77.0, axis='x')
    m31.rotate(angle_deg=35.0, axis='z')

    # Real Galactocentric Position: (-379, +612, -283) kpc -> in sim units
    m31_x = -379.0 / unit_scale_kpc
    m31_y =  612.0 / unit_scale_kpc
    m31_z = -283.0 / unit_scale_kpc

    # Real Galactocentric Velocity: (+66, -76, +45) km/s -> in sim units
    m31_vx =  66.0 / unit_velocity_kms
    m31_vy = -76.0 / unit_velocity_kms
    m31_vz =  45.0 / unit_velocity_kms

    m31.translate(dx=m31_x, dy=m31_y, dz=m31_z)
    m31.boost(vx=m31_vx, vy=m31_vy, vz=m31_vz)

    # -------------------------------------------------------------------------
    # 3. Triangulum (M33) - Real Galactocentric Coordinates
    # -------------------------------------------------------------------------
    print(f"[3/3] Building Triangulum / M33 ({N_m33:,} particles at d_M31=202 kpc)...")
    m33 = EquilibriumSpiralGalaxy(
        N_disk=int(N_m33 * 0.50),
        N_bulge=0,                         # Pure bulgeless disk
        N_halo=int(N_m33 * 0.50),
        R_d=1.5 / unit_scale_kpc,          # R_d = 1.5 kpc (~0.15 sim units)
        z_d=0.18 / unit_scale_kpc,
        mass_disk=0.15,                    # ~10% MW mass
        mass_bulge=0.0,
        bulge_scale=0.5 / unit_scale_kpc,
        mass_halo=1.2,
        halo_scale=8.0 / unit_scale_kpc,
        Toomre_Q=1.2,
        spiral_arms=2,
        pitch_angle_deg=18.0,
        spiral_amplitude=0.14,
        G=1.0
    )

    # Real 3D Disk Orientation: 54° inclination
    m33.rotate(angle_deg=54.0, axis='x')
    m33.rotate(angle_deg=-20.0, axis='y')

    # Real Galactocentric Position: (-476, +491, -413) kpc
    m33_x = -476.0 / unit_scale_kpc
    m33_y =  491.0 / unit_scale_kpc
    m33_z = -413.0 / unit_scale_kpc

    # Real Galactocentric Velocity: (+43, +101, +139) km/s
    m33_vx =  43.0 / unit_velocity_kms
    m33_vy = 101.0 / unit_velocity_kms
    m33_vz = 139.0 / unit_velocity_kms

    m33.translate(dx=m33_x, dy=m33_y, dz=m33_z)
    m33.boost(vx=m33_vx, vy=m33_vy, vz=m33_vz)

    # -------------------------------------------------------------------------
    # 4. Merge all 3 galaxies into 1 unified Local Group system
    # -------------------------------------------------------------------------
    local_group = mw + m31 + m33

    out_dir = os.path.dirname(os.path.abspath(__file__))
    txt_path = os.path.join(out_dir, output_txt)
    local_group.save(txt_path)

    try:
        hdf5_path = os.path.join(out_dir, output_hdf5)
        local_group.save_gadget(hdf5_path, box_size=150.0)
    except Exception as e:
        print(f"Note: HDF5 export skipped ({e})")

    print("=" * 75)
    print(f"SUCCESS: Real-world Local Group generated ({len(local_group):,} particles total)!")
    print(f"Output: {txt_path}")
    print("=" * 75)


if __name__ == "__main__":
    generate_real_local_group(
        N_total=30000,
        unit_scale_kpc=10.0,
        unit_velocity_kms=100.0,
        output_txt="andromeda_triangulum_mw.txt"
    )
