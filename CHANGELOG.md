# NEXT 2026.08.13
#### Major changes:
- **Gadget-Compliant HDF5 Export**: Upgraded HDF5 output with `/Header` metadata and separate `/PartType4` (Stars) and `/PartType1` (Dark Matter) groups.
- **Fixed Dark Matter Reloading Bug**: Fixed issue where stars in HDF5 snapshots were converted to Dark Matter upon reloading.
- **Octree Stability & Collapse Acceleration**: Added recursion depth guards to prevent Octree stalls during dense galaxy collapses.

#### Minor changes:
- **Flexible Text Loader**: Added fallback support for optional 8th column in initial condition `.txt` files.
- **Codebase Cleanups**: Unified multi-threaded rank logging with `log_once()` in `begrun.cpp`.
- Fixed transliteration (Timofiy -> Tymofiy).

---

# NEXT 2026.08.12
#### Major changes:
- Added optional `[max_steps]` CLI argument to automatically limit total simulation steps / dumps.

#### Minor changes:
- Updated CLI parser (`argparse`) to support 6th optional positional parameter.
- Updated startup splash screen to display `Max Steps` when configured.
- Fix citation transliteration (Timofey -> Timofiy).
- Unified licensing under GNU GPLv3+ (removed LGPL files).

---

# NEXT 2026.02.08
#### Major changes:
Added Dark Matter

---

# NEXT 2026.02.03

#### Major changes:

Added Barnes Hut Octree ( O (N log N)) 
Added VTK/VTU output
Added splash screen on start + precision and threads are printed

#### Minor changes:
Changed naming scheme to CalVer YYYY-MM-DD
Added the following examples:
- TwoDifferentGalaxies
- ColdCollapseGalaxy
- GalaxyDemo

---

# The first release of NEXT (0.01) 
## Features:
O(N²) 
Basic simulation
Loading initial conditions (txt struct positions)
