// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#pragma once
#include "floatdef.h"
#include "dt/softening.h"
#include <vector>
#include <cmath>
#include <algorithm>

/**
 * @brief Structure of Arrays (SoA) container for the particle data.
 * This layout is significantly faster for SIMD vectorization and cache locality.
 */
struct Particle {
    // Positions
    std::vector<real> x, y, z;
    // Velocities
    std::vector<real> vx, vy, vz;
    // Accelerations (used for force summation)
    std::vector<real> ax, ay, az;
    // Masses and Types (0 = Star, 1 = Dark Matter)
    std::vector<real> m;
    std::vector<int> type;

    /**
     * @brief Resizes all buffers and ensures accelerations are zeroed.
     */
    void resize(size_t n) {
        x.resize(n, 0); y.resize(n, 0); z.resize(n, 0);
        vx.resize(n, 0); vy.resize(n, 0); vz.resize(n, 0);
        ax.assign(n, 0); ay.assign(n, 0); az.assign(n, 0);
        m.resize(n, 0); type.resize(n, 0);
    }

    /**
     * @brief Adds a single particle to the system.
     */
    void addParticle(real px, real py, real pz, real pvx, real pvy, real pvz, real pm, int ptype) {
        x.push_back(px); y.push_back(py); z.push_back(pz);
        vx.push_back(pvx); vy.push_back(pvy); vz.push_back(pvz);
        ax.push_back(0); ay.push_back(0); az.push_back(0);
        m.push_back(pm);
        type.push_back(ptype);
    }

    size_t size() const { return x.size(); }

    void clear() {
        x.clear(); y.clear(); z.clear();
        vx.clear(); vy.clear(); vz.clear();
        ax.clear(); ay.clear(); az.clear();
        m.clear(); type.clear();
    }
};

/**
 * @brief Calculates gravity between two indices in the SoA system.
 * This follows Newton's 3rd law to update both particles simultaneously.
 */
inline void GravitySoA(ParticleSystem &ps, size_t i, size_t j, real dt) {
    constexpr real G = real(1.0);

    // Get separation vector
    real dx = ps.x[j] - ps.x[i];
    real dy = ps.y[j] - ps.y[i];
    real dz = ps.z[j] - ps.z[i];

    // Adaptive Softening (DM vs Stars)
    // Note: eps is based on your pairSoftening logic
    real eps = pairSoftening(ps.m[i], ps.m[j]);

    // r^2 + epsilon^2
    real r2 = dx * dx + dy * dy + dz * dz + eps * eps;

    // Standard Gravity: F = G * m1 * m2 / r^2
    real invR2 = real(1.0) / r2;
    real invR = std::sqrt(invR2);
    real invR3 = invR * invR2;

    // Common force factor
    real f_ij = G * invR3;

    // Acceleration on i due to j
    real aix = f_ij * ps.m[j] * dx;
    real aiy = f_ij * ps.m[j] * dy;
    real aiz = f_ij * ps.m[j] * dz;

    // Apply Velocity Kicks (dt)
    ps.vx[i] += aix * dt;
    ps.vy[i] += aiy * dt;
    ps.vz[i] += aiz * dt;

    // Apply Velocity Kicks to j (Newton's 3rd Law: a_j = -a_i * (m_i / m_j))
    // We multiply by m_i here because f_ij only contained G and r^-3
    ps.vx[j] -= (f_ij * ps.m[i] * dx) * dt;
    ps.vy[j] -= (f_ij * ps.m[i] * dy) * dt;
    ps.vz[j] -= (f_ij * ps.m[i] * dz) * dt;
}

/**
 * @brief Contiguous Drift step (Update Positions).
 * High locality ensures the compiler can vectorize this with SIMD.
 */
inline void DriftSoA(ParticleSystem &ps, real dt) {
    const size_t n = ps.size();
    #pragma omp parallel for schedule(static)
    for (size_t i = 0; i < n; ++i) {
        ps.x[i] += ps.vx[i] * dt;
        ps.y[i] += ps.vy[i] * dt;
        ps.z[i] += ps.vz[i] * dt;
    }
}
