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

    auto buildTree = [&](Octree*& root) {
        // Compute bounding box
        real minx=+1e30, miny=+1e30, minz=+1e30;
        real maxx=-1e30, maxy=-1e30, maxz=-1e30;

        for (auto& a : p) {
            minx = std::min(minx, a.x);
            miny = std::min(miny, a.y);
            minz = std::min(minz, a.z);
            maxx = std::max(maxx, a.x);
            maxy = std::max(maxy, a.y);
            maxz = std::max(maxz, a.z);
        }

        real cx = (minx + maxx) * 0.5;
        real cy = (miny + maxy) * 0.5;
        real cz = (minz + maxz) * 0.5;
        real size = std::max({maxx-minx, maxy-miny, maxz-minz}) * 0.5;

        if (size <= 0) size = 1; // safety

        root = new Octree(cx, cy, cz, size);

        for (auto& a : p)
            root->insert(&a);

        root->computeMass();
    };

    // =========================
    // First Kick (dt/2)
    // =========================
    {
        Octree* root = nullptr;
        buildTree(root);

        #pragma omp parallel for schedule(static)
        for (int i = 0; i < (int)p.size(); i++) {
            real ax = 0, ay = 0, az = 0;
            bhAccel(root, p[i], theta, ax, ay, az);

            p[i].vx += ax * half;
            p[i].vy += ay * half;
            p[i].vz += az * half;
        }

        delete root;
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
        Octree* root = nullptr;
        buildTree(root);

        #pragma omp parallel for schedule(static)
        for (int i = 0; i < (int)p.size(); i++) {
            real ax = 0, ay = 0, az = 0;
            bhAccel(root, p[i], theta, ax, ay, az);

            p[i].vx += ax * half;
            p[i].vy += ay * half;
            p[i].vz += az * half;
        }

        delete root;
    }
}
