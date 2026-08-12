# Overview of NEXT

NEXT (**Newtonian EXact Trajectories**) is a high-performance N-body gravitational simulation engine designed for astrophysics and computational physics.

---

## What is an N-Body Simulation?

In physics and astronomy, an **N-body simulation** calculates how N distinct physical objects interact with one another under gravitational forces. 

- N represents the particle count (ranging from a few bodies up to millions of particles).
- **Body** refers to any discrete mass point—such as a planet, star, or dark matter tracer.

An N-body simulator solves the coupled equations of motion for all N bodies simultaneously over discrete time steps.

---

## Computational Efficiency in NEXT

Direct calculation of gravitational forces between all pairs of N particles requires O(N^2) calculations per step, which becomes computationally prohibitive for large particle counts (N > 100,000).

NEXT addresses this challenge using the **Barnes-Hut Octree Algorithm** O(N log N):
1. **Spatial Decomposition:** Particles are recursively partitioned into a 3D octree hierarchy.
2. **Higher-Order Multipoles:** Distant nodes are approximated using multipole moment expansions (dipole and quadrupole terms) rather than simple point masses.

This approach delivers scalable, high-throughput simulation capability while preserving strict numerical accuracy.
