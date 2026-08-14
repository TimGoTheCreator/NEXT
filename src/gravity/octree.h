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
#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>

#include "dt/softening.h"
#include "floatdef.h"
#include "struct/particle.h"

/**
 * @brief Octree node structure redesigned for Structure of Arrays (SoA).
 * Instead of storing Particle pointers, it stores indices into the ParticleSystem.
 */
struct OctreePool;

struct Octree {
    real cx = 0, cy = 0, cz = 0;  // Center of Mass
    real m = 0;           // Total Mass
    real cbrt_m = 0;      // Cube root of mass precomputed for fast softening
    real x = 0, y = 0, z = 0;     // Geometric center of node
    real size = 0;        // Half-width of node
    bool leaf = true;

    // Index of the particle in the ParticleSystem. -1 means empty.
    int bodyIdx = -1;

    Octree* child[8] = {nullptr};

    // Quadrupole tensor for higher-order gravity approximation
    real Qxx = 0, Qyy = 0, Qzz = 0;
    real Qxy = 0, Qxz = 0, Qyz = 0;

    Octree() = default;
    Octree(real X, real Y, real Z, real S) : x(X), y(Y), z(Z), size(S) {}

    ~Octree() = default;

    /**
     * @brief Determines which octant a particle belongs to.
     */
    int getOctant(real px, real py, real pz) const {
        return (px > x) * 1 + (py > y) * 2 + (pz > z) * 4;
    }

    /**
     * @brief Inserts a particle index into the tree using an arena pool.
     */
    void insert(int idx, const ParticleSystem& ps, OctreePool& pool, int depth = 0);

    /**
     * @brief Recursively computes mass properties and quadrupole moments.
     */
    void computeMass(const ParticleSystem& ps) {
        if (leaf) {
            if (bodyIdx != -1) {
                m = ps.m[bodyIdx];
                cbrt_m = std::cbrt(m);
                cx = ps.x[bodyIdx];
                cy = ps.y[bodyIdx];
                cz = ps.z[bodyIdx];
            } else {
                m = 0;
                cbrt_m = 0;
                cx = cy = cz = 0;
            }
            Qxx = Qyy = Qzz = Qxy = Qxz = Qyz = 0;
            return;
        }

        m = 0;
        cx = cy = cz = 0;
        for (auto* c : child) {
            if (!c) continue;
            c->computeMass(ps);
            if (c->m == 0) continue;
            m += c->m;
            cx += c->cx * c->m;
            cy += c->cy * c->m;
            cz += c->cz * c->m;
        }
        if (m > 0) {
            cx /= m;
            cy /= m;
            cz /= m;
            cbrt_m = std::cbrt(m);
        } else {
            cbrt_m = 0;
        }

        Qxx = Qyy = Qzz = Qxy = Qxz = Qyz = 0;
        for (auto* c : child) {
            if (!c || c->m == 0) continue;
            real rx = c->cx - cx;
            real ry = c->cy - cy;
            real rz = c->cz - cz;
            real r2 = rx * rx + ry * ry + rz * rz + (size * size * real(0.01));
            real mc = c->m;
            Qxx += mc * (3 * rx * rx - r2);
            Qyy += mc * (3 * ry * ry - r2);
            Qzz += mc * (3 * rz * rz - r2);
            Qxy += mc * (3 * rx * ry);
            Qxz += mc * (3 * rx * rz);
            Qyz += mc * (3 * ry * rz);
        }
    }
};

struct OctreePool {
    std::vector<std::unique_ptr<Octree>> storage;
    size_t cursor = 0;

    void reset() {
        cursor = 0;
    }

    Octree* createNode(real X, real Y, real Z, real S) {
        if (cursor >= storage.size()) {
            storage.push_back(std::make_unique<Octree>(X, Y, Z, S));
        } else {
            *storage[cursor] = Octree(X, Y, Z, S);
        }
        return storage[cursor++].get();
    }
};

inline void Octree::insert(int idx, const ParticleSystem& ps, OctreePool& pool, int depth) {
    if (depth >= 30 || size < real(1e-10)) {
        return;
    }

    if (leaf && bodyIdx == -1) {
        bodyIdx = idx;
        return;
    }

    if (leaf) {
        leaf = false;
        int oldIdx = bodyIdx;
        bodyIdx = -1;
        int oct = getOctant(ps.x[oldIdx], ps.y[oldIdx], ps.z[oldIdx]);
        if (!child[oct]) {
            real hs = size * real(0.5);
            child[oct] = pool.createNode(x + ((oct & 1) ? hs : -hs), y + ((oct & 2) ? hs : -hs),
                                         z + ((oct & 4) ? hs : -hs), hs);
        }
        child[oct]->insert(oldIdx, ps, pool, depth + 1);
    }

    int oct = getOctant(ps.x[idx], ps.y[idx], ps.z[idx]);
    if (!child[oct]) {
        real hs = size * real(0.5);
        child[oct] = pool.createNode(x + ((oct & 1) ? hs : -hs), y + ((oct & 2) ? hs : -hs),
                                     z + ((oct & 4) ? hs : -hs), hs);
    }
    child[oct]->insert(idx, ps, pool, depth + 1);
}

/**
 * @brief Barnes-Hut acceleration calculation for a target particle at index 'i'.
 */
inline void bhAccel(Octree* node, int i, const ParticleSystem& ps, real theta, real& ax, real& ay,
                    real& az) {
    if (!node || node->m == 0) return;
    if (node->leaf && node->bodyIdx == i) return;

    constexpr real G = real(1.0);
    real dx = node->cx - ps.x[i];
    real dy = node->cy - ps.y[i];
    real dz = node->cz - ps.z[i];
    real r2 = dx * dx + dy * dy + dz * dz;

    const real open_r2 = (node->size / theta) * (node->size / theta);
    if (!node->leaf && r2 < open_r2) {
        for (auto* c : node->child) {
            if (c) bhAccel(c, i, ps, theta, ax, ay, az);
        }
        return;
    }

    constexpr real eps = real(0.05);
    real r2_soft = r2 + eps * eps;
    real dist_inv = real(1.0) / std::sqrt(r2_soft);

    real inv3 = dist_inv * dist_inv * dist_inv;
    real fac = G * node->m * inv3;

    ax += dx * fac;
    ay += dy * fac;
    az += dz * fac;

    // Quadrupole contributions
    real inv5 = inv3 * (dist_inv * dist_inv);
    real inv7 = inv5 * (dist_inv * dist_inv);

    real q = node->Qxx * dx * dx + node->Qyy * dy * dy + node->Qzz * dz * dz +
             2 * (node->Qxy * dx * dy + node->Qxz * dx * dz + node->Qyz * dy * dz);

    real Qrx = 2 * (node->Qxx * dx + node->Qxy * dy + node->Qxz * dz);
    real Qry = 2 * (node->Qxy * dx + node->Qyy * dy + node->Qzz * dz);
    real Qrz = 2 * (node->Qxz * dx + node->Qyz * dy + node->Qzz * dz);

    ax += (G * real(0.5)) * (Qrx * inv5 - 5 * q * inv7 * dx);
    ay += (G * real(0.5)) * (Qry * inv5 - 5 * q * inv7 * dy);
    az += (G * real(0.5)) * (Qrz * inv5 - 5 * q * inv7 * dz);
}
