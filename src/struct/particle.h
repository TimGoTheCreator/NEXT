#pragma once
#include <cmath>
#include "floatdef.h"

struct alignas(32) Particle {
    real x, y, z;
    real vx, vy, vz;
    real m;
};

inline void Gravity(Particle& a, Particle& b, real dt)
{
    constexpr real G    = real(1.0);
    constexpr real eps2 = real(1e-4);

    // Relative position
    real dx = b.x - a.x;
    real dy = b.y - a.y;
    real dz = b.z - a.z;

    // Softened squared distance
    real r2 = dx*dx + dy*dy + dz*dz + eps2;

    // Inverse distance and inverse distance cubed
    real invR = std::sqrt(real(1.0) / r2);
    real invR3 = invR * invR * invR;

    // Accelerations (pairwise, symmetric)
    real ax = G * b.m * dx * invR3;
    real ay = G * b.m * dy * invR3;
    real az = G * b.m * dz * invR3;

    real bx = -G * a.m * dx * invR3;
    real by = -G * a.m * dy * invR3;
    real bz = -G * a.m * dz * invR3;

    a.vx += ax * dt;
    a.vy += ay * dt;
    a.vz += az * dt;

    b.vx += bx * dt;
    b.vy += by * dt;
    b.vz += bz * dt;
}