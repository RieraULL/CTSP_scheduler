# sync_checker - Synchronization Constraint Checking for CTSP

## Overview

The `sync_checker` library provides LP-based verification of synchronization constraints in CTSP (Consistent Traveling Salesman Problem) solutions. It determines whether a given routing solution can be scheduled such that vehicles arrive at common customers in a synchronized manner, respecting temporal consistency requirements.

## Problem Context

In CTSP, multiple vehicles visit overlapping sets of customers. **Synchronization constraints** require that:

- **CTSP1** (Time Window Synchronization): Vehicles visiting the same customer must arrive within a specified time window of each other
- **CTSP2** (Exact Synchronization): Vehicles must arrive simultaneously at common customers

Given a routing solution (which arcs are traversed), the synchronization checker determines if valid arrival times exist that satisfy these constraints.

## Key Components

### 1. Core Checker (`ctsp_sync_checker`)

The base synchronization checker that:

- **Builds LP Model**: Creates a primal LP whose variables are arrival times (α), route durations (β), and synchronization slacks (γ)
- **Checks Feasibility**: Determines if synchronization is achievable for a given routing
- **Extracts Duals**: Provides dual variables for cut generation when infeasible
- **Parametric Solving**: Efficiently updates LP based on routing solution

```cpp
#include "ctsp_sync_checker.hpp"

// Create checker from model builder
ctsp_sync_checker checker(builder, primal_model, tolerance);

// Check if routing solution x satisfies synchronization
vector<double> alpha, beta, gamma;
if (!checker.is_feasible(x, alpha, beta, gamma)) {
    // Infeasible - use dual variables to generate cut
    // alpha: arrival time multipliers
    // beta: duration multipliers  
    // gamma: synchronization multipliers
}
```

### 2. Primal Models

#### `ctsp_primal_model`

Base primal LP model with three constraint types:

**Alpha Constraints** (Arrival Time Consistency):

$$\alpha_j \geq \alpha_i + t_{ij} + s_i \quad \forall (i,j) \in A_{routing}$$

where:
- $\alpha_i$: arrival time at operation $i$
- $t_{ij}$: travel time from $i$ to $j$
- $s_i$: service time at operation $i$

**Beta Constraints** (Route Duration):

$$\beta_k \geq \alpha_i \quad \forall i \in \text{operations of vehicle } k$$

$$\beta_k \leq T_{max}$$

where:
- $\beta_k$: total duration of route $k$
- $T_{max}$: maximum allowed route duration

**Gamma Constraints** (Synchronization Coupling):

- **CTSP1**: $|\alpha_i - \alpha_j| \leq w_c + \gamma_c \quad \forall i,j \text{ visiting customer } c$
- **CTSP2**: $\alpha_i = \alpha_j \quad \forall i,j \text{ visiting customer } c$

where $w_c$ is the allowed time window width.

#### `ctsp_lb_primal_model`

Specialized model for lower bound computation:

- Relaxes certain constraints to obtain valid bounds
- Used in Lagrangian relaxation schemes
- Supports branch-and-bound bounding procedures

### 3. Lower Bound Checker (`ctsp_lb_sync_checker`)

Extends base checker for bound computation:

```cpp
#include "ctsp_lb_sync_checker.hpp"

ctsp_lb_sync_checker lb_checker(builder, tolerance);

// Use in branch-and-bound for bounding
if (lb_checker.is_feasible(relaxed_solution)) {
    double bound = lb_checker.get_obj();
    // Use bound to prune search tree
}
```

### 4. Iterative Checker Template (`sync_iterative_checker`)

Template wrapper providing convenient interfaces:

```cpp
#include "sync_iterative_checker.hpp"

// Wrap any checker type
sync_iterative_checker<ctsp_sync_checker> iter_checker(builder, tol);

// Convenient one-call interface
if (!iter_checker.is_feasible(x, alpha, beta, gamma)) {
    // Automatically extracts duals if infeasible
    generate_cut(alpha, beta, gamma);
}

// Alternative: extract slacks if feasible
vector<double> slack;
if (iter_checker.is_feasible(x, slack)) {
    // Analyze constraint tightness
}
```

## How It Works

### Feasibility Checking Process

