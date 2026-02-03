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

/* Softening for BarnesHut */
inline real nextSoftening(real nodeSize,
                          real nodeMass,
                          real dist)
{
    // Base softening from node size
    real eps_size = nodeSize * real(0.015);

    // Mass-based softening (physical radius proxy)
    real eps_mass = std::cbrt(nodeMass) * real(0.002);

    // Distance taper: strong at r→0, fades smoothly
    real eps_taper = real(1.0) / (real(1.0) + dist * real(10.0));

    // Combine
    real eps = (eps_size + eps_mass) * eps_taper;

    // Minimum floor
    const real eps_min = real(1e-4);
    if (eps < eps_min)
        eps = eps_min;

    return eps;
}

/* Softening for Gravity kernel */
inline real pairSoftening(real ma, real mb)
{
    // Physical radius proxy
    real ea = std::cbrt(ma) * real(0.002);
    real eb = std::cbrt(mb) * real(0.002);

    // Symmetric combination (quadrature)
    real eps = std::sqrt(ea*ea + eb*eb);

    // Minimum floor
    const real eps_min = real(1e-4);
    if (eps < eps_min)
        eps = eps_min;

    return eps; // return epsilon
}