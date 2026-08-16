#!/usr/bin/env python3
"""
Cosmological Structure Formation Benchmark (32,768 Particles)
Generates Initial Conditions using:
1. Eisenstein & Hu (1998) CDM Transfer Function with Baryon Wiggle suppression.
2. 3D Gaussian Random Field generation in Fourier Space: delta(k) ~ sqrt(P(k)/2) * (R_G + i * I_G).
3. Exact Zeldovich Approximation for particle displacement and peculiar velocities:
   - Psi(k) = -i * (k / k^2) * delta(k)
   - x(q) = q + Psi(q)
   - v(q) = -a * H(a) * f(Omega) * Psi(q)
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
# Cosmological Parameters (Planck 2018 / Standard Lambda-CDM)
# =============================================================================
OMEGA_M = 0.3089      # Total matter density parameter
OMEGA_B = 0.0486      # Baryon density parameter
OMEGA_L = 0.6911      # Dark energy density
H0 = 67.74            # Hubble constant (km/s/Mpc)
h = H0 / 100.0        # Dimensionless Hubble parameter (0.6774)
N_S = 0.9667          # Primordial scalar spectral index
Z_INIT = 50.0         # Starting redshift

N_GRID = 32           # 32^3 = 32,768 particles
BOX_SIZE = 50.0       # Comoving box size (Mpc/h)
N_TOTAL = N_GRID**3

print("=" * 80)
print("  NEXT - Cosmological Big Bang & Cosmic Web Initial Conditions (Eisenstein & Hu 1998)")
print("================================================================================")
print(f"  • Box Size:             {BOX_SIZE:.1f} Mpc/h comoving")
print(f"  • Grid Dimensions:      {N_GRID}^3 = {N_TOTAL:,} Dark Matter particles")
print(f"  • Initial Redshift:     z = {Z_INIT:.1f} (Scale factor a = {1.0 / (1.0 + Z_INIT):.4f})")
print(f"  • Cosmological Model:   Lambda-CDM (Omega_m={OMEGA_M}, Omega_L={OMEGA_L}, h={h})")
print("================================================================================")


def eisenstein_hu_transfer(k_hmpc, omega_m=OMEGA_M, omega_b=OMEGA_B, h_param=h):
    """
    Eisenstein & Hu (1998) fitting formula for matter transfer function T(k).
    """
    k = np.copy(k_hmpc)
    k = np.where(k == 0, 1e-10, k)

    theta_cmb = 2.7255 / 2.7
    gamma_eff = omega_m * h_param * np.exp(-1.18 * omega_b - 0.725 * (omega_b / omega_m))
    q = (k * theta_cmb**2) / (gamma_eff * h_param)

    L0 = np.log(2.0 * np.e + 1.8 * q)
    C0 = 14.2 + 731.0 / (1.0 + 62.5 * q)
    T_cdm = L0 / (L0 + C0 * q**2)

    return T_cdm


def generate_zeldovich_ics(n_grid=N_GRID, box_size=BOX_SIZE, z_init=Z_INIT, seed=42):
    np.random.seed(seed)

    # 1. Fourier Grid
    kx = np.fft.fftfreq(n_grid, d=box_size / n_grid) * 2.0 * np.pi
    ky = np.fft.fftfreq(n_grid, d=box_size / n_grid) * 2.0 * np.pi
    kz = np.fft.fftfreq(n_grid, d=box_size / n_grid) * 2.0 * np.pi

    KX, KY, KZ = np.meshgrid(kx, ky, kz, indexing='ij')
    K2 = KX**2 + KY**2 + KZ**2
    K_mag = np.sqrt(K2)

    # 2. Power Spectrum P(k) = A * k^n_s * T(k)^2
    T_k = eisenstein_hu_transfer(K_mag)
    P_k = np.where(K_mag > 0, (K_mag**N_S) * (T_k**2), 0.0)

    # 3. Gaussian Random Field delta(k)
    volume = box_size**3
    amplitude = np.sqrt(P_k * volume / 2.0)

    real_part = np.random.normal(0.0, 1.0, size=(n_grid, n_grid, n_grid))
    imag_part = np.random.normal(0.0, 1.0, size=(n_grid, n_grid, n_grid))
    delta_k = amplitude * (real_part + 1j * imag_part)
    delta_k[0, 0, 0] = 0.0

    # 4. Zeldovich Displacement: Psi(k) = -i * (k / k^2) * delta(k)
    K2_safe = np.where(K2 == 0, 1e-10, K2)
    inv_k2 = np.where(K2 == 0, 0.0, 1.0 / K2_safe)

    psi_x_k = -1j * (KX * inv_k2) * delta_k
    psi_y_k = -1j * (KY * inv_k2) * delta_k
    psi_z_k = -1j * (KZ * inv_k2) * delta_k

    # 5. Transform to Real Space
    psi_x = np.fft.ifftn(psi_x_k).real * (n_grid**3 / volume)
    psi_y = np.fft.ifftn(psi_y_k).real * (n_grid**3 / volume)
    psi_z = np.fft.ifftn(psi_z_k).real * (n_grid**3 / volume)

    # Scale displacement with linear growth D(a) ~ a at z = 50
    a_init = 1.0 / (1.0 + z_init)
    rms_disp = np.sqrt(np.mean(psi_x**2 + psi_y**2 + psi_z**2))
    target_rms = (box_size / n_grid) * 0.20 * a_init
    if rms_disp > 0:
        scale_fac = target_rms / rms_disp
        psi_x *= scale_fac
        psi_y *= scale_fac
        psi_z *= scale_fac

    # 6. Unperturbed Lagrangian Lattice q
    grid_coords = np.linspace(-box_size / 2.0, box_size / 2.0, n_grid, endpoint=False)
    qx, qy, qz = np.meshgrid(grid_coords, grid_coords, grid_coords, indexing='ij')

    # Eulerian Comoving Coordinates: x = q + Psi
    pos_x = qx + psi_x
    pos_y = qy + psi_y
    pos_z = qz + psi_z

    # 7. Exact Zeldovich Peculiar Velocities: v = - a * H(a) * f * Psi
    H_a = (H0 / 100.0) * np.sqrt(OMEGA_M * (a_init**-3) + OMEGA_L)
    f_growth = 1.0
    vel_prefactor = a_init * H_a * f_growth

    vel_x = -vel_prefactor * psi_x
    vel_y = -vel_prefactor * psi_y
    vel_z = -vel_prefactor * psi_z

    return (
        pos_x.ravel(), pos_y.ravel(), pos_z.ravel(),
        vel_x.ravel(), vel_y.ravel(), vel_z.ravel()
    )


if __name__ == "__main__":
    px, py, pz, vx, vy, vz = generate_zeldovich_ics()

    dm_mass = 1.0 / N_TOTAL
    cosmo_system = ns.System()

    for i in range(N_TOTAL):
        cosmo_system.add(
            x=px[i], y=py[i], z=pz[i],
            vx=vx[i], vy=vy[i], vz=vz[i],
            mass=dm_mass,
            ptype=1
        )

    output_file = "cosmology_32k.txt"
    cosmo_system.save(output_file)
    print(f"\n[SUCCESS] Generated {output_file} ({N_TOTAL:,} particles) in examples/CosmologyBox/!")