1. **Input**: Routing solution $x$ (which arcs are used)
2. **LP Construction**: Build primal model with α, β, γ variables
3. **Parameterization**: Update LP coefficients based on $x$
4. **Solve**: Run LP solver (CPLEX/CLP)
5. **Result**:
   - **Feasible**: Synchronization achievable → extract arrival times
   - **Infeasible**: Synchronization impossible → extract duals for cut

### Dual Variables and Cut Generation

When the LP is infeasible, the dual variables provide a **proof of infeasibility** that can be converted into a valid inequality (cut):

```
Sum over arcs: (dual_alpha[arc] * x[arc]) 
    + Sum over vehicles: (dual_beta[vehicle] * route_used[vehicle])
    + Sum over customers: (dual_gamma[customer] * sync_required[customer])
    >= violation_threshold
```

This cut eliminates the current infeasible routing without removing any feasible solutions.

## Usage in Branch-and-Cut

```cpp
// 1. Solve routing relaxation (ignoring some sync constraints)
solve_routing_relaxation();

// 2. Check synchronization feasibility
sync_iterative_checker<ctsp_sync_checker> checker(builder, tolerance);

vector<double> alpha, beta, gamma;
while (!optimal) {
    vector<double> current_solution = get_current_solution();
    
    if (!checker.is_feasible(current_solution, alpha, beta, gamma)) {
        // 3. Generate and add violated cut from duals
        Cut violated_cut = generate_synchronization_cut(alpha, beta, gamma);
        add_cut(violated_cut);
        
        // 4. Re-solve
        resolve_with_new_cuts();
    } else {
        // Solution is feasible w.r.t. synchronization
        if (is_integer(current_solution)) {
            optimal = true;
        } else {
            branch();
        }
    }
}
```

## API Reference

### `ctsp_sync_checker`

| Method | Purpose |
|--------|---------|
| `is_feasible_(x)` | Check if routing x satisfies synchronization |
| `is_feasible(x, α, β, γ)` | Check feasibility and extract duals |
| `get_alpha_beta_gamma(α, β, γ)` | Get dual variables from last solve |
| `get_s(s)` | Get slack variables if feasible |

### `ctsp_primal_model`

| Method | Purpose |
|--------|---------|
| Constructor | Build primal LP model from builder |
| `get_n_beta_constraints()` | Get count of route duration constraints |

### `sync_iterative_checker<T>`

| Method | Purpose |
|--------|---------|
| `is_feasible(x, α, β, γ)` | Check and extract duals if infeasible |
| `is_feasible(x, s)` | Check and extract slacks if feasible |

## Dependencies

- **sync_model_a**: CTSP model building and arc representations
- **sync_checker_solver**: LP solver interface (CPLEX or CLP)
- **gomautil**: Matrix operations and utility functions

## Building

Part of the main CTSP_scheduler project:

```bash
cd CTSP_scheduler/build
cmake ..
make sync_checker
```

## Mathematical Formulation

The primal LP for synchronization checking:

**Variables:**

- $\alpha_i \geq 0$: arrival time at operation $i$
- $\beta_k \geq 0$: duration of route $k$
- $\gamma_c \geq 0$: synchronization slack for customer $c$

**Constraints:**

```
α-constraints: α_j ≥ α_i + (t_ij + s_i) * x_ij    ∀(i,j)
β-constraints: β_k ≥ α_i                           ∀i ∈ route_k
               β_k ≤ T_max                         ∀k
γ-constraints: |α_i - α_j| ≤ w_c + γ_c            ∀i,j visiting c (CTSP1)
               α_i = α_j                           ∀i,j visiting c (CTSP2)
```

**Objective:**

Minimize $\sum_c \gamma_c$ (penalize synchronization violations)

**Feasibility:**

- If optimal objective = 0: synchronization achievable
- If infeasible: routing cannot be synchronized

## Performance Considerations

- **Sparse Updates**: Only modified coefficients are updated between solves
- **Warm Starts**: LP basis is preserved across iterations when possible
- **Constraint Counting**: Different models (base, lower bound) use different constraint sets

## References

The synchronization checking approach is described in:

Jorge Riera-Ledesma, Inmaculada Rodríguez-Martín, Hipólito Hernández–Pérez,
"Dual-driven path elimination for vehicle routing with idle times and arrival-time consistency",
Computers & Operations Research, 2025, 107326,
ISSN 0305-0548,
<https://doi.org/10.1016/j.cor.2025.107326>

## License

See main project LICENSE file.
