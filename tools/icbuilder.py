import random
import math

def save(filename, particles):
    with open(filename, "w") as f:
        for p in particles:
            f.write(" ".join(str(v) for v in p) + "\n")


def coldPlummer(N, scale=1.0, mass=1.0):
    particles = []
    for _ in range(N):
        r = scale / math.sqrt(random.random()**(-2/3) - 1)
        theta = math.acos(2*random.random() - 1)
        phi = 2 * math.pi * random.random()

        x = r * math.sin(theta) * math.cos(phi)
        y = r * math.sin(theta) * math.sin(phi)
        z = r * math.cos(theta)

        particles.append((x, y, z, 0, 0, 0, mass / N))

    return particles


def disk(N, radius=10.0, mass=1.0, thickness=0.1):
    """
    Generates a simple equilibrium-ish exponential disk galaxy.
    - N: number of particles
    - radius: scale length of the disk
    - mass: total mass of the disk
    - thickness: vertical Gaussian thickness
    """

    particles = []
    M = mass

    for _ in range(N):
        # Sample radius from exponential disk: p(r) ~ r * exp(-r/R)
        u = random.random()
        r = -radius * math.log(1 - u)

        # Random angle
        phi = 2 * math.pi * random.random()

        # Position in the plane
        x = r * math.cos(phi)
        y = r * math.sin(phi)

        # Vertical position (Gaussian)
        z = random.gauss(0, thickness)

        # Circular velocity
        enclosed = M * (1 - math.exp(-r / radius) * (1 + r / radius))
        v = math.sqrt(enclosed / (r + 1e-6))

        # Tangential velocity
        vx = -v * math.sin(phi)
        vy =  v * math.cos(phi)

        # Small vertical velocity dispersion
        vz = random.gauss(0, 0.05 * v)

        particles.append((x, y, z, vx, vy, vz, mass / N))

    return particles