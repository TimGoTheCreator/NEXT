# What is NEXT? (Astrophysical Context)

NEXT (**Newtonian EXact Trajectories**) is an N-body gravitational simulation engine written in C++ from the ground up.

Currently, NEXT models **pure Newtonian gravity** using gravitational multipole interactions and symplectic integration. Future extensions may incorporate gas dynamics, radiative processes, or dark matter models.

---

## Numerical Stability & Algorithm

### Barnes-Hut Octree with Higher-Order Multipoles
NEXT builds a **Barnes-Hut Octree** O(N log N) enhanced with **higher-order multipole expansions** (including dipole and quadrupole terms). Rather than approximating distant particle groups as point-mass monopoles, NEXT accounts for spatial mass distributions and geometric shape.

Key advantages:
- **Reduced Force Error:** Significantly lower force truncation errors compared to standard monopole Barnes-Hut.
- **Enhanced Phase Space & Energy Conservation:** Superior long-term stability for bound Hamiltonian systems.
- **Accurate Cluster Dynamics:** Reliable modeling of galaxy interactions, tidal streams, and dense star clusters.
- **Tunable Precision/Speed:** Multipole expansion order and cell opening angle theta allow precise trade-offs between computational speed and numerical accuracy.

---

## Integrator & Physical Units

### Symplectic KDK Leapfrog
NEXT uses a 2nd-order **Kick-Drift-Kick (KDK) Leapfrog Integrator**:
- **Symplectic:** Preserves phase-space volume and prevents secular energy drift over long integration periods.
- **Adaptive Timestepping & Softening:** Dynamically adjusts numerical time steps based on local inter-particle distances and gravitational acceleration, employing adaptive softening to prevent singular close-encounter forces.

### Units & Constants
NEXT does not impose rigid unit systems. Physical parameters scale directly with the gravitational constant G (G = 1.0 by default).

---

## Simulation Capabilities

NEXT is optimized for gravitationally dominated astrophysical systems:
- Stellar clusters and globular clusters
- Galactic disc dynamics and galaxy mergers
- Tidal stream evolution
- General N-body systems (N = 1,000 to 10,000,000+)
