# sync_path_finder

## Violated Cycle Detection for Temporal Consistency Constraints

This module implements the **separation oracle** for path elimination constraints in CTSP branch-and-cut algorithms. It detects violated cycles in partial solutions by analyzing the combinatorial structure of the dual problem, as described in Section 5 of Riera-Ledesma et al. (2025).

---

## Purpose

When a partial solution (fractional or integer) violates temporal consistency constraints, the separation oracle must:

1. **Identify violated cycles** in the support graph constructed from the LP solution
2. **Enumerate all simple cycles** through active synchronization arcs
3. **Convert cycles to cutting planes** (path elimination constraints)

This module provides the core cycle detection algorithm that powers the dual-driven separation mechanism.

---

## Algorithm Overview

### Support Graph Construction

Given an LP solution with variables:

- **α (alpha)**: Routing arc selection (0 ≤ α ≤ 1)
- **β (beta)**: Needed for another variation of the CTSP
- **γ (gamma)**: Synchronization arc activation (0 ≤ γ ≤ 1)

The support graph **G = (V, E)** is constructed as:

- **Vertices V**: All operations + depots (n_operations + 2 nodes)
- **Edges E**:
  - Routing arcs (i,j) where α[i,j] > tolerance
  - Sync arcs (i,j) where γ[i,j] > tolerance

**Tolerance**: 1E-3 (variables below this threshold are considered inactive)

### Cycle Detection Algorithm

```text
For each active sync arc (i, j) where γ[i,j] > tol:
  1. Find all simple paths from vertex i to vertex j using DFS
  2. For each path P = [i, v₁, v₂, ..., vₖ, j]:
     a. Convert vertex sequence to arc sequence
     b. Close the cycle by adding reverse arc (j, i)
     c. Store cycle as vector of arc indices
  3. Remove duplicate cycles (same routing arc set)
```

### Arc Indexing

Cycles are represented as vectors of **global arc indices**:

- **Routing arcs**: indices [0, n_routing_arcs)
- **Sync arcs**: indices [n_routing_arcs, n_routing_arcs + n_sync_arcs)

Example cycle: `[3, 15, 42, 107]` means routing arcs 3, 15, 42 and sync arc (107 - n_routing_arcs).

---

## Key Components

### `path_finder` Class

Main class implementing cycle detection.

#### Constructor

```cpp
path_finder(const sync_model_a_builder &builder)
```

**Initializes from model builder:**

- Routing and sync arc structures
- Operation/depot counts
- Arc maps for O(1) index lookup
- Travel times for arc weights

#### Main Method

```cpp
void find_paths(
    const vector<double> &alpha_v,    // Routing arc variables
    const vector<double> &beta_v,     // Timing variables
    const vector<double> &gamma_v,    // Sync arc variables
    vector<vector<int>> &cycles       // Output: detected cycles
)
```

**Returns:** Vector of cycles, where each cycle is a vector of arc indices.

---

## Implementation Details

### 1. Support Graph Update

```cpp
void update_support_graph_(
    const vector<double> &alpha_v,
    const vector<double> &beta_v,
    const vector<double> &gamma_v,
    vector<pair<int,int>> &active_sync_arcs
)
```

**Process:**

1. Clear existing support graph
2. Add routing arcs where α[i,j] > tolerance
3. Add sync arcs where γ[i,j] > tolerance
4. Collect active sync arcs for enumeration

**Active sync arc filtering:**

- Non-depot operations: always include if γ > tol
- Depot operations: include only if both depots are active in routing

### 2. Path Enumeration

```cpp
void find_full_paths_(
    const vector<double> &alpha_v,
    const vector<double> &beta_v,
    const vector<double> &gamma_v,
    const vector<pair<int,int>> &active_sync_arcs,
    vector<vector<int>> &cycles
)
```

**For each active sync arc (i,j):**

1. Use DFS (via `search_graph::find_all_paths()`) to find all simple paths i→j
2. Convert each vertex sequence to arc sequence
3. Close cycle with reverse arc (j,i)

### 3. Duplicate Removal

```cpp
void remove_repeated_cycles_(vector<vector<int>> &cycles) const
```

**Deduplication criteria:** Two cycles are duplicates if they contain the **same routing arcs** (ignoring sync arcs and order).

**Algorithm:**

1. Convert each cycle to boolean vector of routing arcs
2. Compare all pairs (O(n²×m) complexity - noted as inefficient in code comments)
3. Remove duplicates

---

## Output Format

### Cycle Representation

Each cycle is a `vector<int>` of arc indices:

```cpp
vector<vector<int>> cycles = {
    {5, 12, 23, 150},      // Cycle 1: routing arcs 5,12,23 + sync arc 150
    {7, 18, 45, 67, 152},  // Cycle 2: routing arcs 7,18,45,67 + sync arc 152
    ...
};
```

