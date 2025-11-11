# sync_checker_solver - LP Solver Wrapper for CTSP

## Overview

The `sync_checker_solver` library provides a high-level C++ wrapper around LP solvers for solving linear programming (LP) problems that arise in CTSP (Consistent Traveling Salesman Problem) algorithms.

**Current Implementation**: Uses IBM ILOG CPLEX as the underlying solver.

**Alternative Solvers**: The CPLEX calls can be replaced with calls to open-source alternatives such as [CLP (COIN-OR Linear Programming)](https://github.com/coin-or/Clp), which provides similar functionality without requiring a commercial license. The interface is designed to be solver-agnostic through the PIMPL pattern (see Design Pattern section below).

The library is specifically designed for:

- **Constraint checking**: Verifying feasibility of synchronization constraints
- **Dual variable computation**: Extracting shadow prices for reduced cost calculation
- **Separation algorithms**: Solving LP relaxations to identify violated inequalities
- **Branch-and-cut**: Dynamic constraint generation and model modification

## Key Features

### Simplified CPLEX Interface

The library abstracts away CPLEX's verbose API, providing clean methods for common operations:

```cpp
// Create solver with model
sync_checker_solver solver(model_description, tolerance);

// Solve LP
solver.solve();

// Get solution
double obj_value = solver.get_obj();
solver.get_vars(primal_vars);
solver.get_dual_vars(dual_vars);

// Add cuts dynamically
solver.add_cut(variables, coefficients, rhs, sense, "cut_name");
```

### Dynamic Model Modification

Support for on-the-fly model updates without reconstruction:

- **Change RHS**: `set_rhs()` - Modify constraint right-hand sides
- **Update objective**: `set_obj()` - Change objective coefficients
- **Modify bounds**: `set_bdn()` - Update variable bounds
- **Add constraints**: `add_cut()` - Insert new cuts
- **Change coefficients**: `set_coef()` - Modify constraint matrix entries

### Use Cases

#### 1. Constraint Separation

```cpp
// Solve LP relaxation
solver.solve();

// Extract dual variables
double duals[num_constraints];
solver.get_dual_vars(duals);

// Use duals to compute reduced costs
// Identify and add violated cuts
if (constraint_violated) {
    solver.add_cut(lhs_vars, lhs_coefs, rhs, 'L', "valid_inequality");
    solver.solve();  // Re-optimize
}
```

#### 2. Synchronization Checking

```cpp
// Check if synchronization constraints are satisfied
solver.set_rhs(count, constraint_indices, new_values);
solver.solve();

if (solver.get_lp_stat() == 1) {  // Optimal
    // Constraints are feasible
} else {
    // Infeasible - synchronization cannot be achieved
}
```

#### 3. Branch-and-Cut Algorithm

```cpp
while (!optimality_achieved) {
    solver.solve();
    
    // Check for violated cuts
    vector<Cut> violated_cuts = separation_routine(solver);
    
    for (const auto& cut : violated_cuts) {
        solver.add_cut(cut.vars, cut.coefs, cut.rhs, cut.sense, cut.name);
    }
    
    if (violated_cuts.empty()) {
        optimality_achieved = true;
    }
}
```

## API Reference

### Constructor

```cpp
sync_checker_solver(const model_description &model, const double tol = 1e-6)
```

Creates solver with initial LP model and numerical tolerance.

### Core Methods

| Method | Purpose |
|--------|---------|
| `solve()` | Solve the LP problem |
| `get_obj()` | Get optimal objective value |
| `get_lp_stat()` | Get solution status (1=optimal, 3=infeasible, etc.) |
| `get_vars(double*)` | Retrieve primal variable values |
| `get_dual_vars(double*)` | Retrieve dual variable values (shadow prices) |

### Model Modification

| Method | Purpose |
|--------|---------|
| `add_cut(vars, coefs, rhs, sense, name)` | Add new linear constraint |
| `set_rhs(count, indices, values)` | Modify constraint RHS |
| `set_obj(coefs, indices, size)` | Change objective coefficients |
| `set_bdn(values, sense, indices, size)` | Update variable bounds |
| `set_coef(count, rows, cols, values)` | Modify constraint matrix |

### Utilities

| Method | Purpose |
|--------|---------|
| `get_nz()` | Get number of non-zeros in constraint matrix |
| `write_model(filename)` | Export model to file (.lp, .mps, .sav) |

## Dependencies

### Required

- **LP Solver** (one of the following):
  - **CPLEX** (current implementation): IBM ILOG CPLEX Optimization Studio 12.8 or later
    - Commercial license required
    - Download from [IBM CPLEX website](https://www.ibm.com/products/ilog-cplex-optimization-studio)
  - **CLP** (alternative, open-source): COIN-OR Linear Programming solver
    - Free and open-source (Eclipse Public License)
    - Available at [COIN-OR CLP repository](https://github.com/coin-or/Clp)
    - Can replace CPLEX calls by implementing a `CLP_solver` class following the same `LP_solver` interface

- **gomautil**: Utility library for model descriptions and data structures

- **sync_model_a**: CTSP model building library

### Configuration

#### Using CPLEX (Current Implementation)

Update the CPLEX path in `CMakeLists.txt`:

```cmake
set(CPX_PATH /path/to/your/CPLEX_Studio221)
```

Common installation paths:

- **Linux**: `/opt/ibm/ILOG/CPLEX_Studio221`
- **Windows**: `C:/Program Files/IBM/ILOG/CPLEX_Studio221`
- **macOS**: `/Applications/CPLEX_Studio221`

#### Using CLP (Open-Source Alternative)

To use CLP instead of CPLEX:

1. Install COIN-OR CLP:

   ```bash
   # Ubuntu/Debian
   sudo apt-get install coinor-libclp-dev
   
   # From source
   git clone https://github.com/coin-or/Clp
   cd Clp
   mkdir build && cd build
   cmake ..
   make && sudo make install
   ```

2. Implement a `CLP_solver` class that inherits from `LP_solver` interface
3. Modify `sync_checker_solver.cpp` to instantiate `CLP_solver` instead of `CPX_solver`
4. Update `CMakeLists.txt` to link against CLP libraries instead of CPLEX

The interface remains the same, so no changes are needed in client code.

## Building

This library is typically built as part of the main CTSP_scheduler project:

```bash
cd CTSP_scheduler/build
cmake ..
make sync_checker_solver
```

## Design Pattern

The library uses the **PIMPL (Pointer to Implementation)** pattern:

- **Public interface**: `sync_checker_solver` (solver-agnostic)
- **Private implementation**: `CPX_solver` (CPLEX-specific) or `CLP_solver` (CLP-specific)

This design allows:

- Hiding solver implementation details
- Easy switching between different LP solvers (CPLEX, CLP, Gurobi, HiGHS, etc.)
- Cleaner public API
- Compile-time decoupling from specific solver libraries

## Limitations

- Currently implemented only for CPLEX (CLP implementation requires additional work)
- CPLEX version requires commercial license for production use
- No built-in support for integer programming (MIP) - only LP relaxations

## Future Enhancements

Potential improvements:

- Complete implementation of CLP adapter (open-source alternative to CPLEX)
- Support for additional commercial solvers (Gurobi, SCIP)
- Support for other open-source solvers (HiGHS, GLPK)
- Warm-start capabilities
- Advanced solver features (barrier method, network simplex)
- Callback integration for custom cut generation

## License

See main project LICENSE file.

## References

This library is used in the implementation of algorithms described in:

Jorge Riera-Ledesma, Inmaculada Rodríguez-Martín, Hipólito Hernández–Pérez,
"Dual-driven path elimination for vehicle routing with idle times and arrival-time consistency",
Computers & Operations Research, 2025, 107326,
ISSN 0305-0548,
<https://doi.org/10.1016/j.cor.2025.107326>
