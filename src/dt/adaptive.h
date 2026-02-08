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

real computeAdaptiveDt(const std::vector<Particle> &p, real base_dt) {
  real maxSpeed = 0;

  for (const auto &a : p) {
    real speed = std::sqrt(a.vx * a.vx + a.vy * a.vy + a.vz * a.vz);
    if (speed > maxSpeed)
      maxSpeed = speed;
  }

  maxSpeed = std::min(maxSpeed, real(1e4));

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
