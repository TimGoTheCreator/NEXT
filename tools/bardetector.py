#!/usr/bin/env python3
"""
Convenience launcher for NEXTInSim Bar Detector
"""
import os
import sys

ROOT_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
if ROOT_DIR not in sys.path:
    sys.path.insert(0, ROOT_DIR)

from NEXTInSim.bardetector import main

if __name__ == "__main__":
    main()
