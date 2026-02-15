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
#include <cmath>
#include <algorithm>

/**
 * @brief Softening for Barnes-Hut node interactions.
 * Note: nodeMass and dist are passed as individual reals from the SoA arrays.
 */
inline real nextSoftening(real nodeSize, real nodeMass, real dist) {
    // std::pow(x, 1/3) or std::cbrt is slow. 
    // In SoA loops, this is often the bottleneck.
    real eps_size = nodeSize * real(0.015);
    real eps_mass = std::cbrt(nodeMass) * real(0.002);

    // Distance taper: strong at r->0, fades smoothly
    real eps_taper = real(1.0) / (real(1.0) + dist * real(10.0));

    // Combine
    real eps = (eps_size + eps_mass) * eps_taper;

    // Minimum floor - using std::max is cleaner and easier for the compiler to optimize
    return std::max(eps, real(1e-4));
}

/**
 * @brief Softening for direct particle-particle gravity kernels.
 * ma and mb are the masses pulled from the ps.m[i] and ps.m[j] arrays.
 */
inline real pairSoftening(real ma, real mb) {
    // Physical radius proxy
    real ea = std::cbrt(ma) * real(0.002);
    real eb = std::cbrt(mb) * real(0.002);

    // Symmetric combination (quadrature)
    real epsSq = ea * ea + eb * eb;
    real eps = std::sqrt(epsSq);

    // Minimum floor
    return std::max(eps, real(1e-4));
}
