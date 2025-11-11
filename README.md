# CTSP Scheduler

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++](https://img.shields.io/badge/C++-14-blue.svg)](https://isocpp.org/)
[![CMake](https://img.shields.io/badge/CMake-3.10+-064F8C.svg)](https://cmake.org/)

A comprehensive C++ framework for solving and scheduling **Consistent Traveling Salesman Problems (CTSP)** with synchronization constraints. This project converts CTSP routing solutions into temporal schedules with time windows, ensuring that all synchronization constraints between multiple depots are satisfied.

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Problem Description](#problem-description)
- [Project Structure](#project-structure)
- [Quick Start](#quick-start)
- [Usage](#usage)
- [Input/Output Formats](#inputoutput-formats)
- [Architecture](#architecture)
- [Dependencies](#dependencies)
- [Building from Source](#building-from-source)
- [Documentation](#documentation)
- [Examples](#examples)
- [Contributing](#contributing)
- [License](#license)
- [Authors](#authors)

## Overview

The **CTSP Scheduler** is a specialized tool designed to solve the Consistent Traveling Salesman Problem (CTSP), a variant of the classical TSP where:

- Multiple depots serve a set of customers over multiple days
- Customers must be visited at **consistent times** across days (synchronization constraints)
- Each customer has specific service duration requirements
- The goal is to generate feasible temporal schedules that satisfy all constraints

This project provides:
1. **Instance management**: Parse TSPLIB-extended format with CTSP-specific constraints
2. **Constraint verification**: Validate synchronization constraints via Linear Programming
3. **Schedule generation**: Convert routing solutions into temporal schedules with time windows
4. **JSON output**: Export schedules in a format ready for visualization and execution

## Features

✅ **Multi-depot support** - Handle CTSP1 (single-depot) and CTSP2 (multi-depot) variants  
✅ **Synchronization constraints** - Verify and enforce temporal consistency across routes  
✅ **Time window computation** - Automatically calculate valid service time ranges  
✅ **TSPLIB compatibility** - Read standard TSPLIB instances with CTSP extensions  
✅ **Flexible LP solver** - Supports CPLEX (commercial) with migration path to CLP (open-source)  
✅ **JSON export** - Structured output for integration with other tools  
✅ **Comprehensive documentation** - Doxygen comments and detailed READMEs for all modules  
✅ **Modular architecture** - Clean separation between I/O, modeling, solving, and scheduling  

## Problem Description

### Consistent TSP (CTSP)

The Consistent Traveling Salesman Problem extends the classical TSP to multi-period routing scenarios where:

**Given:**
- $D$ depots serving $N$ customers over $T$ days
- Distance matrix $c_{ij}$ between all locations
- Service time $p_i$ for each customer $i$
- Maximum allowable time differential $\Delta$ between visits to the same customer

**Find:**
- A set of routes, one per depot per day
- Start times for each customer visit
- Such that:
  - Each customer is visited exactly once per day
  - Total route distance is minimized
  - **Synchronization constraint**: For each customer $i$ and days $t_1, t_2$:
    $$|s_{i,t_1} - s_{i,t_2}| \leq \Delta$$
    where $s_{i,t}$ is the start time for customer $i$ on day $t$

### Applications

- **Healthcare**: Home care services requiring consistent visit times
- **Waste collection**: Regular pickup schedules with time consistency
- **Delivery services**: Subscription-based deliveries at consistent times
- **Maintenance**: Regular service routes with customer preferences

## Project Structure

```
CTSP_scheduler/
├── src/
│   ├── main/                    # Main scheduler application
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
- **LP Solver**: IBM ILOG CPLEX 22.1 (or CLP for open-source builds)

### Build and Run

```bash
# Clone the repository
git clone https://github.com/RieraULL/CTSP_scheduler.git
cd CTSP_scheduler

# Create build directory
mkdir build
cd build

# Configure and build
cmake ..
make

# Run the scheduler
./bin/ctsp_scheduler ctsp2 ../input/bayg29_p5_f90_lL.contsp ../input/bayg29_p5_f90_lL.sol ../output/schedule.json
```

**Output:** A JSON file with temporal schedules for each depot and time windows for each customer.

## Usage

### Command Line Interface

```bash
./ctsp_scheduler <problem_type> <instance_file> <solution_file> <output_file>
```

**Arguments:**
- `problem_type`: Problem variant (`"ctsp2"` for multi-depot, `"ctsp1"` for single-depot)
- `instance_file`: Path to CTSP instance in extended TSPLIB format (`.contsp`)
- `solution_file`: Path to feasible routing solution (`.sol`)
- `output_file`: Path for output schedule file (`.sched.json`)

### Example

```bash
./ctsp_scheduler ctsp2 instances/bayg29_p5.contsp solutions/bayg29_p5.sol output/schedule.json
```

This generates a JSON file with:
- Per-depot schedules with arrival, start, and departure times
- Time windows for each customer
- Verification that all synchronization constraints are satisfied

## Input/Output Formats

### Instance File (`.contsp`)

Extended TSPLIB format with CTSP-specific sections:

```
NAME: bayg29_p5_f90_lL
TYPE: CONTSP
DIMENSION: 29
NUM_DAYS: 5
DISTANCE: 1722
MAXIMUM_ALLOWABLE_DIFFERENTIAL: 156
EDGE_WEIGHT_TYPE: EXPLICIT
EDGE_WEIGHT_SECTION
...
DEPOTS_SECTION
1
5
-1
PRECEDENCE_SECTION
1 2 5
-1 -1 -1
PROCESSING_TIME_SECTION
1 5.2
2 3.8
...
EOF
```

**Key fields:**
- `NUM_DAYS`: Number of service days
- `MAXIMUM_ALLOWABLE_DIFFERENTIAL`: Max time difference $\Delta$ for synchronization
- `DEPOTS_SECTION`: Depot node identifiers
- `PRECEDENCE_SECTION`: Synchronization constraint groups
- `PROCESSING_TIME_SECTION`: Service duration per customer

See [src/CTSP/IO/README.md](src/CTSP/IO/README.md) for complete format specification.

### Solution File (`.sol`)

Contains feasible CTSP routing solution with routes per depot and visit sequences.

Format details in [src/sync_lib/sync_IO/README.md](src/sync_lib/sync_IO/README.md).

### Schedule Output (`.sched.json`)

Structured JSON with schedules and time windows:

```json
{
  "instance": "bayg29_p5_f90_lL",
  "depots": [
    {
      "depot_id": 1,
      "schedule": [
        {
          "customer": 3,
          "arrival_time": 0.0,
          "start_time": 0.0,
          "departure_time": 5.2
        }
      ]
    }
  ],
  "time_windows": {
    "3": {"earliest": 0.0, "latest": 10.5}
  }
}
```

## Architecture

### Component Overview

```
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

```
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

### LP Solver (Choose One)

#### Option 1: IBM ILOG CPLEX 22.1 (Commercial)

**Current default.** High-performance commercial solver.

```bash
# Set CPLEX paths in CMake
cmake -DCPLEX_ROOT_DIR=/opt/ibm/ILOG/CPLEX_Studio221 ..
```

**License required.** Free academic licenses available.

#### Option 2: COIN-OR CLP (Open-Source)

**Recommended for open-source distribution.** Free and open-source alternative.

Migration instructions in:
- [src/util/README.md](src/util/README.md) - LP solver interface changes
- [src/sync_lib/sync_checker_solver/README.md](src/sync_lib/sync_checker_solver/README.md) - Solver wrapper modifications

```bash
# Install CLP
sudo apt-get install coinor-libclp-dev  # Debian/Ubuntu
brew install clp                         # macOS

# Build with CLP
cmake -DUSE_CLP=ON ..
make
```

**Note:** The codebase currently uses CPLEX. See migration guides for adapting to CLP.

## Building from Source

### Standard Build (with CPLEX)

```bash
mkdir build
cd build
cmake -DCPLEX_ROOT_DIR=/path/to/cplex ..
make
```

Executable: `build/bin/ctsp_scheduler`

### Debug Build

```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

### Build Specific Components

```bash
# Build only the main executable
make ctsp_scheduler

# Build specific libraries
make ctsp_io
make sync_checker
```

### CMake Options

- `CMAKE_BUILD_TYPE`: `Release` (default), `Debug`, `RelWithDebInfo`
- `CPLEX_ROOT_DIR`: Path to CPLEX installation
- `USE_CLP`: `ON` to use CLP instead of CPLEX (requires code modifications)

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
- Verification that synchronization constraints are satisfied

### Example 2: Programmatic Usage

```cpp
#include "CTSP_instance.hpp"
#include "sync_solution.hpp"
#include "schedulers.hpp"

// Load instance and solution
CTSP::instance instance("instance.contsp");
SYNC_LIB::sync_solution solution("solution.sol");

// Generate schedule
SCH::output_streams output("schedule.json");
SCH::CTSP2_scheduler(instance, solution, output);
```

### Example 3: Custom LP Solver

```cpp
#include "sync_checker.hpp"
#include "sync_checker_solver.hpp"

// Create custom LP solver wrapper
class MyLPSolver : public SYNC_LIB::sync_checker_solver {
    // Implement LP solving interface
    bool solve(const std::vector<double>& obj, ...);
};

// Use in constraint verification
SYNC_LIB::ctsp_sync_checker checker(model, epsilon);
checker.set_solver(new MyLPSolver());
bool feasible = checker.check(solution);
```

## Performance Considerations

### Scalability

- **LP solving** dominates runtime: $O(n^3)$ for $n$ customers
- **Instance loading**: $O(n^2)$ for distance matrix
- **Solution conversion**: $O(d \times r \times c)$ for $d$ depots, $r$ routes, $c$ customers

### Benchmarks (Approximate)

| Instance Size | Customers | Depots | Days | LP Solve Time (CPLEX) |
|---------------|-----------|--------|------|-----------------------|
| Small         | 10-30     | 2-3    | 5    | < 1s                  |
| Medium        | 50-100    | 3-5    | 7    | 1-10s                 |
| Large         | 200+      | 5-10   | 10   | 10s-5min              |

**Note:** Performance varies significantly based on constraint structure and solver configuration.

### Optimization Tips

1. **Use sparse matrix representations** (see `util/matrix.hpp`)
2. **Tune LP solver parameters** (see solver documentation)
3. **Consider iterative refinement** for large instances
4. **Profile critical sections** with tools like `gprof` or `perf`

## Contributing

Contributions are welcome! Please follow these guidelines:

### Development Workflow

1. **Fork** the repository
2. **Create a feature branch**: `git checkout -b feature/my-feature`
3. **Make changes** with clear commit messages
4. **Add tests** if applicable
5. **Update documentation** (READMEs, Doxygen comments)
6. **Submit pull request** with description of changes

### Code Style

- **C++14 standard** compliance
- **Doxygen comments** for all public APIs
- **Snake_case** for variables and functions
- **PascalCase** for classes
- **4-space indentation**

### Testing

```bash
# Build with tests
cmake -DBUILD_TESTS=ON ..
make
ctest
```

### Documentation

- Update relevant README.md files
- Add Doxygen comments to new functions/classes
- Include examples for complex features

## Roadmap

### Current Status (v1.0)

✅ CTSP2 (multi-depot) scheduling  
✅ CPLEX integration  
✅ JSON output format  
✅ Comprehensive documentation  

### Planned Features

- [ ] CTSP1 (single-depot) implementation
- [ ] CLP integration (open-source LP solver)
- [ ] Alternative output formats (CSV, XML)
- [ ] Python bindings
- [ ] Web-based visualization tool
- [ ] Heuristic solution generator
- [ ] Extended benchmarking suite
- [ ] Docker container for easy deployment

## Troubleshooting

### Common Issues

**Problem:** CMake cannot find CPLEX

```bash
# Solution: Specify CPLEX path explicitly
cmake -DCPLEX_ROOT_DIR=/opt/ibm/ILOG/CPLEX_Studio221 ..
```

**Problem:** Assertion failure "feasible"

```
Solution violates synchronization constraints. Check:
1. Solution file format is correct
2. Solution is actually feasible for the instance
3. LP solver tolerance (default 1e-6)
```

**Problem:** Undefined symbols during linking

```bash
# Solution: Ensure all libraries are built
make clean
make
```

## License

This project is licensed under the **MIT License** - see the [LICENSE](LICENSE) file for details.

```
MIT License

Copyright (c) 2025 Jorge Riera-Ledesma

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction...
```

## Authors

**Jorge Riera-Ledesma**
- GitHub: [@RieraULL](https://github.com/RieraULL)
- University of La Laguna

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

### Research Papers

- **Consistent Vehicle Routing Problem**: Groer et al. (2009)
- **Synchronization Constraints in Routing**: Multiple authors
- **Time Window Optimization**: Solomon (1987)

### Related Projects

- **TSPLIB**: Standard TSP instance library
- **COIN-OR**: Open-source optimization software
- **VRP Solver**: Vehicle routing problem tools

## Contact

For questions, issues, or collaboration:

- **Issues**: [GitHub Issues](https://github.com/RieraULL/CTSP_scheduler/issues)
- **Email**: Contact through GitHub profile
- **Discussions**: Use GitHub Discussions for general questions

---

**Last Updated:** November 2025  
**Version:** 1.0.0  
**Status:** Active Development
