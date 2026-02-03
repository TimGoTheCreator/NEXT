// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

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