import random
import math

_particles = []

def particleAdd(x, y, z, vx, vy, vz, m):
    _particles.append((x, y, z, vx, vy, vz, m))

def save(filename):
    with open(filename, "w") as f:
        for p in _particles:

            f.write(" ".join(str(v) for v in p) + "\n")
            
def plummer(N, scale=1.0, mass=1.0):
    for _ in range(N):
        r = scale / math.sqrt(random.random()**(-2/3) - 1)
        theta = math.acos(2*random.random() - 1)
        phi = 2 * math.pi * random.random()

        x = r * math.sin(theta) * math.cos(phi)
        y = r * math.sin(theta) * math.sin(phi)
        z = r * math.cos(theta)

        particleAdd(x, y, z, 0, 0, 0, mass / N)
