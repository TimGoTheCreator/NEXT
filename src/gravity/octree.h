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

struct Octree {
    // monopole
    real cx, cy, cz;     // center of mass
    real m;              // total mass

    // node geometry
    real x, y, z;        // center of node
    real size;           // half-width
    bool leaf = true;
    Particle* body = nullptr;
    Octree* child[8] = {nullptr};

    // symmetric quadrupole tensor (6 independent components)
    real Qxx = 0, Qyy = 0, Qzz = 0;
    real Qxy = 0, Qxz = 0, Qyz = 0;

    Octree(real X, real Y, real Z, real S)
        : cx(0), cy(0), cz(0), m(0),
          x(X), y(Y), z(Z), size(S) {}

    ~Octree() {
        for (auto c : child) delete c;
    }

    int index(const Particle& p) const {
        return (p.x > x) * 1 + (p.y > y) * 2 + (p.z > z) * 4;
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
            if (!child[idx])
                child[idx] = createChild(idx);
            child[idx]->insert(old);
        }

        int idx = index(*p);
        if (!child[idx])
            child[idx] = createChild(idx);
        child[idx]->insert(p);
    }

    Octree* createChild(int idx) {
        real hs = size * real(0.5);
        return new Octree(
            x + ((idx & 1) ? hs : -hs),
            y + ((idx & 2) ? hs : -hs),
            z + ((idx & 4) ? hs : -hs),
            hs
        );
    }

    void computeMass() {
        if (leaf) {
            if (body) {
                m  = body->m;
                cx = body->x;
                cy = body->y;
                cz = body->z;
            } else {
                m = 0;
                cx = cy = cz = 0;
            }

            // single particle → no internal quadrupole
            Qxx = Qyy = Qzz = 0;
            Qxy = Qxz = Qyz = 0;
            return;
        }

        m = 0;
        cx = cy = cz = 0;

        // first: recurse and accumulate mass + COM
        for (auto c : child) {
            if (!c) continue;
            c->computeMass();
            if (c->m == 0) continue;

            m  += c->m;
            cx += c->cx * c->m;
            cy += c->cy * c->m;
            cz += c->cz * c->m;
        }

        if (m > 0) {
            cx /= m;
            cy /= m;
            cz /= m;
        } else {
            cx = cy = cz = 0;
        }

        // second: build quadrupole from children treated as point masses
        Qxx = Qyy = Qzz = 0;
        Qxy = Qxz = Qyz = 0;

        for (auto c : child) {
            if (!c || c->m == 0) continue;

            real rx = c->cx - cx;
            real ry = c->cy - cy;
            real rz = c->cz - cz;
            real r2 = rx*rx + ry*ry + rz*rz;
            real mchild = c->m;

            Qxx += mchild * (3*rx*rx - r2);
            Qyy += mchild * (3*ry*ry - r2);
            Qzz += mchild * (3*rz*rz - r2);

            Qxy += mchild * (3*rx*ry);
            Qxz += mchild * (3*rx*rz);
            Qyz += mchild * (3*ry*rz);
        }
    }
};

inline void bhAccel(Octree* node, const Particle& p, real theta,
                    real& ax, real& ay, real& az)
{
    if (!node || node->m == 0)
        return;

    // Skip self-force
    if (node->leaf && node->body == &p)
        return;

    constexpr real G   = real(1.0);

    // Adaptive softening
    real eps = node->size * real(0.01);

    real dx = node->cx - p.x;
    real dy = node->cy - p.y;
    real dz = node->cz - p.z;

    real r2_soft = dx*dx + dy*dy + dz*dz + eps*eps;
    real dist    = std::sqrt(r2_soft);

    // BH acceptance criterion
    if (node->leaf || (node->size / dist) < theta)
    {
        // Monopole
        real invDist  = real(1.0) / dist;
        real invDist2 = invDist * invDist;
        real invDist3 = invDist * invDist2;

        real fac = G * node->m * invDist3;

        real ax_m = dx * fac;
        real ay_m = dy * fac;
        real az_m = dz * fac;

        // Quadrupole (use non-softened r^2 for shape; still approximate)
        real rx = dx;
        real ry = dy;
        real rz = dz;
        real r2 = rx*rx + ry*ry + rz*rz + real(1e-12); // avoid zero
        real r   = std::sqrt(r2);
        real invr  = real(1.0) / r;
        real invr2 = invr * invr;
        real invr3 = invr * invr2;
        real invr5 = invr3 * invr2;
        real invr7 = invr5 * invr2;

        // q = r_i Q_ij r_j
        real q =
            node->Qxx * rx * rx +
            node->Qyy * ry * ry +
            node->Qzz * rz * rz +
            2 * (node->Qxy * rx * ry +
                 node->Qxz * rx * rz +
                 node->Qyz * ry * rz);

        // ∇q = 2 Q r
        real Qrx = 2 * (node->Qxx * rx + node->Qxy * ry + node->Qxz * rz);
        real Qry = 2 * (node->Qxy * rx + node->Qyy * ry + node->Qyz * rz);
        real Qrz = 2 * (node->Qxz * rx + node->Qyz * ry + node->Qzz * rz);

        // ∇(r^-5) = -5 r^-7 r
        real grad_r5_x = -5 * invr7 * rx;
        real grad_r5_y = -5 * invr7 * ry;
        real grad_r5_z = -5 * invr7 * rz;

        // a_Q = (G/2) [ (∇q) r^-5 + q ∇(r^-5) ]
        real ax_q = (G * real(0.5)) * (Qrx * invr5 + q * grad_r5_x);
        real ay_q = (G * real(0.5)) * (Qry * invr5 + q * grad_r5_y);
        real az_q = (G * real(0.5)) * (Qrz * invr5 + q * grad_r5_z);

        ax += ax_m + ax_q;
        ay += ay_m + ay_q;
        az += az_m + az_q;
        return;
    }

    // Recurse
    for (int i = 0; i < 8; i++)
        if (node->child[i])
            bhAccel(node->child[i], p, theta, ax, ay, az);
}