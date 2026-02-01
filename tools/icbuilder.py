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

def hernquist(N, scale=1.0, mass=1.0):
    """
    Generates a Hernquist sphere with scale radius 'scale' and total mass 'mass'.
    Velocities are set to zero (cold start).
    """
    particles = []
    M = mass
    a = scale

    for _ in range(N):
        # Sample radius from Hernquist distribution
        u = random.random()
        r = a * math.sqrt(u) / (1 - math.sqrt(u))

        # Random isotropic angles
        theta = math.acos(2*random.random() - 1)
        phi = 2 * math.pi * random.random()

        # Convert to Cartesian
        x = r * math.sin(theta) * math.cos(phi)
        y = r * math.sin(theta) * math.sin(phi)
        z = r * math.cos(theta)

        particles.append((x, y, z, 0, 0, 0, M / N))

    return particles

def rotate_z(x, y, z, angle):
    ca = math.cos(angle)
    sa = math.sin(angle)
    return (ca*x - sa*y, sa*x + ca*y, z)

def two_galaxies(
    N1, R1, M1, scale1,
    N2, R2, M2, scale2,
    angle_deg=0.0,
    v_rel=1.0
):
    """
    Builds a two-galaxy merger:
    - Galaxy 1: Hernquist sphere at (-R1, 0, 0)
    - Galaxy 2: Hernquist sphere at (+R2, 0, 0), rotated by angle_deg
    - v_rel: relative velocity along y-axis
    """

    g1 = hernquist(N1, scale1, M1)
    g2 = hernquist(N2, scale2, M2)

    angle = math.radians(angle_deg)
    particles = []

    # Galaxy 1: shift left, give velocity +v_rel/2
    for (x, y, z, vx, vy, vz, m) in g1:
        particles.append((
            x - R1, y, z,
            vx, vy + v_rel/2, vz,
            m
        ))

    # Galaxy 2: rotate, shift right, give velocity -v_rel/2
    for (x, y, z, vx, vy, vz, m) in g2:
        xr, yr, zr = rotate_z(x, y, z, angle)
        particles.append((
            xr + R2, yr, zr,
            vx, vy - v_rel/2, vz,
            m
        ))

    return particles

def composite_galaxy(
    N_bulge, bulge_scale, bulge_mass,
    N_disk,  disk_scale,  disk_mass, disk_thickness=0.1,
    N_halo=0, halo_scale=1.0, halo_mass=0.0
):
    """
    Builds a composite galaxy:
    - Hernquist bulge
    - Exponential disk
    - Optional Hernquist halo
    """

    particles = []

    # Bulge
    if N_bulge > 0:
        bulge = hernquist(N_bulge, bulge_scale, bulge_mass)
        particles.extend(bulge)

    # Disk
    if N_disk > 0:
        disk_p = disk(N_disk, disk_scale, disk_mass, disk_thickness)
        particles.extend(disk_p)

    # Halo (optional)
    if N_halo > 0:
        halo = hernquist(N_halo, halo_scale, halo_mass)
        particles.extend(halo)

    return particles

def uniform_sphere(N, radius=1.0, mass=1.0):
    particles = []
    for _ in range(N):
        # Sample inside sphere with r ∝ u^(1/3)
        u = random.random()
        r = radius * (u ** (1/3))

        theta = math.acos(2*random.random() - 1)
        phi = 2 * math.pi * random.random()

        x = r * math.sin(theta) * math.cos(phi)
        y = r * math.sin(theta) * math.sin(phi)
        z = r * math.cos(theta)

        particles.append((x, y, z, 0, 0, 0, mass / N))
    return particles

def binary_system(separation=1.0, mass1=1.0, mass2=1.0):
    M = mass1 + mass2
    r1 = -mass2 / M * separation
    r2 =  mass1 / M * separation

    v = math.sqrt(M / separation)

    return [
        (r1, 0, 0, 0,  v, 0, mass1),
        (r2, 0, 0, 0, -v, 0, mass2)
    ]

def three_body_figure_eight(mass=1.0):
    """
    Classic equal-mass figure-eight three-body solution.
    Positions and velocities are scaled to G = 1.
    """

    m = mass

    # Standard initial conditions (Chenciner & Montgomery / Moore orbit)
    x1, y1 = -0.97000436,  0.24308753
    x2, y2 =  0.97000436, -0.24308753
    x3, y3 =  0.0,         0.0

    vx1, vy1 =  0.4662036850,  0.4323657300
    vx2, vy2 =  0.4662036850,  0.4323657300
    vx3, vy3 = -0.93240737,   -0.86473146

    return [
        (x1, y1, 0.0, vx1, vy1, 0.0, m),
        (x2, y2, 0.0, vx2, vy2, 0.0, m),
        (x3, y3, 0.0, vx3, vy3, 0.0, m),
    ]

def four_body_cross(mass=1.0):
    """
    Symmetric four-body 'cross' choreography.
    Four equal masses start on cardinal axes and orbit the barycenter.
    """

    m = mass

    # Standard symmetric initial conditions (scaled for G = 1)
    # Positions:
    x1, y1 =  1.0,  0.0
    x2, y2 = -1.0,  0.0
    x3, y3 =  0.0,  1.0
    x4, y4 =  0.0, -1.0

    # Velocities chosen to produce the cross choreography
    vx1, vy1 =  0.0,  0.5
    vx2, vy2 =  0.0, -0.5
    vx3, vy3 = -0.5,  0.0
    vx4, vy4 =  0.5,  0.0

    return [
        (x1, y1, 0.0, vx1, vy1, 0.0, m),
        (x2, y2, 0.0, vx2, vy2, 0.0, m),
        (x3, y3, 0.0, vx3, vy3, 0.0, m),
        (x4, y4, 0.0, vx4, vy4, 0.0, m),
    ]