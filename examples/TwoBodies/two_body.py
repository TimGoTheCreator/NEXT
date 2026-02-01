import os, sys

root = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
if root not in sys.path:
    sys.path.insert(0, root)

from tools.icbuilder import save

# Build your two-body system manually
particles = [
    (0, 0, 0, 0, 0, 0, 10),   # massive body
    (5, 0, 0, 0, 1, 0, 1),    # lighter orbiting body
]

save("two_body.txt", particles)