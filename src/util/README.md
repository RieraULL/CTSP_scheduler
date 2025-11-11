# util - Linear Programming Utilities Library

## Overview

The `util` library provides fundamental utilities for optimization and numerical computation in the CTSP_scheduler project. It includes:

- **Generic matrix class**: Flexible 2D matrix with 1-based indexing
- **Solver-independent model representation**: Abstract LP/MIP model descriptions
- **CPLEX solver interface**: High-performance commercial solver wrapper
- **Extensible architecture**: Easy to add support for other solvers

## ⚠️ Important Note on CPLEX and Open-Source Distribution

**Current Status:** This library uses **IBM ILOG CPLEX**, a commercial optimization solver requiring a license.

**For Open-Source Publication:** CPLEX cannot be freely distributed. We recommend migrating to an open-source alternative:

### Recommended Open-Source Alternatives

| Solver | License | Performance | Ease of Migration |
|--------|---------|-------------|-------------------|
| **COIN-OR CLP** | EPL | Excellent for LP | ⭐⭐⭐ Recommended |
| **HiGHS** | MIT | Excellent (modern) | ⭐⭐ Good |
| **GLPK** | GPL | Good for LP | ⭐⭐ Good |

### Migration to CLP (Recommended)

The library is designed with abstraction in mind. To migrate to CLP:

1. **Implement `CLP_solver`** class inheriting from `LP_solver`
2. **Update CMakeLists.txt** to link against CLP libraries instead of CPLEX
3. **Replace solver instantiation** in dependent code:
   ```cpp
   // Old (CPLEX):
   CPX_solver solver(model, tol);
   
   // New (CLP):
   CLP_solver solver(model, tol);
   ```

See the **Migration Guide** section below for detailed instructions.

## Components

### 1. Matrix Class (`matrix.hpp`)

A generic template matrix class with mathematical notation (1-based indexing).

**Features:**
- 1-based indexing: `M(i, j)` for mathematical notation
- 0-based raw access: `M[k]` for efficiency
- Dynamic resizing (with/without data preservation)
- Matrix operations: transpose, copy, fill
- I/O support with formatted output

**Example:**
```cpp
#include "matrix.hpp"

// Create a 3x4 matrix initialized to 0.0
GOMA::matrix<double> M(3, 4, 0.0);

// Set elements using 1-based indexing
M(1, 1) = 5.5;  // First element (row 1, col 1)
M(2, 3) = 7.2;  // Second row, third column

// Resize and keep existing data
M.resize_and_keep(5, 6);

// Transpose
GOMA::matrix<double> MT;
M.transpose(MT);

// I/O
std::cout << M;  // Formatted output
```

**Key Methods:**
- `matrix(m, n)`: Constructor with dimensions
- `operator()(i, j)`: 1-based element access
- `resize(m, n)`: Resize (data lost)
- `resize_and_keep(m, n)`: Resize preserving data
- `transpose(M)`: Compute transpose
- `fill(value)`: Fill all elements
- `get_m()`, `get_n()`: Get dimensions

### 2. Model Description (`model_description.hpp`)

Solver-independent representation of LP/MIP models.

**Standard Form:**
```
Minimize/Maximize:  c^T x
Subject to:         Ax {≤, =, ≥} b
                    l ≤ x ≤ u
                    x_i ∈ {Continuous, Binary, Integer}
```

**Example:**
```cpp
#include "model_description.hpp"

GOMA::model_description model;

// Set dimensions
model.set_n_col(3);  // 3 variables
model.set_n_row(2);  // 2 constraints

// Objective: minimize 2x1 + 3x2 + x3
model.set_obj({2.0, 3.0, 1.0});
model.set_obj_sense(GOMA::Minimize);

// Constraints:
//   x1 + 2x2 + x3 <= 10
//   2x1 + x2 + 3x3 >= 5
model.set_sense({'L', 'G'});
model.set_rhs({10.0, 5.0});

GOMA::matrix<double> A(2, 3);
A(1,1) = 1.0; A(1,2) = 2.0; A(1,3) = 1.0;
A(2,1) = 2.0; A(2,2) = 1.0; A(2,3) = 3.0;
model.set_M(A);

// Variable bounds: 0 <= x <= inf
model.set_prob_type(GOMA::LP);
```

**Key Enums:**
- `ObjSen`: `Minimize`, `Maximize`
- `VarType`: `C` (continuous), `B` (binary), `I` (integer)
- `ProbType`: `LP`, `MIP`

### 3. LP Solver Interface (`LP_solver.hpp`)

Abstract base class for optimization solvers.

**Interface Methods:**
```cpp
class LP_solver {
    virtual void solve() = 0;
    virtual void get_vars(double *x) const = 0;
    virtual void get_dual_vars(double *pi) const = 0;
    virtual double get_obj() const = 0;
    virtual void set_obj(double *coef, int *idx, int sz) = 0;
    virtual void set_rhs(int cnt, const int *idx, const double *val) = 0;
    virtual void add_cut(...) = 0;
    // ... more methods
};
```

