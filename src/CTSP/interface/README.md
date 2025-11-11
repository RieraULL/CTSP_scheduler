# CTSP Interface - Model Builder Adapter

## Overview

The `ctsp_interfaz` library provides the bridge between CTSP problem instances and the synchronization constraint framework. It adapts CTSP-specific data structures to the generic `sync_model_a_builder` interface used throughout the solver.

**Purpose:**
- Convert CTSP instances to synchronization models
- Support multiple CTSP variants (CTSP1, CTSP2)
- Enable use of sync_lib constraint verification tools

## Components

### CTSP_model_a_builder

Adapter class that transforms CTSP instances into synchronization models.

**Key Responsibilities:**
1. Extract problem data from CTSP instance
2. Determine depot configuration based on problem type
3. Pass data to sync_model_a_builder base class
4. Enable constraint verification and solving

**Problem Variants:**

| Variant | Description | Depot Configuration |
|---------|-------------|---------------------|
| **CTSP1** | Single-depot | All vehicles share one depot |
| **CTSP2** | Multi-depot | Each vehicle has its own depot |

## Usage

### Basic Example

```cpp
#include "CTSP_model_a_builder.hpp"
#include "CTSP_instance.hpp"

// Load CTSP instance
CTSP::instance instance("bayg29_p5_f90_lL.contsp");

// Create model builder for CTSP2 (multi-depot variant)
CTSP::CTSP_model_a_builder builder(
    CTSP::CTSP_problem_type::CTSP2,
    instance
);

// Now the builder can be used with sync_lib tools:

// 1. Constraint verification
SYNC_LIB::sync_iterative_checker<ctsp_lb_sync_checker> checker(builder, 1e-6);
bool feasible = checker.is_feasible(solution_x, start_times);

// 2. Solution validation
conTSP2_scheduling verifier(builder, 1e-6);
sync_scheduling schedule;
sync_time_windows tw;
verifier.solve(instance.get_instance_name(), solution_x, schedule, tw);

// 3. Access model properties
size_t n_depots = builder.get_n_depots();
size_t n_customers = builder.get_n_customers();
size_t n_operations = builder.get_n_operations();
const auto& arc_times = builder.get_arc_time_matrix();
```

### Problem Type Selection

**CTSP1 - Single Depot:**

```cpp
// All vehicles depart from and return to the same depot
CTSP::CTSP_model_a_builder builder1(
    CTSP::CTSP_problem_type::CTSP1,
    instance
);

// Results in:
// - n_depots = 1
// - All depot operations reference the same location
```

**CTSP2 - Multi-Depot:**

```cpp
// Each vehicle has its own depot (more general)
CTSP::CTSP_model_a_builder builder2(
    CTSP::CTSP_problem_type::CTSP2,
    instance
);

// Results in:
// - n_depots = n_days (one depot per vehicle)
// - Each vehicle operates independently from its depot
```

**When to use each:**
- **CTSP1**: Simpler model, all vehicles at same location
- **CTSP2**: More flexible, allows different depot locations per vehicle

### Integration with Solver Pipeline

Complete workflow from instance to solution:

```cpp
#include "CTSP_model_a_builder.hpp"
#include "CTSP_instance.hpp"
#include "sync_iterative_checker.hpp"
#include "sol_2_scheduling.hpp"

void solve_ctsp_instance(const std::string& filename) {
    // Step 1: Load instance
    CTSP::instance instance(filename);
    
    std::cout << "Instance: " << instance.get_instance_name() << std::endl;
    std::cout << "Customers: " << instance.get_n_customers() << std::endl;
    std::cout << "Days: " << instance.get_n_days() << std::endl;
    
    // Step 2: Build model
    CTSP::CTSP_model_a_builder builder(
        CTSP::CTSP_problem_type::CTSP2,
        instance
    );
    
    // Step 3: Solve optimization problem
    // (This would use your optimization algorithm)
    std::vector<double> solution_x = solve_optimization(builder);
    
    // Step 4: Verify synchronization constraints
    SYNC_LIB::sync_iterative_checker<ctsp_lb_sync_checker> checker(builder, 1e-6);
    std::vector<double> start_times(builder.get_n_operations());
    
    bool is_feasible = checker.is_feasible(solution_x, start_times);
    
    if (!is_feasible) {
        std::cerr << "Solution violates synchronization constraints!" << std::endl;
        return;
    }
    
    // Step 5: Generate schedule
    conTSP2_scheduling scheduler(builder, 1e-6);
    sync_scheduling schedule;
    sync_time_windows time_windows;
    
    scheduler.solve(instance.get_instance_name(), solution_x, 
                    schedule, time_windows);
    
    // Step 6: Output results
    output_schedule(schedule, time_windows);
}
```

## Architecture

### Class Hierarchy

```
SYNC_LIB::sync_model_a_builder (base)
            ↑
            |
    CTSP::CTSP_model_a_builder (adapter)
```

### Data Flow

```
CTSP::instance
    ↓
CTSP::CTSP_model_a_builder
    ↓
SYNC_LIB::sync_model_a_builder
    ↓
[sync_checker, sync_verify, optimization, ...]
```

