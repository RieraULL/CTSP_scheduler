# CTSP Scheduler Application

## Overview

The **CTSP Scheduler** is the main executable application that converts Consistent Traveling Salesman Problem (CTSP) routing solutions into temporal schedules with time windows. It integrates all project libraries to produce actionable schedules from optimization solutions.

## Purpose

This application bridges the gap between optimization output (routing solutions) and practical implementation by:

1. **Loading problem instances** in TSPLIB-extended format (.contsp)
2. **Loading routing solutions** from optimizer output (.sol)
3. **Computing temporal schedules** that satisfy synchronization constraints
4. **Generating time windows** for each customer visit
5. **Writing schedules** in JSON format for visualization and execution

## Architecture

### Components

```
main/
├── include/
│   ├── sch_io.hpp         # I/O utilities for file management
│   └── schedulers.hpp     # Scheduling algorithm declarations
├── src/
│   ├── main.cpp           # Entry point and command-line parsing
│   ├── sch_io.cpp         # Implementation of I/O utilities
│   └── schedulers.cpp     # Implementation of scheduling algorithms
└── CMakeLists.txt         # Build configuration
```

### Key Classes

#### `SCH::input_files`
Manages input file paths:
- `ins_file`: Path to CTSP instance (.contsp format)
- `out_file`: Path to solution file (.sol format)
- `prob_type`: Problem variant (CTSP1 or CTSP2)

#### `SCH::output_streams`
Manages output file stream:
- `sch_s`: Output stream for schedule (.sched.json format)

### Scheduling Functions

#### `CTSP2_scheduler`
Generates temporal schedule for CTSP2 (multi-depot) problems:
1. Builds synchronization model from CTSP instance
2. Converts solution to model_a representation
3. Verifies synchronization constraints via LP
4. Computes operation start times
5. Generates customer time windows
6. Writes JSON schedule to output

#### `run_method`
Dispatches to appropriate scheduler based on problem type using function pointers.

## Usage

### Command Line

```bash
./ctsp_scheduler <problem_type> <instance_file> <solution_file> <output_file>
```

**Arguments:**
- `problem_type`: Problem variant identifier
  - `"ctsp2"` - Multi-depot CTSP (currently supported)
  - `"ctsp1"` - Single-depot CTSP (future)
- `instance_file`: Path to CTSP instance file (.contsp)
- `solution_file`: Path to feasible solution file (.sol)
- `output_file`: Path for output schedule file (.sched.json)

### Example

```bash
./ctsp_scheduler ctsp2 input/bayg29_p5_f90_lL.contsp input/bayg29_p5_f90_lL.sol output/bayg29_p5_f90_lL.sched.json
```

## Input File Formats

### Instance File (.contsp)

Extended TSPLIB format with CTSP-specific sections:
- **Standard TSPLIB**: `NAME`, `TYPE`, `DIMENSION`, `EDGE_WEIGHT_TYPE`, `NODE_COORD_SECTION`
- **CTSP Extensions**: 
  - `DEPOTS_SECTION`: Multi-depot configuration
  - `PRECEDENCE_SECTION`: Synchronization constraints
  - `PROCESSING_TIME_SECTION`: Customer service durations

Example:
```
NAME: bayg29_p5_f90_lL
TYPE: TSP
DIMENSION: 29
EDGE_WEIGHT_TYPE: EUC_2D
NODE_COORD_SECTION
1 1150.0 1760.0
...
DEPOTS_SECTION
1
5
-1
PRECEDENCE_SECTION
...
EOF
```

### Solution File (.sol)

Contains feasible CTSP routing solution:
- Routes per depot
- Visit sequences
- Arc variables

Format is specific to the sync_solution class (see sync_lib/sync_IO documentation).

## Output Format

### Schedule File (.sched.json)

