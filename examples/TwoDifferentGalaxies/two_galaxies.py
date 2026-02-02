import os, sys

root = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
if root not in sys.path:
    sys.path.insert(0, root)
    
from tools.icbuilder import two_galaxies, save

particles = two_galaxies(
    N1=5000, R1=5.0, M1=1.0, scale1=1.0,
    N2=4000, R2=4.0, M2=0.8, scale2=1.0,
    angle_deg=45.0,
    v_rel=0.5
)

save("two_galaxies.txt", particles)
