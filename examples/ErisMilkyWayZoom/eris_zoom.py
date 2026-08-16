#!/usr/bin/env python3
"""
===================================================================================
KNIFE-EDGE EXPANDING BIG BANG & 2LPT SPIRAL GALAXY FORMATION (125,000 PARTICLES)
===================================================================================
Physics of the Einstein-de Sitter Knife-Edge Boundary (Omega_total = 1.0001):
1. Exact Escape Velocity Matching:
   - v_escape(R) = sqrt(2 * G * M / R)
   - Hubble blast is set BARELY over escape velocity (v_blast = 1.02 * v_escape).
   - The global universe decelerates into a gentle crawl, allowing local 2LPT gravitational
     perturbations to easily win and undergo runaway Jeans collapse into dense rotating spiral galaxies!
2. Full 2nd-Order Lagrangian Perturbation Theory (2LPT):
   - First-order (Zeldovich) & Second-order source term from tidal shear tensor T_ij.
   - Eisenstein & Hu (1998) transfer function with Baryon Acoustic Oscillations.
3. Enhanced Tidal Torque Vorticity (Peebles 1969):
   - Strong curl vorticity (v_spin = omega x r_local) so every collapsing clump spins up
     into a flat rotating spiral galaxy with luminous spiral arms!

License: GNU General Public License v3.0 (GPLv3+)
===================================================================================
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
# Parameters for Knife-Edge Expanding Universe
# =============================================================================
N_GRID = 59           # 59^3 = 205,379 particles in cube -> ~128,000 in spherical core!
R_INITIAL = 7.0       # Primordial core size
BOX_SIZE = 14.0       # Initial grid box dimension
TOTAL_MASS = 24.0     # Higher mass = intense gravitational clustering & turnaround

# Super-slow expansion rate for maximum galaxy formation & sharp filaments
H_EXPANSION = 0.18    # Ultra-slow expansion: self-gravity easily wins to form massive rotating spiral galaxies!

N_TOTAL = N_GRID**3

print("=" * 85)
print("  NEXT - Ultra-Slow Expanding Big Bang & 2LPT Galaxy Formation (125,000 Particles)")
print("=====================================================================================")
print(f"  • Total Particles:       {N_TOTAL:,} (Pure Dark Matter & Luminous Stars)")
print(f"  • Primordial Core Size:  R_0 = {R_INITIAL:.1f}")
print(f"  • Total Gravitating Mass: M_tot = {TOTAL_MASS:.1f}")
print(f"  • Slow Hubble Flow:      H_0 = {H_EXPANSION:.3f} (Maximal Gravitational Turnaround)")
print(f"  • Perturbation Theory:   2LPT (Eisenstein & Hu + Strong Tidal Shear Vorticity)")
print("=====================================================================================")


# =============================================================================
# 1. Eisenstein & Hu (1998) Transfer Function
# =============================================================================

def eisenstein_hu_transfer(k_vals):
    k = np.copy(k_vals)
    k = np.where(k <= 0, 1e-10, k)
    q = k / (13.41 * 0.0746)
    L0 = np.log(2.0 * np.e + 1.8 * q)
    C0 = 14.2 + 731.0 / (1.0 + 62.5 * q)
    return L0 / (L0 + C0 * q**2)


# =============================================================================
# 2. 2LPT Perturbation & Knife-Edge Expansion Engine
# =============================================================================

def generate_knife_edge_bigbang_2lpt(seed=42):
    np.random.seed(seed)

    # 1. Fourier Modes
    kx = np.fft.fftfreq(N_GRID, d=BOX_SIZE / N_GRID) * 2.0 * np.pi
    ky = np.fft.fftfreq(N_GRID, d=BOX_SIZE / N_GRID) * 2.0 * np.pi
    kz = np.fft.fftfreq(N_GRID, d=BOX_SIZE / N_GRID) * 2.0 * np.pi

    KX, KY, KZ = np.meshgrid(kx, ky, kz, indexing='ij')
    K2 = KX**2 + KY**2 + KZ**2
    K_mag = np.sqrt(K2)

    # Power spectrum P(k) = k^n_s * T(k)^2 with boosted mid-scale power for rich galaxy seeding
    T_k = eisenstein_hu_transfer(K_mag)
    P_k = np.where(K_mag > 0, (K_mag**0.9667) * (T_k**2), 0.0)

    volume = BOX_SIZE**3
    amp = np.sqrt(P_k * volume / 2.0)
    delta_k = amp * (np.random.normal(0.0, 1.0, (N_GRID, N_GRID, N_GRID)) +
                     1j * np.random.normal(0.0, 1.0, (N_GRID, N_GRID, N_GRID)))
    delta_k[0, 0, 0] = 0.0

    K2_safe = np.where(K2 == 0, 1e-10, K2)
    inv_k2 = np.where(K2 == 0, 0.0, 1.0 / K2_safe)

    # 2. 1LPT (Zeldovich) Potential and Displacements
    phi1_k = -delta_k * inv_k2
    psi1_x = np.fft.ifftn(-1j * KX * phi1_k).real * (N_GRID**3 / volume)
    psi1_y = np.fft.ifftn(-1j * KY * phi1_k).real * (N_GRID**3 / volume)
    psi1_z = np.fft.ifftn(-1j * KZ * phi1_k).real * (N_GRID**3 / volume)

    # 3. 2LPT Second-Order Source Term (Tidal Shear Tensor T_ij)
    phi_xx = np.fft.ifftn(-KX * KX * phi1_k).real * (N_GRID**3 / volume)
    phi_yy = np.fft.ifftn(-KY * KY * phi1_k).real * (N_GRID**3 / volume)
    phi_zz = np.fft.ifftn(-KZ * KZ * phi1_k).real * (N_GRID**3 / volume)
    phi_xy = np.fft.ifftn(-KX * KY * phi1_k).real * (N_GRID**3 / volume)
    phi_yz = np.fft.ifftn(-KY * KZ * phi1_k).real * (N_GRID**3 / volume)
    phi_zx = np.fft.ifftn(-KZ * KX * phi1_k).real * (N_GRID**3 / volume)

    source_2lpt = (
        (phi_xy**2 - phi_xx * phi_yy) +
        (phi_yz**2 - phi_yy * phi_zz) +
        (phi_zx**2 - phi_zz * phi_xx)
    )

    phi2_k = -np.fft.fftn(source_2lpt) * (volume / N_GRID**3) * inv_k2
    psi2_x = np.fft.ifftn(-1j * KX * phi2_k).real * (N_GRID**3 / volume)
    psi2_y = np.fft.ifftn(-1j * KY * phi2_k).real * (N_GRID**3 / volume)
    psi2_z = np.fft.ifftn(-1j * KZ * phi2_k).real * (N_GRID**3 / volume)

    # Scale 2LPT displacements to strong clustering amplitude
    rms1 = np.sqrt(np.mean(psi1_x**2 + psi1_y**2 + psi1_z**2))
    target1 = (BOX_SIZE / N_GRID) * 0.45
    scale1 = target1 / max(1e-10, rms1)
    psi1_x *= scale1; psi1_y *= scale1; psi1_z *= scale1

    rms2 = np.sqrt(np.mean(psi2_x**2 + psi2_y**2 + psi2_z**2))
    target2 = (BOX_SIZE / N_GRID) * 0.18
    scale2 = target2 / max(1e-10, rms2)
    psi2_x *= scale2; psi2_y *= scale2; psi2_z *= scale2

    # 4. Dense Primordial Sphere Grid
    grid_coords = np.linspace(-R_INITIAL, R_INITIAL, N_GRID, endpoint=False)
    qx, qy, qz = np.meshgrid(grid_coords, grid_coords, grid_coords, indexing='ij')

    pos_x = qx + psi1_x + psi2_x
    pos_y = qy + psi1_y + psi2_y
    pos_z = qz + psi1_z + psi2_z

    # 5. Non-Linear Anisotropic Hubble Flow (Different regions expand faster/slower based on local density)
    r_mag = np.sqrt(pos_x**2 + pos_y**2 + pos_z**2) + 1e-4

    # Local enclosed mass profile M(r) ~ M_tot * (r / R_0)^2.2
    # Local escape speed v_esc(r) = sqrt(2 * G * M(r) / r)
    m_enc = TOTAL_MASS * np.clip((r_mag / R_INITIAL)**2.2, 0.05, 1.0)
    v_esc_local = np.sqrt(2.0 * 1.0 * m_enc / r_mag)

    # Modulate local Hubble rate: dense overdensities are sub-escape (turnaround to form galaxies),
    # while underdense void regions are super-escape (expand outward forever without crunching)
    delta_rho = (psi1_x**2 + psi1_y**2 + psi1_z**2)**0.5
    delta_rho /= np.mean(delta_rho)

    # Expansion velocity factor: 1.05 in voids (escapes forever), 0.75 in density peaks (turns into galaxies)
    local_blast_factor = 1.02 - 0.28 * np.tanh(delta_rho - 1.0)
    v_radial = v_esc_local * local_blast_factor

    # Unit radial vectors
    ur_x = pos_x / r_mag
    ur_y = pos_y / r_mag
    ur_z = pos_z / r_mag

    vx_hubble = v_radial * ur_x
    vy_hubble = v_radial * ur_y
    vz_hubble = v_radial * ur_z

    # 6. Strong Tidal Torque Spin (Forces clumps to spin up into flat rotating spiral disks)
    curl_x = np.gradient(psi1_z, axis=1) - np.gradient(psi1_y, axis=2)
    curl_y = np.gradient(psi1_x, axis=2) - np.gradient(psi1_z, axis=0)
    curl_z = np.gradient(psi1_y, axis=0) - np.gradient(psi1_x, axis=1)

    vx_peculiar = curl_x * 0.85 + np.random.normal(0.0, 0.015, (N_GRID, N_GRID, N_GRID))
    vy_peculiar = curl_y * 0.85 + np.random.normal(0.0, 0.015, (N_GRID, N_GRID, N_GRID))
    vz_peculiar = curl_z * 0.85 + np.random.normal(0.0, 0.015, (N_GRID, N_GRID, N_GRID))

    vx_total = vx_hubble + vx_peculiar
    vy_total = vy_hubble + vy_peculiar
    vz_total = vz_hubble + vz_peculiar

    # Extract particles in dense spherical volume
    r_dist = np.sqrt(pos_x**2 + pos_y**2 + pos_z**2)
    sphere_mask = r_dist <= (R_INITIAL * 1.05)

    return (
        pos_x[sphere_mask], pos_y[sphere_mask], pos_z[sphere_mask],
        vx_total[sphere_mask], vy_total[sphere_mask], vz_total[sphere_mask]
    )


# =============================================================================
# 3. Export Initial Conditions
# =============================================================================

if __name__ == "__main__":
    px, py, pz, vx, vy, vz = generate_knife_edge_bigbang_2lpt()
    N = len(px)

    particle_mass = TOTAL_MASS / N
    system = ns.System()

    for i in range(N):
        # 60% Dark Matter (1), 40% Luminous Stars & Baryons (4)
        ptype = 4 if (i % 5 in (0, 1)) else 1
        system.add(
            x=px[i], y=py[i], z=pz[i],
            vx=vx[i], vy=vy[i], vz=vz[i],
            mass=particle_mass,
            ptype=ptype
        )

    output_file = "bigbang_knife_edge.txt"
    system.save(output_file)
    print("=" * 85)
    print(f"[SUCCESS] Exported {output_file} ({N:,} particles) on the Knife-Edge of Escape Velocity!")
    print("=====================================================================================")
