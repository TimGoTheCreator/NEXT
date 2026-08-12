# NEXT 2026.08.12
#### Major changes:
- Added optional `[max_steps]` CLI argument to automatically limit total simulation steps / dumps.

#### Minor changes:
- Updated CLI parser (`argparse`) to support 6th optional positional parameter.
- Updated startup splash screen to display `Max Steps` when configured.
- Fix citation transliteration (Timofey - Timofiy).
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
