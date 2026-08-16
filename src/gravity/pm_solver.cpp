// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#include "pm_solver.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <omp.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace next {
namespace gravity {

// 1D In-Place Cooley-Tukey Radix-2 FFT Helper
static void fft1d(std::complex<double>* data, int n, bool inverse) {
    if (n <= 1) return;

    // Bit-reversal permutation
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) std::swap(data[i], data[j]);
    }

    // Cooley-Tukey butterflies
    for (int len = 2; len <= n; len <<= 1) {
        double ang = 2.0 * M_PI / len * (inverse ? 1.0 : -1.0);
        std::complex<double> wlen(std::cos(ang), std::sin(ang));
        for (int i = 0; i < n; i += len) {
            std::complex<double> w(1.0, 0.0);
            for (int j = 0; j < len / 2; ++j) {
                std::complex<double> u = data[i + j];
                std::complex<double> v = data[i + j + len / 2] * w;
                data[i + j] = u + v;
                data[i + j + len / 2] = u - v;
                w *= wlen;
            }
        }
    }

    if (inverse) {
        double inv_n = 1.0 / n;
        for (int i = 0; i < n; ++i) data[i] *= inv_n;
    }
}

PMSolver::PMSolver(int grid_dim, real box_size, real r_split) {
    init(grid_dim, box_size, r_split);
}

void PMSolver::init(int grid_dim, real box_size, real r_split) {
    // Ensure grid dimension is power of 2 for high-speed FFT
    int p = 1;
    while (p < grid_dim) p <<= 1;
    Ngrid_ = p;

    box_size_ = box_size;
    r_split_ = (r_split > 0.0) ? r_split : (box_size_ / Ngrid_ * 1.25);
    cell_size_ = box_size_ / Ngrid_;
    inv_cell_size_ = 1.0 / cell_size_;

    size_t total_cells = static_cast<size_t>(Ngrid_) * Ngrid_ * Ngrid_;
    density_grid_.assign(total_cells, 0.0);
    potential_grid_.assign(total_cells, 0.0);
    fx_grid_.assign(total_cells, 0.0);
    fy_grid_.assign(total_cells, 0.0);
    fz_grid_.assign(total_cells, 0.0);
}

