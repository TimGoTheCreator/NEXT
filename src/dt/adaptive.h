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
#include "floatdef.h"
#include "struct/particle.h"
#include <algorithm>
#include <cmath>
#include <vector>

/**
 * @brief Computes a global adaptive time-step based on the maximum velocity in the system.
 * Updated for SoA (Structure of Arrays) for better cache performance.
 */
real computeAdaptiveDt(const Particle &p, real base_dt) {
    real maxSpeedSq = 0;
    const size_t N = p.size();

    // High-speed linear scan through velocity arrays
    // The compiler can easily vectorize this with SIMD (AVX/SSE)
    for (size_t i = 0; i < N; ++i) {
        real speedSq = p.vx[i] * p.vx[i] + p.vy[i] * p.vy[i] + p.vz[i] * p.vz[i];
        if (speedSq > maxSpeedSq)
            maxSpeedSq = speedSq;
    }

    real maxSpeed = std::sqrt(maxSpeedSq);
    
    // Safety clamp to prevent dt from exploding or becoming zero
    maxSpeed = std::min(maxSpeed, real(1e4));

    // If everything is basically stationary, use base dt
    if (maxSpeed < real(1e-8))
        return base_dt;

    // Standard Courant-like condition: smaller dt when speeds are high
    real dt = base_dt / (1 + maxSpeed);

    // Clamp dt to a reasonable range
    dt = std::max(dt, base_dt * real(0.01)); // never smaller than 1% of base
    dt = std::min(dt, base_dt * real(1.0));  // never larger than base

    return dt;
}
