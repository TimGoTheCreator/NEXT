import os, sys

root = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
if root not in sys.path:
    sys.path.insert(0, root)

from tools.icbuilder import particleAdd, save

particleAdd(0, 0, 0, 0, 0, 0, 10)
particleAdd(5, 0, 0, 0, 1, 0, 1)

save("two_body.txt")