# Running a example in NEXT

To run a example in NEXT, you need to *cd* to an example path

You also need a python interpeter

For this, lets try the ColdCollapseGalaxy example

```bash
cd examples
cd ColdCollapseGalaxy
python coldcollapse.py
../../next coldcollapse.txt 8 0.25 0.2 vtu
```

Explaining the flags:

- ../../next = Go up 2 directories (into project root) and run executable

- coldcollapse.txt = The name of the specified "scenario" / "initial condition"

- 8 = Set openmp threads to 8, if your cpu has more, you can change it

- 0.25 = Timestep, how fast the simulation will run

- 0.2 = Caching interval, how fast NEXT would plot caching

- vtu = Caching format, there is currently vtk|vtu
