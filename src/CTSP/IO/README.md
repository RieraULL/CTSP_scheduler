# CTSP IO - Instance File I/O Library

## Overview

The `ctsp_io` library provides comprehensive file input/output capabilities for Traveling Salesman Problem variants, with emphasis on multi-day routing problems with synchronization constraints.

**Supported Problem Types:**
- **TSP**: Classic Traveling Salesman Problem
- **PTSP**: Periodic TSP (multi-day without synchronization)
- **CTSP**: Consistent TSP (multi-day with time window synchronization)

## Components

### 1. TSPLIB_instance (`TSPLIB_instance.hpp/cpp`)

Parser for TSPLIB format instance files - the de facto standard for TSP benchmarks.

**Key Features:**
- Reads standard TSPLIB format files
- Supports multiple distance calculation methods
- Handles various edge weight matrix formats
- Extended keywords for multi-day problems

**Supported Distance Types:**

| Type | Description | Example Use |
|------|-------------|-------------|
| `EUC_2D` | 2D Euclidean distance | Geometric problems |
| `GEO` | Geographic distance (lat/lon) | Real-world routing |
| `MAN_2D` | Manhattan distance | Grid-based routing |
| `CEIL_2D` | Ceiling of Euclidean | Integer distance problems |
| `ATT` | Pseudo-Euclidean | Special benchmark (att48) |
| `EXPLICIT` | Direct matrix input | Arbitrary distances |

**Example TSPLIB File:**

```tsplib
NAME : bayg29
TYPE : TSP
DIMENSION : 29
EDGE_WEIGHT_TYPE : EUC_2D
NODE_COORD_SECTION
1 1150.0 1760.0
2 630.0 1660.0
3 40.0 2090.0
...
EOF
```

**Extended Format for CTSP:**

```tsplib
NAME : bayg29_p5_f90_lL
TYPE : CTSP
DIMENSION : 30
NUM_DAYS : 5
DISTANCE : 900
MAXIMUM_ALLOWABLE_DIFFERENTIAL : 90
EDGE_WEIGHT_TYPE : EUC_2D
NODE_COORD_SECTION
...
DEMAND_SECTION
1 0 0 0 0 0
2 1 0 1 0 1
3 0 1 0 1 0
...
EOF
```

**Usage:**

```cpp
#include "TSPLIB_instance.hpp"

TSP::TSPLIB_instance instance;
instance.read("bayg29.tsp");

// Access instance data
std::cout << "Name: " << instance.get_instance_name() << std::endl;
std::cout << "Dimension: " << instance.get_dimension() << std::endl;

// Get distance matrix
GOMA::matrix<double> distances;
instance.get_distances(distances);

// Access coordinates (if available)
for (const auto& coord : instance.coord()) {
    std::cout << "(" << coord.first << ", " << coord.second << ")" << std::endl;
}

// Multi-day extensions
int n_days = instance.get_num_days();
int max_dist = instance.get_max_distance();
const auto& demands = instance.get_demands();
```

### 2. PTSP_instance (`PTSP_instance.hpp/cpp`)

Periodic TSP instance representation for multi-day routing without synchronization.

**Problem Characteristics:**
- Multiple planning days/periods
- Day-specific customer demands
- Independent routing on each day
- No coordination between days required

**Data Structure:**

```cpp
PTSP::instance ptsp;
ptsp.read("ptsp_instance.tsp");

// Instance properties
size_t n_customers = ptsp.get_n_customers();  // Customers (excluding depot)
size_t n_days = ptsp.get_n_days();            // Planning horizon

// Demands: demands[customer][day]
const auto& demands = ptsp.get_demands();
// demands[i][j] > 0 means customer i needs service on day j

// Distance matrix (shared across all days)
const auto& distances = ptsp.get_distances();

// Validation checks
bool triangle_ineq = ptsp.triangle_inequality();  // Check d(i,k)+d(k,j)≥d(i,j)
bool symmetric = ptsp.symmetry();                  // Check d(i,j)=d(j,i)
```