### Interpretation

For arc index `idx`:

- If `idx < n_routing_arcs`: routing arc `idx`
- If `idx >= n_routing_arcs`: sync arc `(idx - n_routing_arcs)`

To get arc endpoints, use the arc maps provided by `sync_model_a_builder`.

---

## Usage Example

```cpp
#include "path_finder.hpp"
#include "sync_model_a_builder.hpp"

// 1. Build CTSP model
sync_model_a_builder builder;
builder.load_instance("instance.contsp");
builder.load_solution("solution.sol");

// 2. Create path finder
path_finder finder(builder);

// 3. Get LP solution variables
vector<double> alpha = ...; // Routing arc values
vector<double> beta = ...;  // Timing values
vector<double> gamma = ...; // Sync arc values

// 4. Detect violated cycles
vector<vector<int>> cycles;
finder.find_paths(alpha, beta, gamma, cycles);

// 5. Process cycles
cout << "Found " << cycles.size() << " violated cycles:" << endl;
for (const auto& cycle : cycles) {
    cout << "Cycle arcs: ";
    for (int arc_idx : cycle) {
        cout << arc_idx << " ";
    }
    cout << endl;
}
```

---

## Integration with Branch-and-Cut

### Workflow in CTSP Solver

```text
1. Solve LP relaxation
          ↓
2. Extract LP solution (α, β, γ)
          ↓
3. path_finder::find_paths() → detect violated cycles
          ↓
4. Convert cycles to path elimination cuts
          ↓
5. Add cuts to LP and resolve
          ↓
6. Repeat until no violations or integer solution found
```

### Cut Generation

Each detected cycle corresponds to a **path elimination constraint**:

```text
∑ α[i,j] ≤ |cycle| - 1    for all (i,j) in routing arcs of cycle
(i,j)∈cycle
```

This inequality forbids the exact set of routing arcs that creates the temporal inconsistency.

---

## Performance Considerations

### Complexity

- **Support graph update**: O(|A| + |S|) where A = routing arcs, S = sync arcs
- **Path enumeration**: Exponential in worst case (all simple paths)
- **Duplicate removal**: O(n²×m) where n = cycle count, m = routing arc count

### Optimization Opportunities

As noted in code comments, duplicate removal is inefficient. Potential improvements:

1. **Hash-based deduplication**: Use set of routing arc signatures
2. **Early termination**: Stop DFS when path length exceeds threshold
3. **Incremental update**: Only update affected graph regions between LP iterations

### Practical Performance

In typical CTSP instances:

- Support graphs are sparse (few active arcs in fractional solutions)
- Most sync arcs connect nearby operations (limited path length)
- Cycle count is manageable (10-100 cycles per separation call)

---

## Dependencies

### Internal Dependencies

- **`sync_model_a_builder`**: Provides arc structures and maps
- **`graph.hpp`** (`GOMA::search_graph`): DFS path enumeration utilities

### External Dependencies

- Standard Library: `<vector>`, `<utility>`, `<algorithm>`

---

## References

**Riera-Ledesma, J., Rodríguez-Martín, I., & Hernández-Pérez, H.** (2025)  
*Dual-driven path elimination for vehicle routing with idle times and arrival-time consistency*  
Computers & Operations Research, 107326  
[https://doi.org/10.1016/j.cor.2025.107326](https://doi.org/10.1016/j.cor.2025.107326)

**Section 5** describes the dual problem interpretation and separation oracle implementation that this module realizes.

---

## Future Enhancements

### Planned Improvements

1. **Strengthening procedures**: Implement the lifting techniques from Section 5.3 of the paper
2. **Cycle filtering**: Prioritize "most violated" cycles before adding all cuts
3. **Parallel enumeration**: Detect cycles for multiple sync arcs in parallel
4. **Graph visualization**: Export support graph and detected cycles in DOT format

### Extension Points

The `path_finder` class is designed for extension:

```cpp
class enhanced_path_finder : public path_finder {
public:
    // Override to add custom cycle filtering
    void find_paths(...) override;
    
    // Add strengthening
    void strengthen_cycles(vector<vector<int>>& cycles);
};
```

---

## Notes

- **Tolerance value**: The 1E-3 threshold is chosen to balance numerical stability with sensitivity to fractional values
- **Depot handling**: Special logic for sync arcs involving depots prevents spurious cycles in multi-depot instances
- **Arc direction**: Cycles must be closed with the **reverse** sync arc to maintain consistency with the dual formulation

---

**Module Status**: Core separation oracle (stable)  
**Last Updated**: November 2025  
**Maintainer**: Jorge Riera-Ledesma
