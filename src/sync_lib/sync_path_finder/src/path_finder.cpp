/**
 * @file path_finder.cpp
 * @brief Implementation of cycle detection for CTSP synchronization constraints
 * 
 * Implements DFS-based cycle enumeration in support graphs constructed from
 * LP solutions. Cycles indicate violated synchronization constraints in the
 * branch-and-cut algorithm for CTSP solving.
 * 
 * Key algorithms:
 * - Support graph construction from LP solution (active arcs)
 * - DFS path enumeration between sync arc endpoints
 * - Cycle closure and duplicate removal
 * - Vertex sequence to arc sequence conversion
 */

#include "path_finder.hpp"

#include <algorithm>
#include <cassert>
#include <iomanip>
#include <vector>
#include <cmath>

#include <bits/stdc++.h>

namespace SYNC_LIB
{

    /**
     * Constructor: Initialize path finder from model builder
     * 
     * Extracts all necessary references from builder:
     * - Routing and sync arc structures (triplets)
     * - Operation and depot counts
     * - Arc index maps for O(1) lookup
     * - Arc times and names for display
     * 
     * Initializes support graph with n_operations + 2 vertices:
     * - Vertices 0 to n_depots-1: depots
     * - Vertices n_depots to n_operations+1: operations
     */
    path_finder::path_finder(const sync_model_a_builder &builder) : tol_(1E-3),
                                                                    routing_arcs_(builder.get_routing_arcs()),
                                                                    sync_arcs_(builder.get_sync_arcs()),
                                                                    n_operations_(builder.get_n_operations()),
                                                                    n_depots_(builder.get_n_depots()),
                                                                    routing_arc_map_(builder.get_routing_arcs_pair_map()),
                                                                    sync_arc_map_(builder.get_sync_arcs_pair_map()),
                                                                    routing_arc_times_(builder.get_routing_arc_times()),
                                                                    sync_arc_times_(builder.get_sync_arc_times()),
                                                                    routing_arc_names_(builder.get_routing_arc_names()),
                                                                    sync_arc_names_(builder.get_sync_arc_names()),
                                                                    n_routing_arcs_(builder.get_n_routing_arcs()),
                                                                    support_graph_(n_operations_ + 2)
    {
    }

    /**
     * Destructor
     */
    path_finder::~path_finder(void)
    {
    }

    /**
     * Main cycle detection algorithm
     * 
     * Two-phase process:
     * 1. Update support graph with active arcs from LP solution
     * 2. Find all cycles through active sync arcs using DFS
     * 
     * Support graph contains:
     * - Edges for routing arcs with alpha > tolerance
     * - Edges for sync arcs with gamma > tolerance
     * 
     * For each active sync arc (i,j), finds all simple paths from i to j,
     * then closes each path with arc (j,i) to form cycle.
     */
    void path_finder::find_paths(const vector<double> &alpha_v,
                                 const vector<double> &beta_v,
                                 const vector<double> &gamma_v,
                                 vector<vector<int>> &cycles)
    {
        vector<pair<int, int>> active_sync_arcs;

        update_support_graph_(alpha_v, beta_v, gamma_v, active_sync_arcs);

        find_full_paths_(alpha_v, beta_v, gamma_v, active_sync_arcs, cycles);
    }

    /**
     * Write single path using arc names
     * 
     * Converts arc indices to human-readable names:
     * - Indices [0, n_routing_arcs): routing arc names
     * - Indices [n_routing_arcs, ...): sync arc names (offset removed)
     * 
     * Outputs space-separated arc names followed by newline.
     */
    ostream &path_finder::write_path_(ostream &os, const vector<int> &alpha_beta_path) const
    {
        const size_t n_routing_arcs{routing_arcs_.size()};

        for (int inx : alpha_beta_path)
        {
            if (inx < (int)n_routing_arcs)
            {
                os << routing_arc_names_[inx] << " ";
            }
            else
            {
                os << sync_arc_names_[inx - (int)n_routing_arcs] << " ";
            }
        }

        os << endl;

        return os;
    }

    /**
     * Find closing arc index for cycle formation
     * 
     * Given sync arc (i,j), finds reverse arc (j,i) to close the cycle.
     * 
     * Steps:
     * 1. Create inverse arc pair (j,i)
     * 2. Lookup in sync_arc_map to get sync arc index
     * 3. Offset by n_routing_arcs to get global arc index
     * 
     * Example: arc (5,7) -> inverse (7,5) -> sync_arc_index=3 -> return 3+n_routing_arcs
     */
    int path_finder::closing_arc_(const pair<int, int> &arc) const
    {
        const pair<int, int> inv_arc{pair<int, int>(arc.second, arc.first)};

        const int inv_beta_arc{sync_arc_map_.at(inv_arc)};
        assert(inv_beta_arc >= 0);

        const int closing_arc_inx{inv_beta_arc + (int)n_routing_arcs_};

        return closing_arc_inx;
    }

