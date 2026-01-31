_particles = []

def particleAdd(x, y, z, vx, vy, vz, m):
    _particles.append((x, y, z, vx, vy, vz, m))

def save(filename):
    with open(filename, "w") as f:
        for p in _particles:
            f.write(" ".join(str(v) for v in p) + "\n")