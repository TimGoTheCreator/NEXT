#pragma once
#include "../struct/particle.h"
#include <vector>


// =========================
// Octree definition
// =========================
struct Octree {
    real cx, cy, cz;     // center of mass
    real m;              // total mass
    real x, y, z;        // center of node
    real size;           // half-width
    bool leaf = true;
    Particle* body = nullptr;
    Octree* child[8] = {nullptr};

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
                m = body->m;
                cx = body->x;
                cy = body->y;
                cz = body->z;
            }
            return;
        }

        m = 0;
        cx = cy = cz = 0;

        for (auto c : child) {
            if (!c) continue;
            c->computeMass();
            m += c->m;
            cx += c->cx * c->m;
            cy += c->cy * c->m;
            cz += c->cz * c->m;
        }

        if (m > 0) {
            cx /= m;
            cy /= m;
            cz /= m;
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

    constexpr real G   = real(1.0); // temporarily 1 instead of meter units for galaxies to run efficiently

    // Adaptive softening
    real eps = node->size * real(0.01);  

    real dx = node->cx - p.x;
    real dy = node->cy - p.y;
    real dz = node->cz - p.z;

    real distSq = dx*dx + dy*dy + dz*dz + eps*eps;
    real dist   = std::sqrt(distSq);

    // Correct Barnes–Hut acceptance criterion
    if (node->leaf || (node->size / dist) < theta)
    {
        real invDist  = real(1) / dist;
        real invDist3 = invDist * invDist * invDist;

        real fac = G * node->m * invDist3;

        ax += dx * fac;
        ay += dy * fac;
        az += dz * fac;
        return;
    }

    // Recurse
    for (int i = 0; i < 8; i++)
        if (node->child[i])
            bhAccel(node->child[i], p, theta, ax, ay, az);
}