### Constructor Data Extraction

The builder constructor extracts and transforms:

```cpp
CTSP_model_a_builder::CTSP_model_a_builder(
    const CTSP_problem_type &problem_type,
    const CTSP::instance &instance)
    : sync_model_a_builder(
        problem_type == CTSP1 ? 1 : 2,      // Model type
        instance.get_instance_name(),        // Instance ID
        1,                                   // n_depots for CTSP1
        instance.get_n_days(),               // n_days
        instance.get_n_customers(),          // n_customers
        instance.get_demands(),              // Demand matrix
        instance.get_max_distance(),         // Route constraint
        instance.get_T(),                    // Time windows
        instance.get_distances(),            // Distance matrix
        instance.triangle_inequality()       // Validation flag
    )
{
    // Depot count determined by problem type:
    // CTSP1: n_depots = 1 (all share depot)
    // CTSP2: n_depots = n_days (one per vehicle)
}
```

## API Reference

### CTSP_problem_type Enum

```cpp
enum class CTSP_problem_type {
    CTSP1,  // Single-depot variant
    CTSP2   // Multi-depot variant
};
```

### CTSP_model_a_builder Class

**Constructor:**
```cpp
CTSP_model_a_builder(
    const CTSP_problem_type &problem_type,
    const CTSP::instance &instance
);
```

**Inherited Methods from sync_model_a_builder:**

```cpp
// Dimensions
size_t get_n_depots() const;
size_t get_n_customers() const;
size_t get_n_days() const;
size_t get_n_operations() const;

// Problem data
const GOMA::matrix<double>& get_arc_time_matrix() const;
const vector<double>& get_time_windows_max_size() const;
double get_max_distance() const;

// Operation mappings
const vector<int>& get_operation_2_depot() const;
const vector<int>& get_operation_2_customer() const;
const vector<string>& get_operation_names() const;

// Demand information
const vector<vector<int>>& get_demands() const;
```

## Model Conversion Details

### Operation Indexing

The builder creates operations in this order:

1. **Depot deliveries** (indices 0 to n_depots-1)
   - Initial departure from depot for each vehicle

2. **Depot pickups** (indices n_depots to 2*n_depots-1)
   - Return to depot for each vehicle

3. **Customer visits** (indices 2*n_depots onwards)
   - One operation per customer visit requirement
   - Ordered by customer and day

### Example: CTSP2 with 3 customers, 2 days

```
Operations:
[0, 1]       : Depot deliveries (vehicles 0, 1)
[2, 3]       : Depot pickups (vehicles 0, 1)
[4, 5, 6]    : Customer visits on day 0 (if needed)
[7, 8, 9]    : Customer visits on day 1 (if needed)
```

### Arc Time Matrix

Distance between operations i and j:
- Computed from customer distance matrix
- Includes depot-to-customer and customer-to-customer distances
- Used for temporal precedence constraints

## Validation

The builder performs automatic validation:

**Triangle Inequality Check:**
```cpp
if (!instance.triangle_inequality()) {
    // Warning: May affect solver correctness
    // Some algorithms assume triangle inequality
}
```

**Demand Consistency:**
```cpp
// Verifies:
// - All demand values are non-negative
// - At least one customer has positive demand
// - Demand matrix dimensions match n_customers × n_days
```

## Dependencies

- `CTSP::instance` (from ctsp_io): Problem instance
- `SYNC_LIB::sync_model_a_builder` (from sync_IO): Base model builder
- `util`: Matrix and utility classes

## Common Use Cases

### 1. Constraint Verification

```cpp
CTSP::CTSP_model_a_builder builder(
    CTSP::CTSP_problem_type::CTSP2, instance);

// Check if solution satisfies sync constraints
ctsp_sync_checker checker(builder, 1e-6);
bool feasible = checker.check_sync_constraints(solution);
```

### 2. Schedule Generation

```cpp
CTSP::CTSP_model_a_builder builder(
    CTSP::CTSP_problem_type::CTSP2, instance);

// Convert solution to temporal schedule
conTSP2_scheduling converter(builder, 1e-6);
sync_scheduling schedule;
sync_time_windows tw;
converter.solve(instance_name, solution, schedule, tw);
```

### 3. Optimization Model Building

```cpp
CTSP::CTSP_model_a_builder builder(
    CTSP::CTSP_problem_type::CTSP2, instance);

// Access model data for optimization
const auto& arc_times = builder.get_arc_time_matrix();
const auto& max_tw = builder.get_time_windows_max_size();

// Build and solve optimization model...
```

## Performance Considerations

- **Construction**: O(n²) due to distance matrix access
- **Memory**: Stores references to instance data (no deep copy)
- **Model size**: n_operations = n_depots*2 + sum of all demands

## See Also

- `CTSP/IO`: Instance loading and validation
- `sync_lib/sync_IO`: Base model builder interface
- `sync_lib/sync_checker`: Constraint verification
- `sync_lib/sync_verify`: Solution validation

---

**License:** See repository root LICENSE file  
**Author:** Universidad de La Laguna  
**Project:** CTSP_scheduler - Consistent TSP Scheduling System
