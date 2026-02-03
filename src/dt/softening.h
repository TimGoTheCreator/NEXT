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
#include "gravity/octree.h"
#include "struct/particle.h"

inline real nextSoftening(real nodeSize,
                          real nodeMass,
                          real dist)
{
    // Base softening from node size (dominant term)
    real eps_size = nodeSize * real(0.015);

    // Mass-based softening (heavier nodes get slightly more smoothing)
    real eps_mass = std::cbrt(nodeMass) * real(0.002);

    // Distance-based tapering:
    // - strong softening at r → 0
    // - fades out smoothly as r grows
    real eps_taper = real(1.0) / (real(1.0) + dist * real(10.0));

    // Combine components
    real eps = (eps_size + eps_mass) * eps_taper;

    // Minimum floor to avoid zero-softening singularities
    const real eps_min = real(1e-4);
    if (eps < eps_min)
        eps = eps_min;

    return eps;
}

inline real pairSoftening(real ma, real mb)
{
    // Mass-based softening (cubic root gives physical size scale)
    real ea = std::cbrt(ma) * real(0.002);
    real eb = std::cbrt(mb) * real(0.002);

    // Symmetric combination
    real eps2 = ea*ea + eb*eb;

    // Minimum floor
    const real eps_min = real(1e-6);
    if (eps2 < eps_min)
        eps2 = eps_min;

    return eps2;
}