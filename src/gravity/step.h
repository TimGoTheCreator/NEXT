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
#include "octree.h"
#include "particle.h"
#include <memory>
#include <algorithm>
#include <omp.h>

/**
 * @brief Performs a complete Leapfrog Step (Kick-Drift-Kick) using SoA data.
 */
inline void Step(ParticleSystem &ps, real dt) {
    if (ps.size() == 0) return;

    const real theta = 0.5;
    const real half = dt * real(0.5);
    const int N = static_cast<int>(ps.size());

    // Helper lambda to build the tree using the ParticleSystem indices
    auto buildTree = [&]() -> std::unique_ptr<Octree> {
        real minx = 1e30, miny = 1e30, minz = 1e30;
        real maxx = -1e30, maxy = -1e30, maxz = -1e30;

        // Bounding box calculation (SoA access is very fast here)
        for (int i = 0; i < N; ++i) {
            minx = std::min(minx, ps.x[i]); miny = std::min(miny, ps.y[i]); minz = std::min(minz, ps.z[i]);
            maxx = std::max(maxx, ps.x[i]); maxy = std::max(maxy, ps.y[i]); maxz = std::max(maxz, ps.z[i]);
        }

        real cx = (minx + maxx) * 0.5;
        real cy = (miny + maxy) * 0.5;
        real cz = (minz + maxz) * 0.5;
        real size = std::max({maxx - minx, maxy - miny, maxz - minz}) * 0.5;

        if (size <= 0) size = 1.0;

        auto root = std::make_unique<Octree>(cx, cy, cz, size);

        // Insert particle indices 0 to N-1
        for (int i = 0; i < N; ++i) {
            root->insert(i, ps);
        }

        root->computeMass(ps);
        return root;
    };

    // --- First Kick (dt/2) ---
    {
        std::unique_ptr<Octree> root = buildTree();

        #pragma omp parallel for schedule(dynamic, 64)
        for (int i = 0; i < N; ++i) {
            real ax = 0, ay = 0, az = 0;
            bhAccel(root.get(), i, ps, theta, ax, ay, az);

            ps.vx[i] += ax * half;
            ps.vy[i] += ay * half;
            ps.vz[i] += az * half;
        }
    }

    // --- Drift (dt) ---
    // Contiguous memory access makes this loop ideal for SIMD
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < N; ++i) {
        ps.x[i] += ps.vx[i] * dt;
        ps.y[i] += ps.vy[i] * dt;
        ps.z[i] += ps.vz[i] * dt;
    }

    // --- Second Kick (dt/2) ---
    {
        std::unique_ptr<Octree> root = buildTree();

        #pragma omp parallel for schedule(dynamic, 64)
        for (int i = 0; i < N; ++i) {
            real ax = 0, ay = 0, az = 0;
            bhAccel(root.get(), i, ps, theta, ax, ay, az);

            ps.vx[i] += ax * half;
            ps.vy[i] += ay * half;
            ps.vz[i] += az * half;
        }
    }
}
