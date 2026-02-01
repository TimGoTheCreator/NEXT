#pragma once
#include "struct/particle.h"
#include <vector>
#include <algorithm>
#include <cmath>
#include "floatdef.h"

real computeAdaptiveDt(const std::vector<Particle>& p, real base_dt)
{
    real maxSpeed = 0;

    for (const auto& a : p)
    {
        real speed = std::sqrt(a.vx*a.vx + a.vy*a.vy + a.vz*a.vz);
        if (speed > maxSpeed)
            maxSpeed = speed;
    }

    // If everything is basically stationary, use base dt
    if (maxSpeed < real(1e-8))
        return base_dt;

    // Smaller dt when speeds are high
    real dt = base_dt / (1 + maxSpeed);

    // Clamp dt to a reasonable range
    dt = std::max(dt, base_dt * real(0.01)); // never smaller than 1% of base
    dt = std::min(dt, base_dt * real(1.0));  // never larger than base

    return dt;
}
