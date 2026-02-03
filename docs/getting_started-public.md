# What is NEXT (Public)

NEXT is an N-Body gravity code for astrophysics and such missions.

## Code / AstroPhysics

NEXT is a Computer program made in the low-level programming language C++ designed for running on workstations
or supercomputers that simulates forces between objects called **"Newtonian Gravity"**

## What does N-Body mean?

The term "N-Body" simply means **"multiple"** or **"many"** bodies, N could stand for 1, 500, or even millions!
**"Body"** refers to such object in the simulation, it could be a star, a planet, or just particles.

**So, an "N-Body" simulation is just a way of calculating how "N" objects move under gravity when they all pull in at eachother in the same time.**

## What's the purpose of NEXT?

NEXT was made as a new simulation code written from **scratch**, to simulate interaction of bodies
in short compute times.

## How is it efficient?

The challenge is: if you have millions of stars, calculating the pull of each other on every one takes forever
with the basic formula.

The trick: **Barnes-Hut** instead of looking at each particle one-by-one NEXT scans the simulation and divides particles into groups
and starts to treat them as "big stars".

NEXT, however doesn't use basic Barnes-Hut; it uses **Barnes-Hut With High Ordering Multipoles**, instead of treating it like a "big star", it adds extra detail using a **dipole, quadrupole, and higher terms** into the calculation, **adding shape and spread
preserving accuracy of the engine**

That means that NEXT can simulate thousands (if not millions) particles at once interacting, while still being fast!
