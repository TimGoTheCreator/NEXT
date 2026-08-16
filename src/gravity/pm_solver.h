// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#pragma once
#include <vector>
#include <cmath>
#include <complex>
#include "floatdef.h"
#include "struct/particle.h"

namespace next {
namespace gravity {

/**
 * @brief High-Performance Particle-Mesh (PM) FFT Poisson Solver
 * 
 * Implements:
 * 1. Multi-threaded Cloud-In-Cell (CIC) charge/mass assignment onto 3D mesh.
 * 2. 3D Real-to-Complex Fast Fourier Transform Poisson solver with Gaussian force split.
 * 3. Finite-difference force field differentiation & trilinear CIC interpolation back to particles.
 */
class PMSolver {
public:
    PMSolver() = default;
    PMSolver(int grid_dim, real box_size, real r_split);
    ~PMSolver() = default;

    void init(int grid_dim, real box_size, real r_split);

    /**
     * @brief Computes long-range gravitational accelerations for all particles.
     */
    void compute_long_range_forces(const ParticleSystem& ps,
                                   std::vector<real>& ax_long,
                                   std::vector<real>& ay_long,
                                   std::vector<real>& az_long);

    int get_grid_dim() const { return Ngrid_; }
    real get_box_size() const { return box_size_; }
    real get_r_split() const { return r_split_; }

private:
    int Ngrid_ = 64;
    real box_size_ = 100.0;
    real r_split_ = 2.0;      // Splitting radius r_s
    real cell_size_ = 1.0;
    real inv_cell_size_ = 1.0;

    std::vector<real> density_grid_;
    std::vector<real> potential_grid_;
    std::vector<real> fx_grid_;
    std::vector<real> fy_grid_;
    std::vector<real> fz_grid_;

    // Internal 3D FFT Engine
    void solve_poisson_3d();
    void compute_force_fields_3d();
};

} // namespace gravity
} // namespace next
