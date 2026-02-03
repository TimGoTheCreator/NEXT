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