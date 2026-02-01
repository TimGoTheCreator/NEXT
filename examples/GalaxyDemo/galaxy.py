import os, sys
root = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
if root not in sys.path:

    sys.path.insert(0, root)

from tools.icbuilder import particleAdd, save, plummer

plummer(1000)
save("galaxy.txt") 
