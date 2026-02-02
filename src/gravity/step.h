// src/gravity/step.h
#pragma once
#include "../struct/particle.h"
#include <vector>
#include "octree.h"
#include "floatdef.h"

inline void Step(std::vector<Particle>& p, real dt) {
    if (p.empty()) return;

    real theta = 0.5;
    real half  = dt * real(0.5);

    // =========================
    // First Kick (dt/2)
    // =========================
    {
        Octree root(0,0,0, 1e9);

        for (auto& a : p) {
            root.insert(&a);
        }
        root.computeMass();

        #pragma omp parallel for schedule(static)
        for (int i = 0; i < (int)p.size(); i++) {
            real ax = 0, ay = 0, az = 0;
            bhAccel(&root, p[i], theta, ax, ay, az);

            p[i].vx += ax * half;
            p[i].vy += ay * half;
            p[i].vz += az * half;
        }
    }

    // =========================
    // Drift (dt)
    // =========================
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < (int)p.size(); i++) {
        p[i].x += p[i].vx * dt;
        p[i].y += p[i].vy * dt;
        p[i].z += p[i].vz * dt;
    }

    // =========================
    // Second Kick (dt/2)
    // =========================
    {
        Octree root(0,0,0, 1e9);

        for (auto& a : p) {
            root.insert(&a);
        }
        root.computeMass();

        #pragma omp parallel for schedule(static)
        for (int i = 0; i < (int)p.size(); i++) {
            real ax = 0, ay = 0, az = 0;
            bhAccel(&root, p[i], theta, ax, ay, az);

            p[i].vx += ax * half;
            p[i].vy += ay * half;
            p[i].vz += az * half;
        }
    }
}