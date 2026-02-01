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

    #pragma omp parallel for schedule(static)
    for (int i = 0; i < p.size(); i++) {
       bhForce(&root, p[i], theta, dt);
    }

    for (auto& a : p) {
        a.x += a.vx * dt;
        a.y += a.vy * dt;
        a.z += a.vz * dt;
    }
    
    
}
