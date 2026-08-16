#!/usr/bin/env python3
"""
Convenience launcher for NEXTInSim Bar Detector
"""
import os
import sys

# Add nextsim to python path
nextsim_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "..", "nextsim"))
if nextsim_dir not in sys.path:
    sys.path.insert(0, nextsim_dir)

try:
    from bardetector import main
except ImportError:
    from nextsim.bardetector import main

if __name__ == "__main__":
    main()