void PMSolver::compute_long_range_forces(const ParticleSystem& ps,
                                         std::vector<real>& ax_long,
                                         std::vector<real>& ay_long,
                                         std::vector<real>& az_long)
{
    const int N = static_cast<int>(ps.size());
    if (N == 0) return;

    ax_long.assign(N, 0.0);
    ay_long.assign(N, 0.0);
    az_long.assign(N, 0.0);

    const int G = Ngrid_;
    const size_t total_cells = static_cast<size_t>(G) * G * G;
    const real half_box = box_size_ * 0.5;

    // 1. Cloud-In-Cell (CIC) Multi-Threaded Mass Assignment
    std::fill(density_grid_.begin(), density_grid_.end(), 0.0);

    #pragma omp parallel
    {
        std::vector<real> local_density(total_cells, 0.0);

        #pragma omp for schedule(static)
        for (int i = 0; i < N; ++i) {
            // Periodic box coordinates [0, box_size)
            real px = std::fmod(ps.x[i] + half_box, box_size_);
            if (px < 0.0) px += box_size_;
            real py = std::fmod(ps.y[i] + half_box, box_size_);
            if (py < 0.0) py += box_size_;
            real pz = std::fmod(ps.z[i] + half_box, box_size_);
            if (pz < 0.0) pz += box_size_;

            real gx = px * inv_cell_size_ - 0.5;
            real gy = py * inv_cell_size_ - 0.5;
            real gz = pz * inv_cell_size_ - 0.5;

            int i0 = static_cast<int>(std::floor(gx));
            int j0 = static_cast<int>(std::floor(gy));
            int k0 = static_cast<int>(std::floor(gz));

            real dx = gx - i0;
            real dy = gy - j0;
            real dz = gz - k0;

            real tx = 1.0 - dx;
            real ty = 1.0 - dy;
            real tz = 1.0 - dz;

            real mass = ps.m[i];

            int i1 = (i0 + 1 + G) % G;
            int j1 = (j0 + 1 + G) % G;
            int k1 = (k0 + 1 + G) % G;
            i0 = (i0 % G + G) % G;
            j0 = (j0 % G + G) % G;
            k0 = (k0 % G + G) % G;

            local_density[(size_t)i0 * G * G + (size_t)j0 * G + (size_t)k0] += mass * (tx * ty * tz);
            local_density[(size_t)i1 * G * G + (size_t)j0 * G + (size_t)k0] += mass * (dx * ty * tz);
            local_density[(size_t)i0 * G * G + (size_t)j1 * G + (size_t)k0] += mass * (tx * dy * tz);
            local_density[(size_t)i1 * G * G + (size_t)j1 * G + (size_t)k0] += mass * (dx * dy * tz);

            local_density[(size_t)i0 * G * G + (size_t)j0 * G + (size_t)k1] += mass * (tx * ty * dz);
            local_density[(size_t)i1 * G * G + (size_t)j0 * G + (size_t)k1] += mass * (dx * ty * dz);
            local_density[(size_t)i0 * G * G + (size_t)j1 * G + (size_t)k1] += mass * (tx * dy * dz);
            local_density[(size_t)i1 * G * G + (size_t)j1 * G + (size_t)k1] += mass * (dx * dy * dz);
        }

        #pragma omp critical
        {
            for (size_t c = 0; c < total_cells; ++c) {
                density_grid_[c] += local_density[c];
            }
        }
    }

    // 2. Solve Poisson Equation via 3D FFT with Complementary Gaussian Cutoff
    solve_poisson_3d();

    // 3. Finite-Difference Field Differentiation
    compute_force_fields_3d();

    // 4. Trilinear Cloud-In-Cell Force Interpolation to Particles
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < N; ++i) {
        real px = std::fmod(ps.x[i] + half_box, box_size_);
        if (px < 0.0) px += box_size_;
        real py = std::fmod(ps.y[i] + half_box, box_size_);
        if (py < 0.0) py += box_size_;
        real pz = std::fmod(ps.z[i] + half_box, box_size_);
        if (pz < 0.0) pz += box_size_;

        real gx = px * inv_cell_size_ - 0.5;
        real gy = py * inv_cell_size_ - 0.5;
        real gz = pz * inv_cell_size_ - 0.5;

        int i0 = static_cast<int>(std::floor(gx));
        int j0 = static_cast<int>(std::floor(gy));
        int k0 = static_cast<int>(std::floor(gz));

        real dx = gx - i0;
        real dy = gy - j0;
        real dz = gz - k0;

        real tx = 1.0 - dx;
        real ty = 1.0 - dy;
        real tz = 1.0 - dz;

        int i1 = (i0 + 1 + G) % G;
        int j1 = (j0 + 1 + G) % G;
        int k1 = (k0 + 1 + G) % G;
        i0 = (i0 % G + G) % G;
        j0 = (j0 % G + G) % G;
        k0 = (k0 % G + G) % G;

        auto interp = [&](const std::vector<real>& f) -> real {
            return tx * ty * tz * f[(size_t)i0 * G * G + (size_t)j0 * G + (size_t)k0] +
                   dx * ty * tz * f[(size_t)i1 * G * G + (size_t)j0 * G + (size_t)k0] +
                   tx * dy * tz * f[(size_t)i0 * G * G + (size_t)j1 * G + (size_t)k0] +
                   dx * dy * tz * f[(size_t)i1 * G * G + (size_t)j1 * G + (size_t)k0] +
                   tx * ty * dz * f[(size_t)i0 * G * G + (size_t)j0 * G + (size_t)k1] +
                   dx * ty * dz * f[(size_t)i1 * G * G + (size_t)j0 * G + (size_t)k1] +
                   tx * dy * dz * f[(size_t)i0 * G * G + (size_t)j1 * G + (size_t)k1] +
                   dx * dy * dz * f[(size_t)i1 * G * G + (size_t)j1 * G + (size_t)k1];
        };

        ax_long[i] = interp(fx_grid_);
        ay_long[i] = interp(fy_grid_);
        az_long[i] = interp(fz_grid_);
    }
}