**Usage Pattern:**
```cpp
// Create model
GOMA::model_description model;
// ... build model ...

// Create solver (polymorphic)
std::unique_ptr<GOMA::LP_solver> solver;

#ifdef USE_CPLEX
    solver = std::make_unique<GOMA::CPX_solver>(model, 1e-6);
#else
    solver = std::make_unique<GOMA::CLP_solver>(model, 1e-6);
#endif

// Solve
solver->solve();

// Get solution
std::vector<double> x(model.get_n_col());
solver->get_vars(x.data());

std::cout << "Objective: " << solver->get_obj() << std::endl;
```

### 4. CPLEX Solver (`CPX_solver.hpp`)

Concrete implementation using IBM ILOG CPLEX.

**Features:**
- High-performance LP/MIP solving
- Simplex and barrier methods
- Advanced presolve and cutting planes
- Dual variable retrieval (shadow prices)
- Model export (.lp, .mps formats)

**Example:**
```cpp
#include "CPX_solver.hpp"

GOMA::model_description model;
// ... build model ...

// Create CPLEX solver with tolerance 1e-6
GOMA::CPX_solver solver(model, 1e-6);

// Solve
solver.solve();

// Check status
if (solver.get_lp_stat() == CPX_STAT_OPTIMAL) {
    // Get primal solution
    std::vector<double> x(model.get_n_col());
    solver.get_vars(x.data());
    
    // Get dual solution (shadow prices)
    std::vector<double> pi(model.get_n_row());
    solver.get_dual_vars(pi.data());
    
    std::cout << "Optimal objective: " << solver.get_obj() << std::endl;
}

// Export model for debugging
solver.write_model("debug.lp");
```

**Advanced Features:**
```cpp
// Modify objective on-the-fly
int idx[] = {0, 1};
double new_obj[] = {5.0, 2.0};
solver.set_obj(new_obj, idx, 2);

// Add cutting plane
// Format: a^T x <= b
int matbeg[] = {0};
int matind[] = {0, 1, 2};  // Variable indices
double matval[] = {1.0, 1.0, 1.0};  // Coefficients
double rhs[] = {10.0};
char sense[] = {'L'};
solver.add_cut(1, rhs, sense, matbeg, matind, matval, nullptr);

// Re-solve
solver.solve();
```

## Migration Guide: CPLEX to CLP

### Step 1: Install CLP

```bash
# Ubuntu/Debian
sudo apt-get install coinor-libclp-dev coinor-libosi-dev

# From source (recommended for latest version)
git clone https://github.com/coin-or/Clp.git
cd Clp
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local
make && sudo make install
```

### Step 2: Implement CLP_solver

Create `src/util/include/CPX/CLP_solver.hpp`:

```cpp
#pragma once

#include "LP_solver.hpp"
#include <ClpSimplex.hpp>

namespace GOMA {
    class CLP_solver : public LP_solver {
    protected:
        ClpSimplex model_;
        
    public:
        CLP_solver(const model_description &model, double tol = 1e-6);
        virtual ~CLP_solver();
        
        void solve() override;
        void get_vars(double *x) const override;
        void get_dual_vars(double *pi) const override;
        double get_obj() const override;
        // ... implement other virtual methods ...
    };
}
```

Create `src/util/src/CLP_solver.cpp`:

```cpp
#include "CLP_solver.hpp"

namespace GOMA {
    CLP_solver::CLP_solver(const model_description &model, double tol)
        : LP_solver(model, tol) {
        
        // Build CLP model from model_description
        const int nCols = model.get_n_col();
        const int nRows = model.get_n_row();
        
        // Set dimensions
        model_.resize(nRows, nCols);
        
        // Set objective
        const auto& obj = model.get_obj();
        for (int j = 0; j < nCols; ++j) {
            model_.setObjectiveCoefficient(j, obj[j]);
        }
        
        // Set row bounds (RHS and sense)
        const auto& rhs = model.get_rhs();
        const auto& sense = model.get_sense();
        for (int i = 0; i < nRows; ++i) {
            if (sense[i] == 'L') {
                model_.setRowUpper(i, rhs[i]);
            } else if (sense[i] == 'G') {
                model_.setRowLower(i, rhs[i]);
            } else { // 'E'
                model_.setRowBounds(i, rhs[i], rhs[i]);
            }
        }
        
        // Set constraint matrix
        const auto& M = model.get_M();
        // ... (convert to CLP's sparse format)
    }
    
    void CLP_solver::solve() {
        model_.primal();  // Use primal simplex
        lpstat_ = model_.status();
    }
    
    void CLP_solver::get_vars(double *x) const {
        const double *sol = model_.primalColumnSolution();
        std::copy(sol, sol + n_col_, x);
    }
    
    void CLP_solver::get_dual_vars(double *pi) const {
        const double *dual = model_.dualRowSolution();
        std::copy(dual, dual + n_row_, pi);
    }
    
    double CLP_solver::get_obj() const {
        return model_.objectiveValue();
    }
    
    // ... implement remaining methods ...
}
```

### Step 3: Update CMakeLists.txt

