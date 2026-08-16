#!/usr/bin/env python3
"""
Convenience launcher for NEXTInSim NASA Volumetric Renderer & 3D Viewport
"""
import os
import sys

ROOT_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
NEXTSIM_DIR = os.path.abspath(os.path.join(ROOT_DIR, "..", "nextsim"))

for p in [ROOT_DIR, NEXTSIM_DIR]:
    if os.path.isdir(p) and p not in sys.path:
        sys.path.insert(0, p)

try:
    from renderer import main
except ImportError:
    from NEXTInSim.renderer import main

if __name__ == "__main__":
    main()
