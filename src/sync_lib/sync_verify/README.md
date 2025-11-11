# sync_verify - CTSP Solution to Scheduling Converter

## Overview

The `sync_verify` library converts Consistent Traveling Salesman Problem (CTSP) solutions into practical temporal schedules with customer time windows. It verifies that synchronization constraints are satisfied and generates implementation-ready schedules for multi-vehicle routing.

**Key Features:**

- ✅ Verifies synchronization constraints via LP formulation

- ⏰ Computes operation start times for all vehicles

- 📅 Generates ordered schedules for each depot

- 🎯 Computes customer time windows respecting maximum width constraints

- 🔍 Validates temporal feasibility of routes

## Purpose

In CTSP, multiple vehicles must visit shared customers with synchronization constraints (e.g., customer availability windows, coordination requirements). This library:

1. Takes a CTSP solution (decision variables indicating which arcs are used)
2. Verifies that synchronization constraints can be satisfied
3. Computes actual operation times via Linear Programming
4. Generates practical schedules showing when each vehicle visits each location
5. Computes tight customer time windows for implementation

## Main Component

### `conTSP2_scheduling` Class

Converts CTSP solutions to temporal schedules.

**Constructor:**
```cpp
conTSP2_scheduling(const sync_model_a_builder &builder, double tol);
```
- `builder`: Problem instance data (customers, depots, travel times, constraints)
- `tol`: Numerical tolerance for LP solver

**Main Method:**
```cpp
bool solve(const string &instance_name, 
           const vector<double> &x, 
           sync_scheduling &scheduling, 
           sync_time_windows &time_windows);
```

**Parameters:**
- `instance_name`: Identifier for the problem instance
- `x`: CTSP decision variables (arc usage indicators from optimization)
- `scheduling` [output]: Generated schedule for each depot
- `time_windows` [output]: Computed time windows for each customer

**Returns:**
- `true` if synchronization constraints are satisfied
- `false` if constraints are violated (infeasible solution)

## Algorithm

The conversion process consists of four main steps:

### 1. Synchronization Verification
Uses `sync_iterative_checker` with LP formulation to verify that the solution satisfies all synchronization constraints. If feasible, computes operation start times $s_i$ for all operations.

**LP Model:**
```
Minimize:   (any feasible solution)
Subject to: s_j ≥ s_i + t_{ij}  ∀ arcs (i,j) in solution
            |s_i - s_j| ≤ W      ∀ synchronized operations (i,j)
            s_i ≥ 0              ∀ operations i
```

Where:
- $s_i$: Start time of operation $i$
- $t_{ij}$: Travel time from operation $i$ to operation $j$
- $W$: Maximum time window width

### 2. Time Normalization
Shifts all start times so the earliest depot operation begins at $t = 0$:
```cpp
s_min = min{s_k : k ∈ depots}
s_i ← s_i - s_min  ∀ i
```

### 3. Schedule Generation
For each depot:
1. **Group operations** by depot
2. **Sort by start time** (temporal ordering)
3. **Position depot pickup** at the end of the route
4. **Compute arrival times**:
   ```
   arrival[i] = start[i-1] + travel_time(i-1, i)
   ```
5. **Convert IDs** to customer identifiers for output

**Schedule Format:**
Each operation has:
- Customer ID (or depot ID for departure/return)
- Arrival time (earliest possible arrival)
- Start time (actual service start, may include waiting)

### 4. Time Window Computation
For each customer $c$:
1. **Collect all visit times**: $T_c = \{s_i : \text{operation } i \text{ visits customer } c\}$
2. **Compute span**: $\Delta_c = \max(T_c) - \min(T_c)$
3. **Verify constraint**: $\Delta_c \leq W$ (maximum window width)
4. **Center window** around average visit time:
   ```
   center = (min(T_c) + max(T_c)) / 2
   [a, b] = [center - W/2, center + W/2]
   ```
5. **Adjust for non-negativity**: $a \leftarrow \max(0, a)$

The depot time window is set to $[0, D_{max}]$ where $D_{max}$ is the maximum route duration.

## Usage Example

