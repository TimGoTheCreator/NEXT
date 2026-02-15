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
#include "../struct/particle.h"
#include "floatdef.h"
#include "octree.h"
#include <vector>
#include <memory>
#include <algorithm>

inline void Step(std::vector<Particle> &p, real dt) {
  if (p.empty()) return;

  real theta = 0.5;
  real half = dt * real(0.5);

  // Helper lambda that returns a unique_ptr
  auto buildTree = [&]() -> std::unique_ptr<Octree> {
    real minx = +1e30, miny = +1e30, minz = +1e30;
    real maxx = -1e30, maxy = -1e30, maxz = -1e30;

    for (const auto &a : p) {
      minx = std::min(minx, a.x); miny = std::min(miny, a.y); minz = std::min(minz, a.z);
      maxx = std::max(maxx, a.x); maxy = std::max(maxy, a.y); maxz = std::max(maxz, a.z);
    }

    real cx = (minx + maxx) * 0.5;
    real cy = (miny + maxy) * 0.5;
    real cz = (minz + maxz) * 0.5;
    real size = std::max({maxx - minx, maxy - miny, maxz - minz}) * real(0.5);

    if (size <= 0) size = 1;

    // Create the owned root
    auto root = std::make_unique<Octree>(cx, cy, cz, size);

    for (auto &a : p)
      root->insert(&a);

    root->computeMass();
    return root; 
  };

  // --- First Kick (dt/2) ---
  {
    std::unique_ptr<Octree> root = buildTree();

#pragma omp parallel for schedule(static)
    for (int i = 0; i < (int)p.size(); i++) {
      real ax = 0, ay = 0, az = 0;
      // Pass the raw pointer via .get() for the traversal
      bhAccel(root.get(), p[i], theta, ax, ay, az);

      p[i].vx += ax * half;
      p[i].vy += ay * half;
      p[i].vz += az * half;
    }
    // No 'delete root' needed! It happens automatically here.
  }

  // --- Drift (dt) ---
#pragma omp parallel for schedule(static)
  for (int i = 0; i < (int)p.size(); i++) {
    p[i].x += p[i].vx * dt;
    p[i].y += p[i].vy * dt;
    p[i].z += p[i].vz * dt;
  }

  // --- Second Kick (dt/2) ---
  {
    std::unique_ptr<Octree> root = buildTree();

#pragma omp parallel for schedule(static)
    for (int i = 0; i < (int)p.size(); i++) {
      real ax = 0, ay = 0, az = 0;
      bhAccel(root.get(), p[i], theta, ax, ay, az);

      p[i].vx += ax * half;
      p[i].vy += ay * half;
      p[i].vz += az * half;
    }
  }
}
