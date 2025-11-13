# Installation Guide

This guide covers installation and compilation of the project with two solver alternatives:

- CLP (COIN-OR) – open-source, recommended for LP
- CPLEX (IBM ILOG) – commercial, recommended if you already use it

Both options work through CMake presets.

## Common Requirements

- CMake 3.10+
- C++ compiler with C++14 support
- Git (optional, to clone the repo)

Clone the repository:

```bash
git clone https://github.com/RieraULL/CTSP_scheduler.git
cd CTSP_scheduler
```

> **Important**: If you copy or move the project directory manually (not via git clone), make sure to delete any existing `build-*` directories first, as they contain CMake cache files with absolute paths that will cause configuration errors.

---

## Option A: CLP (Open-Source)

The project can build with CLP without requiring `ClpConfig.cmake` thanks to an automatic discovery mechanism for system headers and libraries.

### 1) Install dependencies (Debian/Ubuntu)

```bash
sudo apt-get update
sudo apt-get install -y \
  coinor-libclp-dev coinor-libcoinutils-dev coinor-libosi-dev \
  pkgconf
```

Quick verification (optional):

```bash
ls /usr/include/coin | grep -E "^Clp.*hpp$" | head
ls /usr/lib/x86_64-linux-gnu/libClp.so
```

### 2) Configure and build (clp preset)

```bash
cmake --preset clp
cmake --build --preset clp -j
```

### 3) Run (CLP)

```bash
./build-clp/bin/ctsp_scheduler ctsp2 \
  input/bayg29_p5_f90_lL.contsp \
  input/bayg29_p5_f90_lL.sol \
  output/bayg29_p5_f90_lL.sched.json
```

### Notes and troubleshooting

- If CMake doesn't find `ClpConfig.cmake`, the project automatically falls back to searching for headers in `/usr/include/coin` and libraries in `/usr/lib/x86_64-linux-gnu`.
- CLP is LP-only. If your model has integer variables, the LP relaxation will be solved. For MIP, integrate CBC (COIN-OR Branch-and-Cut).
- Custom installation (if you installed CLP outside standard paths):

```bash
# If you have ClpConfig.cmake
cmake --preset clp -DClp_DIR=/usr/local/lib/cmake/Clp

# Or add the prefix
cmake --preset clp -DCMAKE_PREFIX_PATH=/usr/local
```

---

## Option B: CPLEX (Commercial)

Make sure you have IBM ILOG CPLEX installed locally. Then point the `CPX_PATH` variable in `src/util/CMakeLists.txt` to your installation.

### 1) Adjust CPLEX path

Edit `src/util/CMakeLists.txt` and change:

```cmake
set (CPX_PATH /path/to/your/CPLEX_Studio221)
```

### 2) Configure and build (cplex preset)

```bash
cmake --preset cplex
cmake --build --preset cplex -j
```

### 3) Run (CPLEX)

```bash
./build-cplex/bin/ctsp_scheduler ctsp2 \
  input/bayg29_p5_f90_lL.contsp \
  input/bayg29_p5_f90_lL.sol \
  output/bayg29_p5_f90_lL.sched.json
```

---

## Debugging in VS Code (optional)

We include configurations in `.vscode/launch.json` to run `ctsp_scheduler` with example arguments. There are variants for `build-cplex` and `build-clp`.

---

## Tips

- If you switch solvers (CLP/CPLEX), reconfigure with the corresponding preset.
- If you see link errors with CLP, confirm the presence of `libClp.so`, `libClpSolver.so`, `libCoinUtils.so`, `libOsi.so`, `libOsiClp.so` in `/usr/lib/x86_64-linux-gnu/`.
- For code documentation (Doxygen), see the corresponding section in `README.md`.

### Troubleshooting: "source does not match the source used to generate cache"

If you get this CMake error after copying or moving the project:

```text
CMake Error: The source ".../CMakeLists.txt" does not match the source
"..." used to generate cache. Re-run cmake with a different source directory.
```

**Cause**: CMake cache files (`CMakeCache.txt`) in `build-*` directories contain absolute paths from the original location.

**Solution**: Delete all build directories before reconfiguring:

```bash
rm -rf build-clp build-cplex
cmake --preset clp  # or cplex
```

The build directories are excluded from git via `.gitignore`, so they won't be included if you clone fresh from the repository.
