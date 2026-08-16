import os
import sys

nextsim_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "..", "nextsim"))
if nextsim_dir not in sys.path:
    sys.path.insert(0, nextsim_dir)

import nextinsim as ns

# --- HEAVY STRESS TEST SETUP ---
N_PARTICLES = 50000
SCALE_RADIUS = 3.0
TOTAL_MASS = 1.0
DM_FRACTION = 0.85  # 85% Dark Matter halo, 15% Stars

print(f"Generating Cold Collapse Galaxy Stress Test ({N_PARTICLES:,} particles)...")
system = ns.ColdCollapse(
    N=N_PARTICLES,
    radius=SCALE_RADIUS,
    total_mass=TOTAL_MASS,
    dm_fraction=DM_FRACTION,
    model='plummer',
    virial_ratio=0.0
)
system.save("coldcollapse.txt")
print("Saved coldcollapse.txt successfully!")
