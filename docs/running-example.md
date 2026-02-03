# Running an Example in NEXT

To run an example in **NEXT**, first navigate to the example’s directory.  
You will also need a Python interpreter installed.

As a demonstration, let’s try the **ColdCollapseGalaxy** example:
```bash
    cd examples
    cd ColdCollapseGalaxy
    python coldcollapse.py
    ../../next coldcollapse.txt 8 0.25 0.2 vtu
```
### Explanation of the command

- `../../next` → Move up two directories (to the project root) and run the NEXT executable  
- `coldcollapse.txt` → The scenario / initial condition file  
- `8` → Number of OpenMP (CPU) threads; adjust based on your CPU  
- `0.25` → Timestep, controls how fast the simulation advances  
- `0.2` → Dump interval, controls how often NEXT writes output  
- `vtu` → Output format; options are `vtk` or `vtu`  

Now you can enjoy the simulation.  
To exit, press **Ctrl+C** or type **q** (then Enter).