JSON format with schedules for each depot and time windows for each customer:

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
        },
        ...
      ]
    },
    ...
  ],
  "time_windows": {
    "3": {"earliest": 0.0, "latest": 10.5},
    ...
  }
}
```

**Fields:**
- `depot_id`: Depot identifier
- `customer`: Customer node ID
- `arrival_time`: Time when vehicle arrives at customer
- `start_time`: Time when service begins (respects time windows)
- `departure_time`: Time when vehicle leaves customer
- `time_windows`: Valid service time ranges for each customer

## Dependencies

The scheduler integrates all project libraries:

### Direct Dependencies
- **CTSP/IO** (`sub::ctsp_io`): Instance file parsing
- **CTSP/interface** (`sub::ctsp_interfaz`): CTSP model builder adapter
- **sync_verify** (`sub::sync_verify`): Solution-to-schedule converter
- **sync_checker** (`sub::sync_checker`): Constraint verification
- **sync_checker_solver** (`sub::sync_checker_solver`): LP solver wrapper
- **sync_IO** (`sub::sync_model_a`): Model and solution I/O
- **util** (`sub::gomautil`): Matrix utilities and LP interface

### Transitive Dependencies
- **IBM ILOG CPLEX 22.1**: LP solver (commercial, see migration notes)

## Build Instructions

### Prerequisites
- CMake 3.10 or higher
- C++14 compatible compiler
- CPLEX 22.1 (or CLP for open-source builds)

### Build Steps

```bash
# From project root
mkdir build
cd build
cmake ..
make ctsp_scheduler
```

Executable will be at: `build/bin/ctsp_scheduler`

### Build with CLP (Open-Source)

To migrate from CPLEX to CLP, see:
- `src/util/README.md` - Complete CLP migration guide
- `src/sync_lib/sync_checker_solver/README.md` - Solver wrapper modifications

## Algorithm Workflow

### High-Level Process

```
1. Parse command-line arguments
   ↓
2. Load CTSP instance (TSPLIB + extensions)
   ↓
3. Load feasible solution (routing)
   ↓
4. Build synchronization model
   ↓
5. Convert solution format (sync_solution → model_a)
   ↓
6. Solve LP to compute schedules
   - Minimize depot completion times
   - Satisfy synchronization constraints
   - Respect service durations
   ↓
7. Extract schedules per depot
   ↓
8. Compute customer time windows
   ↓
9. Write JSON output
```

### Scheduling Algorithm Details

The `CTSP2_scheduler` function implements:

1. **Model Building**: Creates synchronization constraint model from instance using `CTSP_model_a_builder`

2. **Solution Conversion**: Transforms routing solution to LP variable format via `model_a_solution_interface`

3. **LP Solving**: Uses `conTSP2_scheduling` to solve LP that computes:
   - Operation start times (t_ij)
   - Customer time windows [e_i, l_i]
   - Depot completion times

4. **Schedule Extraction**: Processes LP solution to generate per-depot schedules with arrival/departure times

5. **JSON Export**: Writes schedules in structured JSON format

## Error Handling

### Invalid Arguments
```
Usage: ./ctsp_scheduler <problem_type> <instance_file> <solution_file> <output_file>
```
Returns exit code 1.

### Invalid Problem Type
```
ERROR: Incorrect problem type
```
Exits with code 1. Valid types: `"ctsp2"`, `"ctsp1"`.

### Infeasible Solution
Assertion failure if LP has no solution (solution violates synchronization constraints).

## Implementation Notes

### Function Pointer Arrays
The implementation uses function pointer arrays for efficient dispatching:
```cpp
scheduler_ptr scheduler_array[] = {CTSP2_scheduler};
sch_method_ptr sch_method_array[] = {ctsp2_scheduler};
```
This allows easy extension to support CTSP1 and other variants.

### Numerical Tolerance
Scheduler uses tolerance of `1e-6` for LP solving to handle floating-point precision.

### Resource Management
- `output_streams` destructor automatically closes output file
- RAII pattern ensures proper cleanup

## Extension Points

### Adding CTSP1 Support

1. Implement `CTSP1_scheduler` function similar to `CTSP2_scheduler`
2. Implement `ctsp1_scheduler` workflow
3. Add to function pointer arrays
4. Update `set_files` to recognize `"ctsp1"` problem type

### Custom Output Formats

Extend `output_streams` class to support additional formats (XML, CSV, etc.).

### Additional Objectives

Modify LP in `conTSP2_scheduling` to optimize different objectives (minimize lateness, maximize throughput, etc.).

## Related Documentation

- **CTSP/IO**: Instance file format specification
- **CTSP/interface**: Model builder API
- **sync_verify**: Scheduling algorithm details
- **sync_checker**: Constraint verification
- **util**: LP solver interface and CLP migration

## Performance Considerations

### Scalability
- LP solving time dominates: O(n³) for n customers
- Instance loading: O(n²) for distance matrix
- Solution conversion: O(routes × customers)

### Optimization Tips
- Use sparse matrix representations in LP (see util/README.md)
- Consider iterative refinement for large instances
- Profile LP solver performance (CPLEX vs CLP)

## License

Part of CTSP_scheduler project. See repository LICENSE file.

## Authors

Contributors to CTSP scheduling research and implementation.

## References

For CTSP problem formulation and synchronization constraints, see:
- Consistent Vehicle Routing Problem literature
- Synchronization constraint verification papers
- Multi-depot routing with time windows
