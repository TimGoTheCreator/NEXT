#pragma once
#include <immintrin.h>
#include "floatdef.h"
#include <cmath>
#include <string>

struct alignas(32) Particle {
    real x, y, z;
    real vx, vy, vz;
    real m;
};

inline void Gravity(Particle& a, Particle& b, real dt)
{
    constexpr real G = real(6.67430e-11);
    real dx = b.x - a.x;
    real dy = b.y - a.y;
    real dz = b.z - a.z;
    real distSq = dx*dx + dy*dy + dz*dz;
    real dist = std::sqrt(distSq);

    if (dist > real(1e-5))
    {
        real invDist = real(1) / dist;
        real invDist3 = invDist * invDist * invDist;
        
        real ax = G * b.m * dx * invDist3;
        real ay = G * b.m * dy * invDist3;
        real az = G * b.m * dz * invDist3;

        real bx = -ax * a.m / b.m;
        real by = -ay * a.m / b.m;
        real bz = -az * a.m / b.m;

        a.vx += ax * dt;
        a.vy += ay * dt;
        a.vz += az * dt;

        b.vx += bx * dt;
        b.vy += by * dt;
        b.vz += bz * dt;
    }
}
