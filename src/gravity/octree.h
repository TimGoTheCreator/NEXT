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
#include "../struct/particle.h"
#include <vector>
#include <cmath>
#include <memory> // Required for unique_ptr
#include "dt/softening.h"
#include "floatdef.h"

struct Octree {
    real cx, cy, cz;     // COM
    real m;              // mass
    real x, y, z;        // node center
    real size;           // half-width
    bool leaf = true;
    Particle* body = nullptr; 
    
    // Ownership: unique_ptr handles memory automatically
    std::unique_ptr<Octree> child[8] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

    // Quadrupole tensor
    real Qxx = 0, Qyy = 0, Qzz = 0;
    real Qxy = 0, Qxz = 0, Qyz = 0;

    Octree(real X, real Y, real Z, real S) : x(X), y(Y), z(Z), size(S), m(0), cx(0), cy(0), cz(0) {}

    // Destructor is now empty; unique_ptr cleans up children automatically
    ~Octree() = default;

    int index(const Particle& p) const {
        return (p.x > x) * 1 + (p.y > y) * 2 + (p.z > z) * 4;
    }

    // Returns unique_ptr to take ownership
    std::unique_ptr<Octree> createChild(int idx) {
        real hs = size * real(0.5);
        return std::make_unique<Octree>(
            x + ((idx & 1) ? hs : -hs), 
            y + ((idx & 2) ? hs : -hs), 
            z + ((idx & 4) ? hs : -hs), 
            hs
        );
    }

    void insert(Particle* p) {
        if (leaf && body == nullptr) { 
            body = p; 
            return; 
        }
        
        if (leaf) {
            leaf = false;
            Particle* old = body; 
            body = nullptr;
            int idx = index(*old);
            if (!child[idx]) child[idx] = createChild(idx);
            child[idx]->insert(old);
        }
        
        int idx = index(*p);
        if (!child[idx]) child[idx] = createChild(idx);
        child[idx]->insert(p);
    }

    void computeMass() {
        if (leaf) {
            if (body) { m = body->m; cx = body->x; cy = body->y; cz = body->z; }
            else { m = 0; cx = cy = cz = 0; }
            Qxx = Qyy = Qzz = Qxy = Qxz = Qyz = 0;
            return;
        }

        m = 0; cx = cy = cz = 0;
        for (auto& c : child) { // Use reference to unique_ptr
            if (!c) continue;
            c->computeMass();
            if (c->m == 0) continue;
            m += c->m;
            cx += c->cx * c->m; cy += c->cy * c->m; cz += c->cz * c->m;
        }
        if (m > 0) { cx /= m; cy /= m; cz /= m; }

        Qxx = Qyy = Qzz = Qxy = Qxz = Qyz = 0;
        for (auto& c : child) {
            if (!c || c->m == 0) continue;
            real rx = c->cx - cx; real ry = c->cy - cy; real rz = c->cz - cz;
            real r2 = rx * rx + ry * ry + rz * rz + (size * size * real(0.01));
            real mchild = c->m;
            Qxx += mchild * (3 * rx * rx - r2);
            Qyy += mchild * (3 * ry * ry - r2);
            Qzz += mchild * (3 * rz * rz - r2);
            Qxy += mchild * (3 * rx * ry);
            Qxz += mchild * (3 * rx * rz);
            Qyz += mchild * (3 * ry * rz);
        }
    }
};

// Traverse using raw pointers (non-owning observer)
inline void bhAccel(Octree* node, const Particle& p, real theta, real& ax, real& ay, real& az) {
    if (!node || node->m == 0) return;
    if (node->leaf && node->body == &p) return;

    constexpr real G = real(1.0);
    real dx = node->cx - p.x; real dy = node->cy - p.y; real dz = node->cz - p.z;
    real r2 = dx*dx + dy*dy + dz*dz;
    real dist = std::sqrt(r2 + real(1e-20));

    real eps = nextSoftening(node->size, node->m, dist);
    if (p.type == 1) {
        eps = std::max(eps, 2.0 * node->size / std::pow(node->m / p.m, 1.0/3));
    }

    real r2_soft = r2 + eps*eps;
    real dist_soft = std::sqrt(r2_soft);

    if (node->leaf || (node->size / dist) < theta) {
        real invDist = real(1.0) / dist_soft;
        real invDist3 = invDist * invDist * invDist;
        real fac = G * node->m * invDist3;

        ax += dx * fac; ay += dy * fac; az += dz * fac;

        real invr5 = invDist3 * (invDist * invDist);
        real invr7 = invr5 * (invDist * invDist);

        real q = node->Qxx*dx*dx + node->Qyy*dy*dy + node->Qzz*dz*dz + 
                 2*(node->Qxy*dx*dy + node->Qxz*dx*dz + node->Qyz*dy*dz);

        real Qrx = 2*(node->Qxx*dx + node->Qxy*dy + node->Qxz*dz);
        real Qry = 2*(node->Qxy*dx + node->Qyy*dy + node->Qyz*dz);
        real Qrz = 2*(node->Qxz*dx + node->Qyz*dy + node->Qzz*dz);

        ax += (G * real(0.5)) * (Qrx * invr5 - 5 * q * invr7 * dx);
        ay += (G * real(0.5)) * (Qry * invr5 - 5 * q * invr7 * dy);
        az += (G * real(0.5)) * (Qrz * invr5 - 5 * q * invr7 * dz);
        return;
    }

    for (auto& c : node->child) {
        if (c) bhAccel(c.get(), p, theta, ax, ay, az); // Use .get() to pass raw pointer
    }
}
