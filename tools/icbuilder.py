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
    Returns a list of (x, y, z, vx, vy, vz, m) particles.
    """

    if seed_value is not None:
        random.seed(seed_value)

    particles = []

    # --- 1. STAR ---
    star_mass = random.uniform(*star_mass_range)
    particles.append((0.0, 0.0, 0.0, 0.0, 0.0, 0.0, star_mass))

    # --- 2. PLANETS ---
    num_planets = random.randint(*planet_count_range)

    for _ in range(num_planets):
        m = random.uniform(*planet_mass_range)

        # Orbital parameters
        a = random.uniform(*semi_major_axis_range)  # semi-major axis
        e = random.uniform(*eccentricity_range)
        inc = math.radians(random.uniform(*inclination_range))
        phase = random.uniform(0, 2*math.pi)

        # Distance at given phase
        r = a * (1 - e**2) / (1 + e * math.cos(phase))

        # Position in orbital plane
        x = r * math.cos(phase)
        y = r * math.sin(phase)
        z = 0.0

        # Rotate by inclination
        z = x * math.sin(inc)
        x = x * math.cos(inc)

        # Orbital velocity magnitude (vis-viva)
        v = math.sqrt(star_mass * (2/r - 1/a))

        # Velocity direction (perpendicular to radius vector)
        vx = -v * math.sin(phase)
        vy =  v * math.cos(phase)
        vz = 0.0

        # Rotate velocity by inclination
        vz = vx * math.sin(inc)
        vx = vx * math.cos(inc)

        particles.append((x, y, z, vx, vy, vz, m))

        # --- 3. MOONS ---
        if random.random() < moon_chance:
            moon_count = random.randint(1, max_moons)
            for _ in range(moon_count):
                mm = m * random.uniform(0.001, 0.05)
                da = random.uniform(0.01, 0.2)  # moon orbit radius
                phase_m = random.uniform(0, 2*math.pi)

                # Moon position relative to planet
                mx = x + da * math.cos(phase_m)
                my = y + da * math.sin(phase_m)
                mz = z

                # Moon velocity relative to planet
                vm = math.sqrt(m / da)
                mvx = vx - vm * math.sin(phase_m)
                mvy = vy + vm * math.cos(phase_m)
                mvz = vz

                particles.append((mx, my, mz, mvx, mvy, mvz, mm))

    return particles

def big_bang_ic(
    N,
    radius=1.0,
    mass=1.0,
    hubble_k=1.0,
    center_bias=1.0,
    perturb_amp=0.05,
    vel_jitter=0.02
):
    """
    Big-Bang-like initial conditions:
    - Expanding sphere
    - Small density perturbations
    - Radial Hubble flow
    - Isotropic velocity jitter
    """

    particles = []
    m = mass / N

    for _ in range(N):
        # --- 1. Sample radius with optional central bias ---
        u = random.random()
        r = radius * (u ** (1.0 / (3.0 * center_bias)))

        # Random isotropic direction
        z = 2*random.random() - 1
        t = 2 * math.pi * random.random()
        s = math.sqrt(1 - z*z)

        ux = s * math.cos(t)
        uy = s * math.sin(t)
        uz = z

        # Position
        x = r * ux
        y = r * uy
        z = r * uz

        # --- 2. Add density perturbations ---
        # Small random displacement to seed structure formation
        dx = perturb_amp * radius * (random.random() - 0.5)
        dy = perturb_amp * radius * (random.random() - 0.5)
        dz = perturb_amp * radius * (random.random() - 0.5)

        x += dx
        y += dy
        z += dz

        # --- 3. Hubble-like expansion ---
        v = hubble_k * r

        vx = v * ux
        vy = v * uy
        vz = v * uz

        # --- 4. Isotropic velocity jitter ---
        # Proper spherical noise
        jz = 2*random.random() - 1
        jt = 2 * math.pi * random.random()
        js = math.sqrt(1 - jz*jz)

        jx = js * math.cos(jt)
        jy = js * math.sin(jt)
        jz = jz

        vx += vel_jitter * v * jx
        vy += vel_jitter * v * jy
        vz += vel_jitter * v * jz

        particles.append((x, y, z, vx, vy, vz, m))

    return particles