```cmake
project(util)

# Option to choose solver
option(USE_CPLEX "Use CPLEX solver" OFF)
option(USE_CLP "Use CLP solver" ON)

file(GLOB SOURCES 
    "src/LP_solver.cpp"
    "src/CPX_model_structure.cpp"
    "src/model_description.cpp"
)

if(USE_CPLEX)
    list(APPEND SOURCES "src/CPX_solver.cpp")
    set(CPX_PATH /path/to/cplex)
    # ... CPLEX configuration ...
    target_link_libraries(${PROJECT_NAME} ${ILO_LIBRARY} ${CPX_LIBRARY})
elseif(USE_CLP)
    list(APPEND SOURCES "src/CLP_solver.cpp")
    find_package(Clp REQUIRED)
    target_link_libraries(${PROJECT_NAME} Clp::Clp)
endif()

add_library(${PROJECT_NAME} ${SOURCES})
```

### Step 4: Update Dependent Code

Use conditional compilation or factory pattern:

**Option 1: Conditional Compilation**
```cpp
#ifdef USE_CPLEX
    #include "CPX_solver.hpp"
    using Solver = GOMA::CPX_solver;
#else
    #include "CLP_solver.hpp"
    using Solver = GOMA::CLP_solver;
#endif

Solver solver(model, tol);
solver.solve();
```

**Option 2: Factory Pattern (Recommended)**
```cpp
std::unique_ptr<GOMA::LP_solver> create_solver(
    const GOMA::model_description& model, double tol) {
#ifdef USE_CPLEX
    return std::make_unique<GOMA::CPX_solver>(model, tol);
#else
    return std::make_unique<GOMA::CLP_solver>(model, tol);
#endif
}

// Usage
auto solver = create_solver(model, 1e-6);
solver->solve();
```

## Build Instructions

### With CPLEX (Current)

```bash
# Set CPLEX path in CMakeLists.txt
# Then build
mkdir build && cd build
cmake .. -DUSE_CPLEX=ON
make
```

### With CLP (After Migration)

```bash
# Install CLP first
sudo apt-get install coinor-libclp-dev

# Build
mkdir build && cd build
cmake .. -DUSE_CLP=ON
make
```

## API Reference Summary

### Matrix Operations
- `matrix<T>(m, n)` - Constructor
- `M(i, j)` - 1-based element access
- `M[k]` - 0-based raw access
- `resize(m, n)` - Resize matrix
- `transpose(M_out)` - Compute transpose

### Model Building
- `set_n_col(n)`, `set_n_row(m)` - Set dimensions
- `set_obj(coef)` - Set objective coefficients
- `set_M(matrix)` - Set constraint matrix
- `set_rhs(values)` - Set right-hand sides
- `set_sense(chars)` - Set constraint types

### Solving
- `solve()` - Solve LP/MIP
- `get_vars(x)` - Get primal solution
- `get_dual_vars(pi)` - Get dual solution
- `get_obj()` - Get objective value
- `get_lp_stat()` - Get solver status

### Model Modification
- `set_obj(coef, idx, n)` - Change objective
- `set_rhs(cnt, idx, val)` - Change RHS
- `add_cut(...)` - Add constraint
- `del_rows(begin, end)` - Remove constraints

## Performance Notes

### CPLEX Performance
- Excellent for large-scale problems (millions of variables)
- State-of-the-art presolve and cutting planes
- Parallel MIP solving
- Typical solve times: 0.001s - 1000s depending on problem

### CLP Performance
- Good for medium-scale LPs (up to ~100,000 variables)
- Simplex and barrier methods
- Less advanced than CPLEX but suitable for most applications
- Open-source and freely available

### Recommendations
- **Small to medium LP (<100k vars)**: CLP is sufficient
- **Large LP or complex MIP**: Consider commercial solvers
- **Open-source requirement**: Use CLP or HiGHS

## Integration with CTSP Project

This library is used throughout the CTSP_scheduler project:

- **sync_checker_solver**: Uses `LP_solver` interface for constraint verification
- **sync_checker**: Builds LP models using `model_description`
- **CTSP optimization**: Solves routing problems with synchronization

## Dependencies

### Current (CPLEX)
- IBM ILOG CPLEX Optimization Studio 22.1 or later
- C++ compiler with C++14 support
- CMake 3.10 or later

### After CLP Migration
- COIN-OR CLP library
- COIN-OR OSI (Open Solver Interface)
- C++ compiler with C++14 support
- CMake 3.10 or later

## License Considerations

- **CPLEX**: Commercial license required (academic licenses available)
- **CLP**: Eclipse Public License (EPL) - open-source friendly
- **GLPK**: GNU GPL - requires GPL for derivative works
- **HiGHS**: MIT License - very permissive

## See Also

- `sync_checker_solver`: Uses this library for LP solving
- `sync_checker`: Builds synchronization constraint models
- COIN-OR CLP documentation: https://github.com/coin-or/Clp
- CPLEX documentation: https://www.ibm.com/docs/en/icos

---

**License:** See repository root LICENSE file  
**Author:** Universidad de La Laguna  
**Project:** CTSP_scheduler - Consistent TSP Scheduling System
