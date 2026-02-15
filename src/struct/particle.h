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
 */
struct Particle {
    std::vector<real> x, y, z;
    std::vector<real> vx, vy, vz;
    std::vector<real> ax, ay, az; // Storing for potential tree-build recycling
    std::vector<real> m;
    std::vector<int> type;

    void resize(size_t n) {
        x.resize(n, 0); y.resize(n, 0); z.resize(n, 0);
        vx.resize(n, 0); vy.resize(n, 0); vz.resize(n, 0);
        ax.assign(n, 0); ay.assign(n, 0); az.assign(n, 0);
        m.resize(n, 0); type.resize(n, 0);
    }

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

/** * ALIAS DEFINITION
 * This must be here for GravitySoA and Step to recognize 'ParticleSystem'
 */
using ParticleSystem = Particle;

/**
 * @brief Calculates direct gravity between two indices in the SoA system.
 * Useful for brute-force or small-N components.
 */
inline void GravitySoA(ParticleSystem &ps, size_t i, size_t j, real dt) {
    constexpr real G = real(1.0);

    real dx = ps.x[j] - ps.x[i];
    real dy = ps.y[j] - ps.y[i];
    real dz = ps.z[j] - ps.z[i];

    real eps = pairSoftening(ps.m[i], ps.m[j]);
    real r2 = dx * dx + dy * dy + dz * dz + eps * eps;

    real invR2 = real(1.0) / r2;
    real invR3 = invR2 / std::sqrt(r2);
    real f_base = G * invR3 * dt;

    ps.vx[i] += f_base * ps.m[j] * dx;
    ps.vy[i] += f_base * ps.m[j] * dy;
    ps.vz[i] += f_base * ps.m[j] * dz;

    ps.vx[j] -= f_base * ps.m[i] * dx;
    ps.vy[j] -= f_base * ps.m[i] * dy;
    ps.vz[j] -= f_base * ps.m[i] * dz;
}
