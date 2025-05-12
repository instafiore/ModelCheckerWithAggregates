# ModelCheckerWithAggregates

A novel propagator for unfounded sets for non-HFC components supporting recursive aggregates.


## Install

### **Dependencies**

Ensure you have the following dependencies installed:

1. **Conda** (version 23.7.2 or higher)
2. **g++**

### **Installation Steps**

1. **Create the Conda Environment:**
   ```bash
   conda create --name recursive_aggregates python=3.10
   ```

2. **Create the Conda Environment:**
   ```bash
   conda activate recursive_aggregates
   ```

3. **Create the Conda Environment:**
   ```bash
   conda config --add channels potassco
   conda env update --file environment.yaml
   ```


---

## Running one instance

### **For running a pair encoding* and instance :*
1. **Build propagator:**
   ```bash
   cd /prop_unf/propagator_c
   make clean
   make release 
   ```
2. **Run propagator:**
   ```bash
    python prop_unf/run.py -e=<encoding>* -i=<instance>
   ```
- \* is mandatory
3. **Help menu propagator:**
   ```bash
    python prop_unf/run.py -h
   ```
   
---
## Running Experiments

### **Step-by-Step Instructions**

1. **Clone the repository for `pyrunlim` and `pyrunner` :**
   ```bash
    git clone https://github.com/alviano/python.git
    ```

2. **Set the `PYRUNLIM` environment variable: Define the path to the `pyrunlim.py` script:**
   ```bash
    export PYRUNLIM=<pathtorepo>/python/pyrunlim/pyrunlim.py
    ```

3. **Set the `PYRUNNER` environment variable: Define the path to the `pyrunner.py` script:**
   ```bash
    export PYRUNNER=<pathtorepo>/python/pyrunner/pyrunner.py
    ```

4. **Set the `ROOT` directory of the propagator :**
   ```bash
    export ROOT=<path>/ModelCheckerWithAggregates
    ```

5. **Run experiments :**
   ```bash
   cd tests/expertiments
    ./run_experiments.sh experiments.py <name_experiements>
    ```

---

### Optional: Define CPU for Running Experiments

If you want to specify the CPU to use for running experiments, set the `CPU` environment variable:

```bash
export CPU=<CPU_INDEX>
```


## Notes

- Make sure to replace `<path_to_repo>` and `<path_to_ModelCheckerWithAggregates>` with the actual paths on your system.
- If you encounter any issues during setup, verify your dependencies and paths are correctly set.
