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
    constexpr real G = real(1.0);
    real dx = b.x - a.x;
    real dy = b.y - a.y;
    real dz = b.z - a.z;

    real distSq0 = dx*dx + dy*dy + dz*dz;
    real dist0   = std::sqrt(distSq0);

    // adaptive softening = 1% of separation
    real eps = dist0 * real(0.01);

    real distSq = distSq0 + eps*eps;
    real dist   = std::sqrt(distSq);

    real invDist = real(1) / dist;
    real invDist3 = invDist * invDist * invDist;

    real ax = G * b.m * dx * invDist3;
    real ay = G * b.m * dy * invDist3;
    real az = G * b.m * dz * invDist3;

    real bx = -G * a.m * dx * invDist3;
    real by = -G * a.m * dy * invDist3;
    real bz = -G * a.m * dz * invDist3;

    

    a.vx += ax * dt;
    a.vy += ay * dt;
    a.vz += az * dt;

    b.vx += bx * dt;
    b.vy += by * dt;
    b.vz += bz * dt;
}