    /**
     * Remove duplicate cycles (inefficient but functional)
     * 
     * Duplicates defined as: cycles with identical routing arc sets.
     * Ignores sync arcs and arc order for comparison.
     * 
     * Algorithm:
     * 1. Convert each cycle to boolean vector of routing arcs (O(n×m))
     * 2. Compare all pairs using boolean vector equality (O(n²×m))
     * 3. Mark duplicates for removal
     * 4. Rebuild cycle list without duplicates (O(n))
     * 
     * Time complexity: O(n²×m) where n = cycles, m = routing arcs
     * Space complexity: O(n×m) for boolean vectors
     * 
     * Note: Self-deprecating comment acknowledges inefficiency.
     * Could be optimized with hashing or canonical cycle representation.
     */
    // Possible, the most inefficient function ever written ;)

    void path_finder::remove_repeated_cycles_(vector<vector<int>> &cycles) const
    {
        const size_t n_cycles{cycles.size()};
        const size_t n_routing_arcs{routing_arcs_.size()};

        // Early exit: 0 or 1 cycle cannot have duplicates
        if (n_cycles <= 1)
            return;

        const vector<vector<int>> old_cycles(cycles);

        cycles.clear();
        vector<bool> redundant_cycles(n_cycles, false);

        {
            // Phase 1: Convert cycles to boolean routing arc vectors
            vector<vector<bool>> ordered_cycles(old_cycles.size());

            for (size_t i{0}; i < n_cycles; i++)
            {
                const vector<int> &old_cycle{old_cycles[i]};

                // Create boolean vector: ordered_cycle_i[k] = true if routing arc k in cycle
                vector<bool> &ordered_cycle_i{ordered_cycles[i]};
                ordered_cycle_i.resize(n_routing_arcs, false);

                const size_t n_old_arcs{old_cycles[i].size()};

                for (size_t j{0}; j < n_old_arcs; j++)
                {
                    const int c_arc{old_cycle[j]};

                    // Only mark routing arcs (indices < n_routing_arcs)
                    if (c_arc < (int)n_routing_arcs)
                    {
                        ordered_cycle_i[c_arc] = true;
                    }
                }
            }

            // Phase 2: Find duplicates by comparing boolean vectors
            for (size_t i{0}; i < n_cycles - 1; i++)
            {

                const vector<bool> &ordered_cycle_i{ordered_cycles[i]};

                // Skip if cycle i already marked redundant
                if (redundant_cycles[i])
                    continue;

                // Compare cycle i with all subsequent cycles
                for (size_t j{i + 1}; j < n_cycles; j++)
                {

                    // Skip if cycle i became redundant during inner loop
                    if (redundant_cycles[i])
                        continue;

                    const vector<bool> &ordered_cycle_j{ordered_cycles[j]};

                    // If boolean vectors equal, cycle j is duplicate
                    if (ordered_cycle_i == ordered_cycle_j)
                    {
                        redundant_cycles[j] = true;
                    }
                }
            }
        }

        // Phase 3: Rebuild cycles vector without duplicates
        for (size_t i{0}; i < n_cycles; i++)
        {
            if (!redundant_cycles[i])
            {
                cycles.push_back(old_cycles[i]);
            }
        }
    }

    /**
     * Enumerate all cycles through active synchronization arcs
     * 
     * For each active sync arc (i,j):
     * 1. Run DFS from i to j to find all simple paths
     * 2. Convert each vertex sequence to arc index sequence
     * 3. Close cycle by appending reverse sync arc (j,i)
     * 4. Add cycle to results
     * 
     * Finally removes duplicate cycles (same routing arc set).
     * 
     * beta_v parameter currently unused (future extension for time-aware paths).
     */
    void path_finder::find_full_paths_(const vector<double> &alpha_v,
                                       const vector<double> &beta_v,
                                       const vector<double> &gamma_v,
                                       const vector<pair<int, int>> &active_sync_arcs,
                                       vector<vector<int>> &cycles)
    {

        const bool beta_empty{(beta_v.size() == 0)};

        vector<vector<int>> c_sequences;  // Vertex sequences (paths)

        vector<int> type;
        vector<int> cycle;  // Arc sequence for current cycle

        // For each active sync arc, find all paths and close to form cycles
        for (const pair<int, int> &arc : active_sync_arcs)
        {
            c_sequences.clear();

            // DFS from arc.first to arc.second to find all simple paths
            support_graph_.DFS(arc.first, arc.second, c_sequences);

            if (c_sequences.size() > 0)
            {
                // Convert each vertex sequence to arc sequence and close cycle
                for (const vector<int> &c_sequence : c_sequences)
                {
                    // Convert vertex sequence to arc indices
                    sequence_2_path_(c_sequence, alpha_v, gamma_v, cycle);

                    // Close cycle with reverse sync arc
                    const int closing_arc{closing_arc_(arc)};

                    cycle.push_back(closing_arc);

                    cycles.push_back(cycle);
                }
            }
            else
            {
                cout << "No path found" << endl;
            }
        }

        // Remove cycles with identical routing arc sets
        remove_repeated_cycles_(cycles);
    }

