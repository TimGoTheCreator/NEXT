import os, sys

root = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
if root not in sys.path:
    sys.path.insert(0, root)

from tools.icbuilder import coldPlummer, save

particles = coldPlummer(1000, scale=1.0, mass=1.0)
save("coldcollapse.txt", particles)