**Example:**

```cpp
#include "PTSP_instance.hpp"

PTSP::instance instance;
instance.read("ptsp_7day_50customers.tsp");

std::cout << "Instance: " << instance.get_instance_name() << std::endl;
std::cout << "Customers: " << instance.get_n_customers() << std::endl;
std::cout << "Days: " << instance.get_n_days() << std::endl;

// Count total visits needed
const auto& demands = instance.get_demands();
int total_visits = 0;
for (const auto& customer_demands : demands) {
    for (int demand : customer_demands) {
        if (demand > 0) total_visits++;
    }
}
std::cout << "Total visits: " << total_visits << std::endl;

// Check distance matrix properties
if (!instance.triangle_inequality()) {
    std::cerr << "Warning: Triangle inequality violated!" << std::endl;
}
```

### 3. CTSP_instance (`CTSP_instance.hpp/cpp`)

Consistent TSP instance with synchronization constraints.

**Additional Constraints Beyond PTSP:**
- **Time windows**: All visits to a customer must occur within time window T[i]
- **Route duration**: Each route limited to max_distance

**Problem Definition:**

```
Minimize: Total routing distance
Subject to:
  - All customer demands satisfied
  - For each customer i:
      max_visit_time[i] - min_visit_time[i] ≤ T[i]
  - Each route duration ≤ max_distance
```

**Usage:**

```cpp
#include "CTSP_instance.hpp"

// Load instance
CTSP::instance ctsp("bayg29_p5_f90_lL.contsp");

// Basic info
std::cout << "Instance: " << ctsp.get_instance_name() << std::endl;
std::cout << "Customers: " << ctsp.get_n_customers() << std::endl;
std::cout << "Days: " << ctsp.get_n_days() << std::endl;

// CTSP-specific constraints
const auto& time_windows = ctsp.get_T();  // T[i] = max time spread for customer i
double max_route_dist = ctsp.get_max_distance();  // Max distance per route

std::cout << "Time window width: " << time_windows[0] << std::endl;
std::cout << "Max route distance: " << max_route_dist << std::endl;

// Count operations
size_t n_operations = ctsp.get_n_customer_operations();
std::cout << "Total customer operations: " << n_operations << std::endl;

// Optional: disable max distance constraint
ctsp.disable_max_distance();  // Sets to very large value

// Benchmark comparison
const auto& optimal_values = ctsp.get_optimal_values();
if (!optimal_values.empty()) {
    std::cout << "Known optimal: " << optimal_values[0] << std::endl;
}
```

**Advanced Example - Full Instance Analysis:**

```cpp
#include "CTSP_instance.hpp"
#include <iomanip>

void analyze_ctsp_instance(const std::string& filename) {
    CTSP::instance instance(filename);
    
    // Print summary
    std::cout << "=== CTSP Instance Analysis ===" << std::endl;
    instance.write_line(std::cout);
    std::cout << std::endl;
    
    // Analyze demands
    const auto& demands = instance.get_demands();
    size_t n_customers = instance.get_n_customers();
    size_t n_days = instance.get_n_days();
    
    std::cout << "Demand Pattern:" << std::endl;
    for (size_t customer = 0; customer < n_customers; ++customer) {
        std::cout << "Customer " << (customer + 1) << ": ";
        for (size_t day = 0; day < n_days; ++day) {
            std::cout << demands[customer + 1][day] << " ";
        }
        std::cout << std::endl;
    }
    
    // Analyze distance matrix
    const auto& dist = instance.get_distances();
    double min_dist = std::numeric_limits<double>::max();
    double max_dist = 0.0;
    double avg_dist = 0.0;
    int count = 0;
    
    for (size_t i = 1; i <= n_customers; ++i) {
        for (size_t j = i + 1; j <= n_customers; ++j) {
            double d = dist(i, j);
            min_dist = std::min(min_dist, d);
            max_dist = std::max(max_dist, d);
            avg_dist += d;
            count++;
        }
    }
    avg_dist /= count;
    
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "\nDistance Statistics:" << std::endl;
    std::cout << "  Min: " << min_dist << std::endl;
    std::cout << "  Max: " << max_dist << std::endl;
    std::cout << "  Avg: " << avg_dist << std::endl;
    
    // Check properties
    std::cout << "\nProperties:" << std::endl;
    std::cout << "  Triangle inequality: " 
              << (instance.triangle_inequality() ? "Yes" : "No") << std::endl;
    std::cout << "  Symmetric: " 
              << (instance.symmetry() ? "Yes" : "No") << std::endl;
}
```

