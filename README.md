# CTSP Scheduler

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++](https://img.shields.io/badge/C++-14-blue.svg)](https://isocpp.org/)
[![CMake](https://img.shields.io/badge/CMake-3.10+-064F8C.svg)](https://cmake.org/)

A comprehensive C++ framework to verify a feasible solution and to provide with a scheduling for the **Consistent Traveling Salesman Problems (CTSP)** (arrival time consistency). This project converts CTSP routing solutions into temporal schedules, ensuring that all consistency constraints between multiple depots are satisfied.

## Overview

The **CTSP Scheduler** is a specialized tool designed to verify a solution for the Consistent Traveling Salesman Problem (CTSP), a variant of the classical TSP where:

- A depot serves a set of customers over multiple days
- Customers must be visited at **consistent times** across days (arrival-time consistency constraints)
- Each customer has specific service duration requirements
- The goal is to generate feasible temporal schedules that satisfy all constraints

### Problem Definition and References

The Consistent Traveling Salesman Problem is defined in the following research papers:

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



### What This Project Provides

1. **Instance management**: Parse TSPLIB-extended format with CTSP-specific constraints
2. **Constraint verification**: Validate arrival-time consistency constraints via Linear Programming
3. **Schedule generation**: Convert routing solutions into temporal schedules with time windows
4. **JSON output**: Export schedules in a format ready for visualization and execution

## Features

✅ **Arrival-time consistency** - Verify and enforce temporal consistency across routes
✅ **TSPLIB compatibility** - Read standard TSPLIB instances with CTSP extensions
✅ **JSON export** - Structured output for integration with other tools
✅ **Comprehensive documentation** - Doxygen comments and detailed READMEs for all modules
✅ **Modular architecture** - Clean separation between I/O, modeling, solving, and scheduling


## Project Structure

```text
CTSP_scheduler/
├── src/
│   ├── main/                   # Main scheduler application
│   │   ├── include/            # Headers (sch_io.hpp, schedulers.hpp)
│   │   ├── src/                # Implementation (main.cpp, schedulers.cpp)
│   │   └── README.md           # Detailed usage guide
│   │
│   ├── CTSP/                   # CTSP-specific components
│   │   ├── IO/                 # Instance file parsing (TSPLIB extended)
│   │   │   └── README.md       # TSPLIB format specification
│   │   └── interface/          # Model builder adapters
│   │       └── README.md       # CTSP model API
│   │
│   ├── sync_lib/               # Synchronization constraint library
│   │   ├── sync_IO/            # Model I/O and solution representation
│   │   │   └── README.md       # Data structures documentation
│   │   ├── sync_checker/       # Constraint verification via LP
│   │   │   └── README.md       # LP formulation details
│   │   ├── sync_checker_solver/ # LP solver wrapper
│   │   │   └── README.md       # Solver interface + CLP migration guide
│   │   └── sync_verify/        # Solution-to-schedule converter
│   │       └── README.md       # Scheduling algorithm
│   │
│   └── util/                   # Utilities (matrix, LP solver interface)
│       └── README.md           # Matrix class + CPLEX/CLP guide
│
├── input/                      # Example instance and solution files
├── output/                     # Generated schedule files
├── build/                      # Build directory (generated)
├── CMakeLists.txt             # Root CMake configuration
└── README.md                  # This file
```

## Quick Start

### Prerequisites

- **CMake** 3.10 or higher
- **C++ compiler** with C++14 support (GCC 5+, Clang 3.4+, MSVC 2015+)
- **LP Solver**: IBM ILOG CPLEX 22.1 (or CLP for open-source builds if you adapt the code)

### Build and Run (CMake Presets)

**Opción A: Con CLP (Open-Source)**

```bash
# Clone the repository
git clone https://github.com/RieraULL/CTSP_scheduler.git
cd CTSP_scheduler

# Configure (CLP build)
cmake --preset clp

# Build (CLP)
cmake --build --preset clp -j

# Run (CLP)
./build-clp/bin/ctsp_scheduler ctsp2 input/bayg29_p5_f90_lL.contsp input/bayg29_p5_f90_lL.sol output/schedule.json
```

**Opción B: Con CPLEX (Comercial)**

```bash
# Clone the repository
git clone https://github.com/RieraULL/CTSP_scheduler.git
cd CTSP_scheduler

# Configure (CPLEX build)
cmake --preset cplex

# Build (CPLEX)
cmake --build --preset cplex -j

# Run (CPLEX)
./build-cplex/bin/ctsp_scheduler ctsp2 input/bayg29_p5_f90_lL.contsp input/bayg29_p5_f90_lL.sol output/schedule.json
```

**Output:** A JSON file with temporal schedules for each depot and time windows for each customer.

> **Nota**: Para detalles de instalación de dependencias (CLP o CPLEX), consulta [docs/INSTALL.md](docs/INSTALL.md).

## Instalación

Consulta la guía completa de instalación en: [docs/INSTALL.md](docs/INSTALL.md)

## Depuración en VS Code (opcional)

Incluimos configuraciones de depuración en `.vscode/launch.json` para ejecutar `ctsp_scheduler` con los mismos argumentos de ejemplo. Hay variantes para ejecutables en `build-cplex` y `build-clp`.

## Usage

### Command Line Interface

```bash
./ctsp_scheduler <problem_type> <instance_file> <solution_file> <output_file>
```

**Arguments:**

- `problem_type`: Problem variant (`"ctsp2"` for multi-depot, `"ctsp1"` for single-depot) (**this version only supports ctsp1**)
- `instance_file`: Path to CTSP instance in extended TSPLIB format (`.contsp`)
- `solution_file`: Path to feasible routing solution (`.sol`)
- `output_file`: Path for output schedule file (`.sched.json`)

### Example

```bash
./ctsp_scheduler ctsp2 instances/bayg29_p5.contsp solutions/bayg29_p5.sol output/schedule.json
```

## Architecture

### Component Overview

```text
┌─────────────────────────────────────────────────────────────┐
│                    main (ctsp_scheduler)                     │
│  Entry point, command-line parsing, workflow orchestration  │
└──────────────────────┬──────────────────────────────────────┘
                       │
        ┌──────────────┼──────────────┬─────────────┐
        │              │              │             │
        ▼              ▼              ▼             ▼
┌──────────────┐ ┌──────────┐ ┌──────────────┐ ┌─────────┐
│  CTSP/IO     │ │ CTSP/    │ │  sync_lib    │ │  util   │
│              │ │ interface│ │              │ │         │
│ TSPLIB       │ │          │ │ sync_IO      │ │ matrix  │
│ parser       │ │ Model    │ │ sync_checker │ │ LP      │
│ Instance     │ │ builder  │ │ sync_verify  │ │ solver  │
│ management   │ │ adapter  │ │ sync_checker_│ │         │
│              │ │          │ │   _solver    │ │         │
└──────────────┘ └──────────┘ └──────────────┘ └─────────┘
```

### Data Flow

```text
1. Load instance (.contsp)
          ↓
2. Load solution (.sol)
          ↓
3. Build synchronization model
          ↓
4. Convert solution format
          ↓
5. Verify constraints via LP
          ↓
6. Compute schedules & time windows
          ↓
7. Write JSON output (.sched.json)
```

### Key Abstractions

- **`CTSP::instance`**: In-memory representation of CTSP problem
- **`sync_solution`**: Routing solution with depot assignments and routes
- **`sync_model_builder`**: Abstract interface for constraint model construction
- **`sync_checker`**: LP-based constraint verification
- **`conTSP2_scheduling`**: Schedule generator from feasible solutions
- **`LP_solver`**: Abstract interface for linear programming solvers

## Dependencies

### Required

- **C++ Standard Library** (C++14)
- **CMake** 3.10+ (build system)

## Documentation

### Module Documentation

Each major component has detailed documentation:

| Module | Description | Documentation |
|--------|-------------|---------------|
| **main** | Scheduler application | [src/main/README.md](src/main/README.md) |
| **CTSP/IO** | Instance file parsing | [src/CTSP/IO/README.md](src/CTSP/IO/README.md) |
| **CTSP/interface** | Model builder API | [src/CTSP/interface/README.md](src/CTSP/interface/README.md) |
| **sync_IO** | Solution representation | [src/sync_lib/sync_IO/README.md](src/sync_lib/sync_IO/README.md) |
| **sync_checker** | Constraint verification | [src/sync_lib/sync_checker/README.md](src/sync_lib/sync_checker/README.md) |
| **sync_checker_solver** | LP solver wrapper | [src/sync_lib/sync_checker_solver/README.md](src/sync_lib/sync_checker_solver/README.md) |
| **sync_verify** | Schedule generation | [src/sync_lib/sync_verify/README.md](src/sync_lib/sync_verify/README.md) |
| **util** | Matrix & LP utilities | [src/util/README.md](src/util/README.md) |

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

### Example 1: Basic Usage

```bash
# Run on example instance
./bin/ctsp_scheduler ctsp2 \
  input/bayg29_p5_f90_lL.contsp \
  input/bayg29_p5_f90_lL.sol \
  output/schedule.json
```

**Input:**

- Instance: 29 nodes (27 customers + 2 depots), 5 days
- Max time differential: 156 time units
- Solution: Pre-computed feasible routes

**Output:**

- JSON file with schedules for both depots
- Time windows for all 27 customers
- Verification that arrival-time consistency constraints are satisfied

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
@software{ctsp_scheduler2025,
  author = {Riera-Ledesma, Jorge},
  title = {CTSP Scheduler: A Framework for Consistent TSP Scheduling},
  year = {2025},
  url = {https://github.com/RieraULL/CTSP_scheduler}
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
**Version:** 1.0.0  
**Status:** Active Development
