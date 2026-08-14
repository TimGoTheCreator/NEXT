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

/**
 * @brief Computes analytic NFW (Navarro-Frenk-White) Dark Matter Halo Acceleration.
 * Used for simulating stars orbiting inside a true cosmological Dark Matter Halo.
 */
inline void applyNFWHaloAccel(real x, real y, real z, real M_halo, real r_scale,
                             real& ax, real& ay, real& az) {
    if (M_halo <= real(0) || r_scale <= real(0)) return;

    constexpr real G = real(1.0);
    real r2 = x * x + y * y + z * z;
    real r = std::sqrt(r2 + real(1e-10));
    real x_scale = r / r_scale;

    // NFW mass profile factor: ln(1 + x) - x / (1 + x)
    real f_nfw = std::log(real(1.0) + x_scale) - (x_scale / (real(1.0) + x_scale));

    // Gravitational acceleration towards halo center
    real mag = (G * M_halo * f_nfw) / (r2 * r + real(1e-6));

    ax -= x * mag;
    ay -= y * mag;
    az -= z * mag;
}
