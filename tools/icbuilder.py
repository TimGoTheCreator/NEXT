import random
import math

def save(filename, particles):
    with open(filename, "w") as f:
        for p in particles:
            f.write(" ".join(str(v) for v in p) + "\n")

# ----------------------------
# Galaxy / halo ICs with DM
# ----------------------------

def coldPlummer(N, scale=1.0, mass=1.0, dm_fraction=0.9):
    """
    Plummer sphere with optional DM fraction.
    type=0 baryons, type=1 DM
    """
    particles = []
    M_b = mass * (1 - dm_fraction)
    M_dm = mass * dm_fraction

    # baryons
    for _ in range(N):
        r = scale / math.sqrt(random.random()**(-2/3) - 1)
        theta = math.acos(2*random.random() - 1)
        phi = 2 * math.pi * random.random()
        x = r * math.sin(theta) * math.cos(phi)
        y = r * math.sin(theta) * math.sin(phi)
        z = r * math.cos(theta)
        particles.append((x, y, z, 0, 0, 0, M_b / N, 0))

    # DM halo
    for _ in range(N):
        r = scale / math.sqrt(random.random()**(-2/3) - 1)
        theta = math.acos(2*random.random() - 1)
        phi = 2 * math.pi * random.random()
        x = r * math.sin(theta) * math.cos(phi)
        y = r * math.sin(theta) * math.sin(phi)
        z = r * math.cos(theta)
        particles.append((x, y, z, 0, 0, 0, M_dm / N, 1))

    return particles

def diskNoDM(N, radius=10.0, mass=1.0, thickness=0.1):
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

        particles.append((x, y, z, vx, vy, vz, mass / N, 0))

    return particles

def disk(N, radius=10.0, mass=1.0, thickness=0.1, dm_fraction=0.9):
    """
    Exponential disk with DM halo.
    """
    particles = []
    M_b = mass * (1 - dm_fraction)
    M_dm = mass * dm_fraction

    # baryonic disk
    for _ in range(N):
        u = random.random()
        r = -radius * math.log(1 - u)
        phi = 2 * math.pi * random.random()
        x = r * math.cos(phi)
        y = r * math.sin(phi)
        z = random.gauss(0, thickness)
        enclosed = M_b * (1 - math.exp(-r / radius) * (1 + r / radius))
        v = math.sqrt(enclosed / (r + 1e-6))
        vx = -v * math.sin(phi)
        vy =  v * math.cos(phi)
        vz = random.gauss(0, 0.05 * v)
        particles.append((x, y, z, vx, vy, vz, M_b / N, 0))

    # DM halo (simple Hernquist sphere)
    for _ in range(N):
        u = random.random()
        r = radius * math.sqrt(u) / (1 - math.sqrt(u))
        theta = math.acos(2*random.random() - 1)
        phi = 2 * math.pi * random.random()
        x = r * math.sin(theta) * math.cos(phi)
        y = r * math.sin(theta) * math.sin(phi)
        z = r * math.cos(theta)
        particles.append((x, y, z, 0, 0, 0, M_dm / N, 1))

    return particles

def hernquist(N, scale=1.0, mass=1.0, dm_fraction=0.9):
    """
    Hernquist sphere with DM halo.
    """
    particles = []
    M_b = mass * (1 - dm_fraction)
    M_dm = mass * dm_fraction
    a = scale

    # baryons
    for _ in range(N):
        u = random.random()
        r = a * math.sqrt(u) / (1 - math.sqrt(u))
        theta = math.acos(2*random.random() - 1)
        phi = 2 * math.pi * random.random()
        x = r * math.sin(theta) * math.cos(phi)
        y = r * math.sin(theta) * math.sin(phi)
        z = r * math.cos(theta)
        particles.append((x, y, z, 0, 0, 0, M_b / N, 0))

    # DM halo
    for _ in range(N):
        u = random.random()
        r = a * math.sqrt(u) / (1 - math.sqrt(u))
        theta = math.acos(2*random.random() - 1)
        phi = 2 * math.pi * random.random()
        x = r * math.sin(theta) * math.cos(phi)
        y = r * math.sin(theta) * math.sin(phi)
        z = r * math.cos(theta)
        particles.append((x, y, z, 0, 0, 0, M_dm / N, 1))

    return particles

# ----------------------------
# Stellar ICs (no DM)
# ----------------------------

def binary_system(separation=1.0, mass1=1.0, mass2=1.0):
    M = mass1 + mass2
    r1 = -mass2 / M * separation
    r2 =  mass1 / M * separation
    v = math.sqrt(M / separation)
    return [
        (r1, 0, 0, 0,  v, 0, mass1, 0),
        (r2, 0, 0, 0, -v, 0, mass2, 0)
    ]

# (figure-eight, cross, solar system etc. remain unchanged, all type=0)

# ----------------------------
# Cosmology ICs with DM
# ----------------------------

