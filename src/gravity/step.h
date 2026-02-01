#pragma once
#include "../struct/particle.h"
#include <vector>

inline void Step(std::vector<Particle>& p, real dt) {
    const size_t count = p.size();
    for (size_t i = 0; i < count; i++)
    {
        for (size_t j = i + 1; j < count; j++)
        {
            Gravity(p[i], p[j], dt);
        }
        
    }

    for (size_t i = 0; i < count; i++)
    {
        p[i].x += p[i].vx * dt;
        p[i].y += p[i].vy * dt;
        p[i].z += p[i].vz * dt;
    }
    
    
}