```cpp
#include "sol_2_scheduling.hpp"
#include "sync_model_a_builder.hpp"

// Load problem instance
sync_model_a_builder builder;
builder.load_instance("bayg29_p5_f90_lL.contsp");

// Create converter with tolerance 1e-6
conTSP2_scheduling converter(builder, 1e-6);

// CTSP solution from optimization (arc usage variables)
vector<double> x_solution = /* solution from solver */;

// Output structures
sync_scheduling scheduling;
sync_time_windows time_windows;

// Convert solution to schedule
bool is_feasible = converter.solve("bayg29_instance", x_solution, 
                                    scheduling, time_windows);

if (is_feasible) {
    cout << "Solution is feasible!" << endl;
    
    // Access depot schedules
    for (size_t depot = 0; depot < scheduling.size(); ++depot) {
        cout << "Depot " << depot << " route:" << endl;
        for (const auto& [customer_id, times] : scheduling[depot]) {
            auto [arrival, start] = times;
            cout << "  Customer " << customer_id 
                 << ": arrival=" << arrival 
                 << ", start=" << start << endl;
        }
    }
    
    // Access time windows
    for (size_t customer = 1; customer < time_windows.size(); ++customer) {
        auto [a, b] = time_windows[customer];
        cout << "Customer " << customer 
             << " time window: [" << a << ", " << b << "]" << endl;
    }
} else {
    cout << "Solution violates synchronization constraints!" << endl;
}
```

## Output Interpretation

### Schedule Format
For each depot, you get an ordered list of operations:
```
Depot 0:
  [0] Customer 1 (depot departure): arrival=0.0, start=0.0
  [1] Customer 5: arrival=12.3, start=12.3
  [2] Customer 8: arrival=25.7, start=26.0  (waited 0.3 time units)
  [3] Customer 3: arrival=40.1, start=40.1
  [4] Customer 1 (depot return): arrival=55.4, start=55.4
```

**Interpretation:**
- First/last operations are depot departure/return (Customer ID = 1)
- `arrival`: Earliest time the vehicle can arrive (previous start + travel)
- `start`: Actual service start (may include waiting for synchronization)
- Waiting occurs when `start > arrival` (synchronization constraint)

### Time Windows Format
```
Customer 5: [10.0, 30.0]   // Width = 20.0
Customer 8: [20.0, 40.0]   // Width = 20.0
Customer 3: [35.0, 55.0]   // Width = 20.0
```

**Interpretation:**
- All vehicle visits to a customer occur within its time window
- Window width ≤ maximum allowed (problem constraint)
- Windows are as tight as possible while containing all visits

## Dependencies

- **sync_model_a**: Problem instance data and model building
- **sync_checker**: Synchronization constraint verification via LP
  - Uses `sync_iterative_checker<ctsp_lb_sync_checker>`
- **COIN-OR CLP or CPLEX**: LP solver (through sync_checker_solver)

## Integration

This library is used in the main scheduler to:
1. **Validate solutions** from optimization algorithms
2. **Generate output schedules** for implementation
3. **Compute time windows** for customer communication
4. **Debug infeasible solutions** (identifies constraint violations)

## Mathematical Foundation

### Synchronization Constraints
For operations $(i, j)$ that must be synchronized:
$$|s_i - s_j| \leq W$$

Where $W$ is the maximum time window width. This ensures both vehicles visit the shared customer within a window of width $W$.

### Temporal Precedence
For consecutive operations in a route:
$$s_j \geq s_i + t_{ij}$$

Where $t_{ij}$ is the travel time from $i$ to $j$.

### Time Window Feasibility
For customer $c$ visited by operations $O_c$:
$$\max_{i \in O_c} s_i - \min_{i \in O_c} s_i \leq W$$

## Notes

- **LP Verification**: Uses Linear Programming to verify constraint satisfaction (polynomial time)
- **Normalization**: Time shifting preserves all temporal relationships
- **Depot Handling**: Ensures depot pickup operations are correctly positioned at route end
- **Assertions**: Debug builds include extensive feasibility checks
- **Tolerance**: Uses 1e-6 numerical tolerance for floating-point comparisons

## See Also

- `sync_checker`: For the underlying LP-based synchronization verification
- `sync_IO`: For I/O utilities and data structures (sync_scheduling, sync_tw)
- `sync_model_a_builder`: For problem instance construction

---

**License:** See repository root LICENSE file  
**Author:** Universidad de La Laguna  
**Project:** CTSP_scheduler - Consistent TSP Scheduling System