## File Format Specification

### TSPLIB Standard Keywords

```
NAME: <string>               # Instance name
TYPE: <string>               # Problem type (TSP, CTSP, etc.)
COMMENT: <string>            # Description
DIMENSION: <int>             # Number of nodes (including depot)
EDGE_WEIGHT_TYPE: <type>     # Distance calculation method
EDGE_WEIGHT_FORMAT: <fmt>    # Matrix format (if EXPLICIT)
```

### CTSP Extensions

```
NUM_DAYS: <int>                          # Number of planning days
DISTANCE: <float>                        # Maximum route distance
MAXIMUM_ALLOWABLE_DIFFERENTIAL: <float>  # Time window width T
```

### Data Sections

```
NODE_COORD_SECTION
<id> <x> <y>
...

DEMAND_SECTION
<location_id> <demand_day_1> <demand_day_2> ... <demand_day_n>
...

EOF
```

## Class Hierarchy

```
TSPLIB_instance (base parser)
         ↑
         |
    PTSP::instance (multi-day base)
         ↑
         |
    CTSP::instance (+ synchronization)
```

## Validation and Error Checking

All instance classes perform automatic validation:

**Distance Matrix Checks:**
- **Triangle inequality**: $d(i,k) + d(k,j) \geq d(i,j)$ for all $i, j, k$
- **Symmetry**: $d(i,j) = d(j,i)$ for all $i, j$

**Warnings are printed if violations detected:**

```cpp
CTSP::instance instance("problematic.tsp");
// Output: Warning: Triangle inequality violated
// Output: Warning: Distances are not symmetric
```

**Tolerance:** Triangle inequality check uses tolerance of 1.01 to account for numerical errors.

## Dependencies

- **gomautil**: Matrix class for distance storage

## Integration with CTSP Solver

These classes are used throughout the solver:

```cpp
// 1. Load instance
CTSP::instance instance("input/problem.contsp");

// 2. Build optimization model
CTSP::CTSP_model_a_builder builder(
    CTSP::CTSP_problem_type::CTSP2, 
    instance
);

// 3. Solve (in main scheduler)
// ... optimization code ...

// 4. Validate solution
sync_scheduling schedule;
sync_time_windows tw;
conTSP2_scheduling verifier(builder, 1e-6);
bool feasible = verifier.solve(instance.get_instance_name(), 
                                solution, schedule, tw);
```

## Performance Notes

- **File parsing**: O(n²) for coordinate-based instances (distance computation)
- **EXPLICIT format**: O(n²) storage, faster loading
- **Memory**: Distance matrix requires 8*n² bytes (double precision)
- **Typical instances**: 30-100 customers, 3-7 days

## Instance Repository

Common benchmark sets:
- **bayg29**: Classic 29-city TSP (Bavaria)
- **att48**: 48-city TSP (special pseudo-Euclidean)
- **CTSP instances**: Extended versions with multi-day demands

## See Also

- `CTSP/interface`: Model builder adapters
- `sync_lib`: Synchronization constraint verification
- TSPLIB documentation: http://comopt.ifi.uni-heidelberg.de/software/TSPLIB95/

---

**License:** See repository root LICENSE file  
**Author:** Universidad de La Laguna  
**Project:** CTSP_scheduler - Consistent TSP Scheduling System