void PMSolver::solve_poisson_3d() {
    const int G = Ngrid_;
    const size_t total = (size_t)G * G * G;
    std::vector<std::complex<double>> complex_grid(total);

    #pragma omp parallel for schedule(static)
    for (size_t i = 0; i < total; ++i) {
        complex_grid[i] = std::complex<double>(density_grid_[i], 0.0);
    }

    // Forward 3D FFT (X, Y, Z)
    #pragma omp parallel for collapse(2) schedule(static)
    for (int y = 0; y < G; ++y) {
        for (int z = 0; z < G; ++z) {
            std::vector<std::complex<double>> line(G);
            for (int x = 0; x < G; ++x) line[x] = complex_grid[(size_t)x * G * G + (size_t)y * G + z];
            fft1d(line.data(), G, false);
            for (int x = 0; x < G; ++x) complex_grid[(size_t)x * G * G + (size_t)y * G + z] = line[x];
        }
    }

    #pragma omp parallel for collapse(2) schedule(static)
    for (int x = 0; x < G; ++x) {
        for (int z = 0; z < G; ++z) {
            std::vector<std::complex<double>> line(G);
            for (int y = 0; y < G; ++y) line[y] = complex_grid[(size_t)x * G * G + (size_t)y * G + z];
            fft1d(line.data(), G, false);
            for (int y = 0; y < G; ++y) complex_grid[(size_t)x * G * G + (size_t)y * G + z] = line[y];
        }
    }

    #pragma omp parallel for collapse(2) schedule(static)
    for (int x = 0; x < G; ++x) {
        for (int y = 0; y < G; ++y) {
            std::vector<std::complex<double>> line(G);
            for (int z = 0; z < G; ++z) line[z] = complex_grid[(size_t)x * G * G + (size_t)y * G + z];
            fft1d(line.data(), G, false);
            for (int z = 0; z < G; ++z) complex_grid[(size_t)x * G * G + (size_t)y * G + z] = line[z];
        }
    }

    // Multiply Green's function with TreePM Complementary Gaussian Filter: -4*pi*G / k^2 * exp(-k^2 * r_s^2)
    const double k_fund = 2.0 * M_PI / box_size_;
    const double rs2 = r_split_ * r_split_;

    #pragma omp parallel for collapse(3) schedule(static)
    for (int i = 0; i < G; ++i) {
        for (int j = 0; j < G; ++j) {
            for (int k = 0; k < G; ++k) {
                if (i == 0 && j == 0 && k == 0) {
                    complex_grid[0] = 0.0;
                    continue;
                }

                double kx = (i <= G / 2 ? i : i - G) * k_fund;
                double ky = (j <= G / 2 ? j : j - G) * k_fund;
                double kz = (k <= G / 2 ? k : k - G) * k_fund;
                double k2 = kx * kx + ky * ky + kz * kz;

                // Green's function with Gaussian long-range cutoff
                double green = -(4.0 * M_PI / k2) * std::exp(-k2 * rs2) * (box_size_ / (G * G * G));
                complex_grid[(size_t)i * G * G + (size_t)j * G + k] *= green;
            }
        }
    }

    // Inverse 3D FFT (Z, Y, X)
    #pragma omp parallel for collapse(2) schedule(static)
    for (int x = 0; x < G; ++x) {
        for (int y = 0; y < G; ++y) {
            std::vector<std::complex<double>> line(G);
            for (int z = 0; z < G; ++z) line[z] = complex_grid[(size_t)x * G * G + (size_t)y * G + z];
            fft1d(line.data(), G, true);
            for (int z = 0; z < G; ++z) complex_grid[(size_t)x * G * G + (size_t)y * G + z] = line[z];
        }
    }

    #pragma omp parallel for collapse(2) schedule(static)
    for (int x = 0; x < G; ++x) {
        for (int z = 0; z < G; ++z) {
            std::vector<std::complex<double>> line(G);
            for (int y = 0; y < G; ++y) line[y] = complex_grid[(size_t)x * G * G + (size_t)y * G + z];
            fft1d(line.data(), G, true);
            for (int y = 0; y < G; ++y) complex_grid[(size_t)x * G * G + (size_t)y * G + z] = line[y];
        }
    }

    #pragma omp parallel for collapse(2) schedule(static)
    for (int y = 0; y < G; ++y) {
        for (int z = 0; z < G; ++z) {
            std::vector<std::complex<double>> line(G);
            for (int x = 0; x < G; ++x) line[x] = complex_grid[(size_t)x * G * G + (size_t)y * G + z];
            fft1d(line.data(), G, true);
            for (int x = 0; x < G; ++x) potential_grid_[(size_t)x * G * G + (size_t)y * G + z] = static_cast<real>(line[x].real());
        }
    }
}

void PMSolver::compute_force_fields_3d() {
    const int G = Ngrid_;
    const real inv_2dx = 1.0 / (2.0 * cell_size_);

    #pragma omp parallel for collapse(3) schedule(static)
    for (int x = 0; x < G; ++x) {
        for (int y = 0; y < G; ++y) {
            for (int z = 0; z < G; ++z) {
                int xp = (x + 1) % G, xm = (x - 1 + G) % G;
                int yp = (y + 1) % G, ym = (y - 1 + G) % G;
                int zp = (z + 1) % G, zm = (z - 1 + G) % G;

                // F = - grad(Phi)
                fx_grid_[(size_t)x * G * G + (size_t)y * G + z] =
                    -(potential_grid_[(size_t)xp * G * G + (size_t)y * G + z] - potential_grid_[(size_t)xm * G * G + (size_t)y * G + z]) * inv_2dx;

                fy_grid_[(size_t)x * G * G + (size_t)y * G + z] =
                    -(potential_grid_[(size_t)x * G * G + (size_t)yp * G + z] - potential_grid_[(size_t)x * G * G + (size_t)ym * G + z]) * inv_2dx;

                fz_grid_[(size_t)x * G * G + (size_t)y * G + z] =
                    -(potential_grid_[(size_t)x * G * G + (size_t)y * G + zp] - potential_grid_[(size_t)x * G * G + (size_t)y * G + zm]) * inv_2dx;
            }
        }
    }
}

} // namespace gravity
} // namespace next
