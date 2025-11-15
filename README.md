# CTSP-SEP: Software to Generate Time-Consistency Inequalities for the CTSP with Idle Times

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++](https://img.shields.io/badge/C++-14-blue.svg)](https://isocpp.org/)
[![CMake](https://img.shields.io/badge/CMake-3.10+-064F8C.svg)](https://cmake.org/)

**CTSP-SEP** is a software tool designed to demonstrate the core procedure for identifying temporal consistency constraints used in Riera-Ledesma et al. (2025) (Section 5). This procedure leverages the combinatorial structure underlying the dual problem associated with verifying whether a partial solution satisfies temporal consistency constraints. The software specifically implements the separation method for detecting violated cycles within partial solutions, which forms the foundation of path elimination constraints in branch-and-cut algorithms.

The tool accepts a **partial solution** as input (though the working example uses an integer solution, it need not be integral). When the solution is temporally feasible, the program generates a feasible schedule returned in JSON format. When temporal consistency constraints are violated, the program produces:

- A **graph in DOT format** identifying the arcs responsible for infeasibility
- A **text file enumerating violated cycles** that can be used to generate cutting planes

This implementation provides researchers with practical insight into the separation oracle described in Section 5 of our paper, demonstrating how dual-driven path elimination constraints are identified and strengthened in practice.

---

> **💡 Why This Repository?**
>
> This repository serves as a **practical implementation companion** to the research presented in:
>
> **Riera-Ledesma, J., Rodríguez-Martín, I., & Hernández-Pérez, H.** (2025)  
> *Dual-driven path elimination for vehicle routing with idle times and arrival-time consistency*  
> Computers & Operations Research, 107326  
> [https://doi.org/10.1016/j.cor.2025.107326](https://doi.org/10.1016/j.cor.2025.107326)
>
> **Purpose:**
>
> - **Demonstrate the separation oracle** described in Section 5 of our paper
> - **Identify violated cycles** in partial solutions through dual problem analysis
> - **Generate path elimination cuts** from infeasible (possibly fractional) solutions
> - **Visualize the combinatorial structure** of temporal consistency constraints
>
> While generating a schedule from a feasible solution could be done with simpler code, this tool reveals the **dual-driven separation mechanism** that powers our branch-and-cut approach. The graph and cycle outputs show researchers exactly how temporal consistency violations are detected and converted into valid cutting planes.

---

## Overview

**CTSP-SEP** is a research tool that implements the **separation oracle** for temporal consistency constraints in vehicle routing problems with arrival-time consistency. The tool analyzes partial solutions (fractional or integer) and:

### Core Functionality

1. **Constraint Verification via LP**: Solves the dual problem to check if a partial solution satisfies temporal consistency constraints
2. **Violated Cycle Detection**: Identifies cycles in the dual graph that violate consistency requirements
3. **Cut Generation**: Produces path elimination constraints from identified violations
4. **Infeasibility Visualization**: Exports DOT graphs showing the combinatorial structure of violations

### What This Project Provides

- **Separation oracle implementation** following the dual-driven approach in Riera-Ledesma et al. (2025), Section 5
- **Cycle enumeration** for generating valid inequalities in branch-and-cut frameworks
- **Visual debugging** through graph exports that reveal the structure of temporal conflicts
- **Working examples** with TSPLIB-extended instances demonstrating the separation process

### Key References

For problem definition and theoretical background, see:

1. **Riera-Ledesma, J., Rodríguez-Martín, I., & Hernández-Pérez, H.** (2025)  
   *Dual-driven path elimination for vehicle routing with idle times and arrival-time consistency*  
   Computers & Operations Research, 107326  
   [https://doi.org/10.1016/j.cor.2025.107326](https://doi.org/10.1016/j.cor.2025.107326)

2. **Subramanyam, A., & Gounaris, C. E.** (2016)  
   *A branch-and-cut framework for the consistent traveling salesman problem*  
   European Journal of Operational Research, 248(2), 384-395  
   [https://doi.org/10.1016/j.ejor.2015.07.030](https://doi.org/10.1016/j.ejor.2015.07.030)

3. **Subramanyam, A., & Gounaris, C. E.** (2018)  
   *A decomposition algorithm for the consistent traveling salesman problem with vehicle idling*  
   Transportation Science, 52(2), 386-401  
   [https://doi.org/10.1287/trsc.2017.0741](https://doi.org/10.1287/trsc.2017.0741)

## Features

✅ **Arrival-time consistency** - Verify and enforce temporal consistency across routes
✅ **TSPLIB compatibility** - Read standard TSPLIB instances with CTSP extensions
✅ **JSON export** - Structured output for integration with other tools
✅ **Comprehensive documentation** - Doxygen comments and detailed READMEs for all modules
✅ **Modular architecture** - Clean separation between I/O, modeling, solving, and scheduling


## Project Structure

```text
CTSP_scheduler/
├── .vscode/
│   ├── launch.json             # Debug configurations (cplex/clp variants)
│   └── tasks.json              # Build tasks (configure/build/rebuild)
│
├── docs/
│   └── INSTALL.md              # Installation guide (CLP/CPLEX setup)
│
├── src/
│   ├── main/                   # Separation oracle driver
│   │   ├── include/            # CLI and I/O headers
│   │   ├── src/                # main.cpp, solution parsing
│   │   └── README.md           # Usage guide
│   │
│   ├── CTSP/
│   │   ├── IO/                 # TSPLIB instance parser
│   │   │   └── README.md       # Instance format specification
│   │   └── interface/          # Model builder interface
│   │       └── README.md       # API documentation
│   │
│   ├── sync_lib/               # Temporal consistency separation library
│   │   ├── sync_IO/            # Solution representation & model I/O
│   │   │   └── README.md       # Data structures for solutions and models
│   │   │
│   │   ├── sync_checker/       # Dual LP formulation & solving
│   │   │   └── README.md       # Constraint verification via dual problem
│   │   │
│   │   ├── sync_checker_solver/ # LP solver abstraction layer
│   │   │   └── README.md       # Generic solver interface
│   │   │
│   │   ├── sync_path_finder/   # Violated cycle detection (separation oracle)
│   │   │   └── README.md       # Algorithm description and usage
│   │   │
│   │   └── sync_verify/        # Schedule generation (feasible solutions)
│   │       └── README.md       # Scheduling algorithm
│   │
│   └── util/                   # Core utilities
│       ├── include/
│       │   ├── LP_solver.hpp          # Abstract solver interface
│       │   ├── model_description.hpp  # Solver-agnostic LP model
│       │   ├── graph.hpp              # Graph utilities (DOT export)
│       │   ├── matrix.hpp             # Matrix operations
│       │   ├── CLP/                   # CLP backend implementation
│       │   └── CPX/                   # CPLEX backend implementation
│       └── README.md           # Utility documentation
│
├── input/                      # Example instances & solutions
│   ├── *.contsp                # CTSP instance files (TSPLIB extended)
│   ├── *.sol                   # Routing solutions (feasible)
│   └── *.infeas.sol            # Infeasible solutions (for cut demo)
│
├── output/                     # Generated outputs
│   ├── *.sched.json            # Feasible schedules (JSON format)
│   ├── *.graph.dot             # Violation graphs (DOT format)
│   └── *.infeasible_paths.txt  # Enumerated violated cycles
│
├── build-cplex/                # CPLEX build artifacts (generated)
├── build-clp/                  # CLP build artifacts (generated)
│
├── CMakeLists.txt              # Root CMake configuration
├── CMakePresets.json           # Build presets (cplex/clp)
└── README.md                   # This file
```

## Quick Start

### Prerequisites

- **CMake** 3.10 or higher
- **C++ compiler** with C++14 support (GCC 5+, Clang 3.4+, MSVC 2015+)
- **LP Solver** (choose one):
  - **COIN-OR CLP** (open-source, recommended for academic use)
  - **IBM ILOG CPLEX 22.1** (commercial, requires license)
- **Graphviz** (optional, for visualizing DOT graphs generated from infeasible solutions)

### Build and Run (CMake Presets)

#### Option A: With CLP (Open-Source)

```bash
# Clone the repository
git clone https://github.com/RieraULL/CTSP_scheduler.git
cd CTSP_scheduler

# Configure (CLP build)
cmake --preset clp

# Build (CLP)
cmake --build --preset clp -j

# Run with feasible solution (generates schedule JSON)
./build-clp/bin/ctsp_scheduler ctsp2 input/burma14_p3_f50_lH.contsp input/burma14_p3_f50_lH.sol output/

# Run with infeasible solution (generates DOT graph + violated cycles)
./build-clp/bin/ctsp_scheduler ctsp2 input/burma14_p3_f50_lH.contsp input/burma14_p3_f50_lH.infeas.sol output/
```

#### Option B: With CPLEX (Commercial)

```bash
# Clone the repository
git clone https://github.com/RieraULL/CTSP_scheduler.git
cd CTSP_scheduler

# Configure (CPLEX build)
cmake --preset cplex

# Build (CPLEX)
cmake --build --preset cplex -j

# Run with feasible solution (generates schedule JSON)
./build-cplex/bin/ctsp_scheduler ctsp2 input/burma14_p3_f50_lH.contsp input/burma14_p3_f50_lH.sol output/

# Run with infeasible solution (generates DOT graph + violated cycles)
./build-cplex/bin/ctsp_scheduler ctsp2 input/burma14_p3_f50_lH.contsp input/burma14_p3_f50_lH.infeas.sol output/
```

**Output:**

- **Feasible solution**: JSON file with temporal schedules and time windows (`output/burma14_p3_f50_lH.sched.json`)
- **Infeasible solution**: DOT graph (`output/burma14_p3_f50_lH.graph.dot`) + violated cycles list (`output/burma14_p3_f50_lH.infeasible_paths.txt`)

> **Note**: For details on installing dependencies (CLP or CPLEX), see [docs/INSTALL.md](docs/INSTALL.md).

## Installation

See the complete installation guide at: [docs/INSTALL.md](docs/INSTALL.md)

## Debugging in VS Code (optional)

We include debug configurations in `.vscode/launch.json` to run `ctsp_scheduler` with example arguments. There are variants for executables in `build-cplex` and `build-clp`.

## Usage

### Command Line Interface

```bash
./ctsp_scheduler <problem_type> <instance_file> <solution_file> <output_dir>
```

**Arguments:**

- `problem_type`: Problem variant (`"ctsp2"` for multi-depot, `"ctsp1"` for single-depot)
- `instance_file`: Path to CTSP instance in extended TSPLIB format (`.contsp`)
- `solution_file`: Path to routing solution (`.sol`) — can be feasible or infeasible
- `output_dir`: Output directory where results will be written

### CLI Examples

```bash
# Feasible solution → generates schedule JSON under output/
# Using CLP:
./build-clp/bin/ctsp_scheduler ctsp2 input/burma14_p3_f50_lH.contsp input/burma14_p3_f50_lH.sol output/
# Using CPLEX:
./build-cplex/bin/ctsp_scheduler ctsp2 input/burma14_p3_f50_lH.contsp input/burma14_p3_f50_lH.sol output/

# Infeasible solution → generates DOT graph + violated cycles under output/
# Using CLP:
./build-clp/bin/ctsp_scheduler ctsp2 input/burma14_p3_f50_lH.contsp input/burma14_p3_f50_lH.infeas.sol output/
# Using CPLEX:
./build-cplex/bin/ctsp_scheduler ctsp2 input/burma14_p3_f50_lH.contsp input/burma14_p3_f50_lH.infeas.sol output/
```

## Architecture

### Component Overview

```text
┌─────────────────────────────────────────────────────────────┐
│                 main (separation driver)                     │
│  CLI, I/O, orchestration of separation/scheduling           │
└───────────────┬───────────────────────────────┬─────────────┘
      │                               │
   ┌───────▼───────┐                 ┌──────▼──────┐
   │   CTSP/IO     │                 │    util     │
   │ TSPLIB parser │                 │ graph/matrix│
   └───────┬───────┘                 └──────┬──────┘
      │                                   │
   ┌───────▼───────────────────────────────────▼────────┐
   │                     sync_lib                        │
   │  sync_IO | sync_checker | sync_checker_solver       │
   │  sync_path_finder (separation oracle) | sync_verify │
   └─────────────────────────────────────────────────────┘
```

### Data Flow

```text
1. Load instance (.contsp) and solution (.sol)
          ↓
2. Build synchronization model and solve LP (dual check)
          ↓
3. Update support graph from active arcs (α, γ)
          ↓
4. Detect violated cycles via DFS (path_finder)
          ↓
5a. If violations → export DOT graph + violated cycles list
5b. Else → generate schedule JSON
          ↓
6. Write files under output/
```

### Support Graph and Cycle Closure

The separation oracle operates on a support graph built from the LP solution:

- Vertices: depots + operations
- Edges: routing arcs with alpha > tol; sync arcs with gamma > tol
- For each active sync arc (i, j), enumerate simple paths i → j and close each path with the reverse sync arc (j, i) to form a cycle.

```text
Active arcs from LP
   (routing: α>tol, sync: γ>tol)
        
   i  →  v1  →  v2  →  …  →  j     (simple path i→j)
    \______________________________/
                 |
                 |  close cycle with reverse sync arc
                 v
                (j → i)

Cycle (arcs) = [routing..., sync (j→i)]
```

Arc indexing in cycles:

- Routing arcs: indices [0, n_routing_arcs)
- Sync arcs: indices [n_routing_arcs, n_routing_arcs + n_sync_arcs)

### Key Abstractions

- **`SYNC_LIB::path_finder`**: DFS-based cycle detector (separation oracle)
- **`sync_checker`**: Dual LP-based constraint verification
- **`sync_checker_solver`**: Solver adapter (CLP/CPLEX)
- **`sync_model_builder`**: Model construction and arc indexing/maps
- **`GOMA::search_graph`**: Support graph and path enumeration utilities
- **`LP_solver`**: Abstract solver interface
- **`conTSP2_scheduling`**: Feasible schedule generator
- **`CTSP::instance`**: CTSP problem representation
- **`sync_solution`**: Routing solution representation

## Dependencies

### Required

- **C++ Standard Library** (C++14)
- **CMake** 3.10+ (build system)

## Documentation

See also: [Support Graph and Cycle Closure](#support-graph-and-cycle-closure).

### Module Documentation

Each major component has detailed documentation:

| Module | Description | Documentation |
|--------|-------------|---------------|
| **main** | Separation driver & CLI | [src/main/README.md](src/main/README.md) |
| **CTSP/IO** | Instance file parsing | [src/CTSP/IO/README.md](src/CTSP/IO/README.md) |
| **CTSP/interface** | Model builder API | [src/CTSP/interface/README.md](src/CTSP/interface/README.md) |
| **sync_IO** | Solution representation | [src/sync_lib/sync_IO/README.md](src/sync_lib/sync_IO/README.md) |
| **sync_checker** | Dual LP constraint verification | [src/sync_lib/sync_checker/README.md](src/sync_lib/sync_checker/README.md) |
| **sync_checker_solver** | LP solver wrapper (CLP/CPLEX) | [src/sync_lib/sync_checker_solver/README.md](src/sync_lib/sync_checker_solver/README.md) |
| **sync_path_finder** | Violated cycle detection (separation oracle) | [src/sync_lib/sync_path_finder/README.md](src/sync_lib/sync_path_finder/README.md) |
| **sync_verify** | Schedule generation | [src/sync_lib/sync_verify/README.md](src/sync_lib/sync_verify/README.md) |
| **util** | Matrix, graph & LP utilities | [src/util/README.md](src/util/README.md) |

### API Documentation (Doxygen)

All headers include comprehensive Doxygen comments. Generate HTML documentation:

```bash
# Install Doxygen
sudo apt-get install doxygen graphviz  # Debian/Ubuntu

# Generate documentation
doxygen Doxyfile

# Open in browser
firefox docs/html/index.html
```

## Examples

### Feasible Solution

```bash
# Using CLP build
./build-clp/bin/ctsp_scheduler ctsp2 \
   input/burma14_p3_f50_lH.contsp \
   input/burma14_p3_f50_lH.sol \
   output/

# Using CPLEX build
./build-cplex/bin/ctsp_scheduler ctsp2 \
   input/burma14_p3_f50_lH.contsp \
   input/burma14_p3_f50_lH.sol \
   output/
```

Outputs (under `output/`):

- `burma14_p3_f50_lH.sched.json` — Temporal schedules and time windows


### Infeasible Solution

```bash
# Using CLP build
./build-clp/bin/ctsp_scheduler ctsp2 \
   input/burma14_p3_f50_lH.contsp \
   input/burma14_p3_f50_lH.infeas.sol \
   output/

# Using CPLEX build
./build-cplex/bin/ctsp_scheduler ctsp2 \
   input/burma14_p3_f50_lH.contsp \
   input/burma14_p3_f50_lH.infeas.sol \
   output/
```

Outputs (under `output/`):

- `burma14_p3_f50_lH.graph.dot` — Violation graph (DOT)
- `burma14_p3_f50_lH.infeasible_paths.txt` — Enumerated violated cycles


Optional visualization with Graphviz:

```bash
dot -Tpdf output/burma14_p3_f50_lH.graph.dot -o output/burma14_p3_f50_lH.graph.pdf
```

## License

This project is licensed under the **MIT License** - see the [LICENSE](LICENSE) file for details.

```text
MIT License

Copyright (c) 2025 Jorge Riera-Ledesma

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction...
```

## Authors

### Main Author

Jorge Riera-Ledesma

- GitHub: [@RieraULL](https://github.com/RieraULL)
- Affiliation: University of La Laguna

## Citation

If you use this software in your research, please cite:

```bibtex
@software{ctsp_sep2025,
  author = {Riera-Ledesma, Jorge},
  title = {CTSP-SEP: Software to Generate Time-Consistency Inequalities for the CTSP with Idle Times},
  version = {2.0.0},
  year = {2025},
  publisher = {Zenodo},
  doi = {10.5281/zenodo.17616597},
  url = {https://doi.org/10.5281/zenodo.17616597}
}
```

## Acknowledgments

- TSPLIB format by Gerhard Reinelt
- COIN-OR project for CLP solver
- IBM ILOG for CPLEX academic licenses
- University of La Laguna for research support

## Related Work

### Key Research Papers

1. **Riera-Ledesma, J., Rodríguez-Martín, I., & Hernández-Pérez, H.** (2025)  
   *Dual-driven path elimination for vehicle routing with idle times and arrival-time consistency*  
   Computers & Operations Research, 107326

2. **Subramanyam, A., & Gounaris, C. E.** (2016)  
   *A branch-and-cut framework for the consistent traveling salesman problem*  
   European Journal of Operational Research, 248(2), 384-395

3. **Subramanyam, A., & Gounaris, C. E.** (2018)  
   *A decomposition algorithm for the consistent traveling salesman problem with vehicle idling*  
   Transportation Science, 52(2), 386-401

## Contact

For questions, issues, or collaboration:

- **Issues**: [GitHub Issues](https://github.com/RieraULL/CTSP_scheduler/issues)
- **Email**: Contact through GitHub profile
- **Discussions**: Use GitHub Discussions for general questions

---

**Last Updated:** November 2025  
**Version:** 2.0.0  
**Status:** Active Development
