# sync_IO - Input/Output and Model Building Library for CTSP

## Overview

The `sync_IO` library provides data structures, model builders, and I/O utilities for solving Consistent Traveling Salesman Problems (CTSP) and related vehicle routing problems with synchronization constraints.

## Key Components

### 1. Data Structures (`sync_types.hpp`, `sync_solution.hpp`, `sync_scheduling.hpp`, `sync_tw.hpp`)

- **sync_solution**: Represents routing solutions as sequences of customer visits per vehicle
- **sync_scheduling**: Extends solutions with precise timing information (arrival times, service start times)
- **sync_time_windows**: Time window constraints for operations
- **triplet**: Represents arcs in the routing graph with operation and subset information

### 2. Operations and Partitions (`sync_operations.hpp`)

Core abstraction for CTSP problems:

- **sync_operation**: A task representing a specific vehicle visiting a specific customer
- **operation_arc**: Feasible transition between two operations
- **operations_subset**: Group of related operations (e.g., all tasks for one vehicle)
- **operations_partition**: Complete decomposition of operations (routing partition, synchronization partition)

### 3. Model Builders

#### Base Builder (`sync_model_builder.hpp`)

Creates an intermediate operation-based representation:

- Transforms problem instances into operations and partitions

- Routing partition: Groups operations by vehicle

- Synchronization partition: Groups operations by customer

- Supports CTSP1 (time window sync) and CTSP2 (exact sync)

#### Model A Builder (`sync_model_a_builder.hpp`)

Advanced arc-based formulation for mathematical programming:

- Explicit routing and synchronization arcs

- Efficient mappings between operations, arcs, and decision variables

- Operation metadata: customer-depot assignments, time windows, costs

- Suitable for MIP solvers (CPLEX, Gurobi) and branch-and-cut algorithms

**Reference**: Riera-Ledesma et al., "Dual-driven path elimination for vehicle routing with idle times and arrival-time consistency", Computers & Operations Research, 2025, 107326.

### 4. Solution Conversion (`model_a_solution_interface.hpp`)

Bidirectional converter between:

- High-level representation: `sync_solution` (routes as sequences)

- Low-level representation: Model A decision variables (arc selection variables)

### 5. I/O Utilities

- **json_format_io.hpp**: Simple JSON parser/writer for solutions and schedules
  - Note: This is a basic ad-hoc implementation. For production, consider using established JSON libraries (e.g., nlohmann/json, RapidJSON)

- **sync_mapping.hpp**: Efficient mapping from operation pairs `(i,j)` to linear indices

## Usage Example

```cpp
#include "sync_model_a_builder.hpp"
#include "sync_solution.hpp"
#include "model_a_solution_interface.hpp"

// Build Model A from problem instance
SYNC_LIB::sync_model_a_builder model_builder(
    problem_type,     // 1=CTSP1, 2=CTSP2
    instance_name,
    n_vehicles,
    n_depots,
    n_customers,
    demands,
    max_distance,
    time_windows,
    distance_matrix,
    triangle_inequality
);

// Access model components
const auto& routing_arcs = model_builder.get_routing_arcs();
const auto& sync_arcs = model_builder.get_sync_arcs();
const auto& operations = model_builder.get_operations();

// Convert solution between representations
SYNC_LIB::model_a_solution_interface converter;
converter.set(model_builder);

// High-level to low-level
vector<double> x_vars;
converter.sync_solution_2_model_a(solution, x_vars);

// Low-level to high-level
SYNC_LIB::sync_solution solution;
converter.model_a_2_sync_solution(x_vars, solution);

// I/O operations
solution.write_json(output_stream);
schedule.write_json(output_stream);
```

## Problem Types

- **CTSP1**: Synchronization within time windows (vehicles must arrive within specified windows)
- **CTSP2**: Exact synchronization (vehicles must arrive simultaneously)

## Dependencies

- `matrix.hpp`: Matrix data structure (from util library)
- C++ Standard Library (vector, map, iostream, string, utility)

## License

See main project LICENSE file.

## References

For more details on Model A and the algorithmic approach:

Jorge Riera-Ledesma, Inmaculada Rodríguez-Martín, Hipólito Hernández–Pérez,
"Dual-driven path elimination for vehicle routing with idle times and arrival-time consistency",
Computers & Operations Research, 2025, 107326,
ISSN 0305-0548,
<https://doi.org/10.1016/j.cor.2025.107326>
