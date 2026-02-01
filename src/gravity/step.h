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

    #pragma omp parallel for
    for (int i = 0; i < p.size(); i++) {
    p[i].x += p[i].vx * dt;
    p[i].y += p[i].vy * dt;
    p[i].z += p[i].vz * dt;
}
    
    
}