def big_bang_ic(
    N,
    radius=1.0,
    mass=1.0,
    hubble_k=1.0,
    center_bias=1.0,
    perturb_amp=0.05,
    vel_jitter=0.02,
    dm_fraction=0.85
):
    """
    Big-Bang-like ICs with DM fraction.
    """
    particles = []
    M_b = mass * (1 - dm_fraction)
    M_dm = mass * dm_fraction

    # baryons
    for _ in range(N):
        u = random.random()
        r = radius * (u ** (1.0 / (3.0 * center_bias)))
        z = 2*random.random() - 1
        t = 2 * math.pi * random.random()
        s = math.sqrt(1 - z*z)
        ux = s * math.cos(t)
        uy = s * math.sin(t)
        uz = z
        x = r * ux + perturb_amp * radius * (random.random() - 0.5)
        y = r * uy + perturb_amp * radius * (random.random() - 0.5)
        z = r * uz + perturb_amp * radius * (random.random() - 0.5)
        v = hubble_k * r
        vx = v * ux
        vy = v * uy
        vz = v * uz
        particles.append((x, y, z, vx, vy, vz, M_b / N, 0))

    # DM particles
    for _ in range(N):
        u = random.random()
        r = radius * (u ** (1.0 / (3.0 * center_bias)))
        z = 2*random.random() - 1
        t = 2 * math.pi * random.random()
        s = math.sqrt(1 - z*z)
        ux = s * math.cos(t)
        uy = s * math.sin(t)
        uz = z
        x = r * ux
        y = r * uy
        z = r * uz
        v = hubble_k * r
        vx = v * ux
        vy = v * uy
        vz = v * uz
        particles.append((x, y, z, vx, vy, vz, M_dm / N, 1))

    return particles

def three_body_figure_eight(mass=1.0):
    """
    Classic equal-mass figure-eight three-body solution.
    All baryons (type=0).
    """
    m = mass
    x1, y1 = -0.97000436,  0.24308753
    x2, y2 =  0.97000436, -0.24308753
    x3, y3 =  0.0,         0.0
    vx1, vy1 =  0.4662036850,  0.4323657300
    vx2, vy2 =  0.4662036850,  0.4323657300
    vx3, vy3 = -0.93240737,   -0.86473146

    return [
        (x1, y1, 0.0, vx1, vy1, 0.0, m, 0),
        (x2, y2, 0.0, vx2, vy2, 0.0, m, 0),
        (x3, y3, 0.0, vx3, vy3, 0.0, m, 0),
    ]

def random_solar_system(
    seed_value=None,
    star_mass_range=(0.5, 2.0),
    planet_count_range=(1, 12),
    planet_mass_range=(1e-6, 1e-3),
    semi_major_axis_range=(0.3, 40.0),
    eccentricity_range=(0.0, 0.3),
    inclination_range=(0.0, 5.0),  # degrees
    moon_chance=0.25,
    max_moons=3
):
    """
    Generates a random solar system using Keplerian orbits.
    All baryons (type=0).
    """
    if seed_value is not None:
        random.seed(seed_value)

    particles = []

    # --- 1. STAR ---
    star_mass = random.uniform(*star_mass_range)
    particles.append((0.0, 0.0, 0.0, 0.0, 0.0, 0.0, star_mass, 0))

    # --- 2. PLANETS ---
    num_planets = random.randint(*planet_count_range)
    for _ in range(num_planets):
        m = random.uniform(*planet_mass_range)
        a = random.uniform(*semi_major_axis_range)
        e = random.uniform(*eccentricity_range)
        inc = math.radians(random.uniform(*inclination_range))
        phase = random.uniform(0, 2*math.pi)
        r = a * (1 - e**2) / (1 + e * math.cos(phase))
        x = r * math.cos(phase)
        y = r * math.sin(phase)
        z = 0.0
        z = x * math.sin(inc)
        x = x * math.cos(inc)
        v = math.sqrt(star_mass * (2/r - 1/a))
        vx = -v * math.sin(phase)
        vy =  v * math.cos(phase)
        vz = 0.0
        vz = vx * math.sin(inc)
        vx = vx * math.cos(inc)
        particles.append((x, y, z, vx, vy, vz, m, 0))

        # --- 3. MOONS ---
        if random.random() < moon_chance:
            moon_count = random.randint(1, max_moons)
            for _ in range(moon_count):
                mm = m * random.uniform(0.001, 0.05)
                da = random.uniform(0.01, 0.2)
                phase_m = random.uniform(0, 2*math.pi)
                mx = x + da * math.cos(phase_m)
                my = y + da * math.sin(phase_m)
                mz = z
                vm = math.sqrt(m / da)
                mvx = vx - vm * math.sin(phase_m)
                mvy = vy + vm * math.cos(phase_m)
                mvz = vz
                particles.append((mx, my, mz, mvx, mvy, mvz, mm, 0))

    return particles
