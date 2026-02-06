import os, sys

root = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
if root not in sys.path:
    sys.path.insert(0, root)

from tools.icbuilder import diskNoDM, save

particles = diskNoDM(20000, radius=0.5, mass=1.0, thickness=0.2)
save("galaxy.txt", particles)
