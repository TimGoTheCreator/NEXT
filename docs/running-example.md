# Running Simulations in NEXT

To execute a simulation scenario, first navigate to the desired example directory in `examples/`.

---

## Example Execution: ColdCollapseGalaxy

```bash
cd examples/ColdCollapseGalaxy
python coldcollapse.py
../../next coldcollapse.txt 8 0.01 0.1 vtu 100
```

---

## Command Line Arguments Explained

```bash
next <input.txt> <threads> <dt> <dump_interval> <vtk|vtu|hdf5> [max_steps]
```

1. **`input.txt`** - Path to initial particle state file (positions, velocities, masses).
2. **`threads`** - Number of OpenMP worker threads for parallel computation.
3. **`dt`** - Baseline simulation time step (dt).
4. **`dump_interval`** - Simulation time interval between data output dumps.
5. **`format`** - Output data format (`vtu`, `vtk`, or `hdf5`).
6. **`[max_steps]`** - *(Optional)* Maximum step count before exiting automatically.

---

## Interactive Termination

During execution, press **`q`** and hit Enter in the terminal to request a graceful exit.
