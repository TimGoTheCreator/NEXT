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
    real eps = real(1e-6);
    real distSq = dx*dx + dy*dy + dz*dz + eps*eps;
    real dist = std::sqrt(distSq);

    real invDist = real(1) / dist;
    real invDist3 = invDist * invDist * invDist;
        
    real ax = G * b.m * dx * invDist3;
    real ay = G * b.m * dy * invDist3;
    real az = G * b.m * dz * invDist3;

    real inv_bm = real(1) / b.m;
    real scale = -a.m * inv_bm;

    real bx = ax * scale;
    real by = ay * scale;
    real bz = az * scale;

    a.vx += ax * dt;
    a.vy += ay * dt;
    a.vz += az * dt;

    b.vx += bx * dt;
    b.vy += by * dt;
    b.vz += bz * dt;
}