    /**
     * Convert vertex sequence to arc index sequence
     * 
     * Takes path through operations (vertex IDs) and converts to
     * sequence of arc indices (routing + sync arcs).
     * 
     * For each consecutive pair (s,t) in sequence:
     * 1. Check routing_arc_map for routing arc (s,t)
     * 2. If not found, check sync_arc_map for sync arc (s,t)
     * 3. Sync arc indices offset by n_routing_arcs
     * 
     * Parameters alpha_v and beta_v currently unused (reserved for future).
     */
    void path_finder::sequence_2_path_(const vector<int> &sequence,
                                       const vector<double> &alpha_v,
                                       const vector<double> &beta_v,
                                       vector<int> &cycle) const
    {
        cycle.clear();

        const size_t n_sequence{sequence.size()};
        const size_t n_routing_arcs{routing_arcs_.size()};

        // Convert each consecutive vertex pair to arc index
        for (size_t i{0}; i < n_sequence - 1; i++)
        {
            const int s = sequence[i];
            const int t = sequence[(i + 1) % n_sequence];

            // First try routing arc
            const int routing_arc = routing_arc_map_.at(s, t);

            if (routing_arc >= 0)
            {
                cycle.push_back(routing_arc);
            }
            else
            {
                // Not a routing arc, must be sync arc
                const int sync_arc = sync_arc_map_.at(s, t);

                assert(sync_arc >= 0);

                if (sync_arc >= 0)
                {
                    // Offset sync arc index by n_routing_arcs
                    cycle.push_back(sync_arc + n_routing_arcs);
                }
            }
        }
    }

    /**
     * Build support graph from LP solution
     * 
     * Constructs directed graph with edges corresponding to active arcs:
     * - Routing arcs with alpha > tolerance
     * - Sync arcs with gamma > tolerance
     * 
     * Also identifies active sync arcs for cycle detection:
     * - If arc connects non-depot operations: always active
     * - If arc connects depots: active only if both depots used in routing
     * 
     * This ensures cycles only form through meaningful synchronizations.
     * 
     * Note: Inverted arc direction stored in active_sync_arcs (j,i) instead of (i,j)
     * for compatibility with DFS cycle closure.
     */
    void path_finder::update_support_graph_(const vector<double> &alpha_v,
                                            const vector<double> &beta_v,
                                            const vector<double> &gamma_v,
                                            vector<pair<int, int>> &active_sync_arcs)
    {
        support_graph_.clear();
        active_sync_arcs.clear();

        const size_t n_routing_arcs{routing_arcs_.size()};
        const size_t n_sync_arcs{sync_arcs_.size()};

        set<int> active_depot_set;  // Depots with at least one active routing arc

        // Add routing arcs to support graph (alpha > tolerance)
        for (size_t i{0}; i < n_routing_arcs; i++)
        {
            if (alpha_v[i] > tol_)
            {

                const triplet &arc{routing_arcs_[i]};
                support_graph_.add_arc(arc.i_, arc.j_);

                // Track which depots are active (used in routes)
                active_depot_set.insert(arc.k_i_);
            }
        }

        // Add sync arcs to support graph (gamma > tolerance)
        for (size_t i{0}; i < n_sync_arcs; i++)
        {
            if (gamma_v[i] > tol_)
            {
                const triplet &arc{sync_arcs_[i]};
                support_graph_.add_arc(arc.i_, arc.j_);

                // Determine if sync arc should be considered for cycle detection
                if ((arc.i_ > n_depots_) || (arc.j_ > n_depots_))
                {
                    // At least one endpoint is non-depot operation: always consider
                    // Note: Stores inverted arc (j,i) for cycle closure
                    active_sync_arcs.push_back(pair<int, int>(arc.j_, arc.i_));
                }
                else if ((active_depot_set.find(arc.i_) != active_depot_set.end()) &&
                         (active_depot_set.find(arc.j_) != active_depot_set.end()))
                {
                    // Both endpoints are active depots: consider for cycle detection
                    active_sync_arcs.push_back(pair<int, int>(arc.i_, arc.j_));
                }
            }
        }
    }
}