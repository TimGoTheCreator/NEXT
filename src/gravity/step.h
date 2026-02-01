#pragma once
#include "../struct/particle.h"
#include <vector>
#include "octree.h"
#include "floatdef.h"

inline void Step(std::vector<Particle>& p, real dt) {
    Octree root(0,0,0, 1e9);

    for (auto& a : p) {
        root.insert(&a);
    }

    root.computeMass();

    real theta = 0.5;

    for (auto& a : p) {
        bhForce(&root, a, theta, dt);
    }

    for (auto& a : p) {
        a.x += a.vx * dt;
        a.y += a.vy * dt;
        a.z += a.vz * dt;
    }
    
    
}
