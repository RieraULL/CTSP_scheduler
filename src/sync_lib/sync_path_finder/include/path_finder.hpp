/**
 * @file path_finder.hpp
 * @brief Path finder for cycle detection in CTSP synchronization graphs
 * 
 * Implements cycle enumeration in support graphs constructed from LP solutions.
 * Used to detect violated synchronization constraints by finding cycles in the
 * graph formed by active routing and synchronization arcs.
 * 
 * Key responsibilities:
 * - Build support graph from LP solution (alpha, beta, gamma variables)
 * - Enumerate all simple cycles using DFS on support graph
 * - Convert vertex sequences to arc sequences (routing + sync arcs)
 * - Detect and remove duplicate cycles
 * 
 * Algorithm:
 * 1. Build support graph from active arcs (alpha > tolerance, gamma > tolerance)
 * 2. For each active sync arc (i,j):
 *    a. Find all simple paths from i to j using DFS
 *    b. Close each path with arc (j,i) to form cycle
 * 3. Remove duplicate cycles (same routing arc set)
 * 
 * Used in constraint generation for branch-and-cut CTSP solving.
 */

#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <utility>

#include "sync_model_a_builder.hpp"
#include "graph.hpp"

using namespace std;

namespace SYNC_LIB
{

    /**
     * @class path_finder
     * @brief Cycle detector for CTSP synchronization constraints
     * 
     * Finds cycles in support graph constructed from LP solution to detect
     * violated synchronization constraints. Uses DFS-based path enumeration
     * on graph with active routing and sync arcs.
     * 
     * The support graph has vertices = operations + depots, and edges from:
     * - Routing arcs with alpha > tolerance (selected in solution)
     * - Sync arcs with gamma > tolerance (active synchronizations)
     * 
     * Cycles indicate potential constraint violations where synchronization
     * creates circular dependencies in the schedule.
     */
    class path_finder
    {
    protected:
        const double tol_;  ///< Tolerance for considering variables active (1E-3)

        const vector<triplet> &routing_arcs_;  ///< Reference to routing arcs (i,j,depot)
        const vector<triplet> &sync_arcs_;     ///< Reference to sync arcs (i,j,k)
        size_t n_operations_;                  ///< Number of operations (vertices)
        size_t n_depots_;                      ///< Number of depots

        const pair_map &routing_arc_map_;  ///< Map (i,j) -> routing arc index
        const pair_map &sync_arc_map_;     ///< Map (i,j) -> sync arc index

        const vector<double> &routing_arc_times_;  ///< Travel times for routing arcs
        const vector<double> &sync_arc_times_;     ///< Time lags for sync arcs

        const vector<string> &routing_arc_names_;  ///< Names for routing arcs (display)
        const vector<string> &sync_arc_names_;     ///< Names for sync arcs (display)

        size_t n_routing_arcs_;  ///< Total number of routing arcs

        GOMA::search_graph support_graph_;  ///< DFS graph for cycle detection

    public:
        /**
         * @brief Construct path finder from model builder
         * @param builder CTSP model builder containing arc definitions
         * 
         * Initializes references to routing/sync arcs, operation count,
         * arc maps, times, names, and allocates support graph.
         */
        path_finder(const sync_model_a_builder &builder);

        /**
         * @brief Destructor
         */
        virtual ~path_finder(void);

        /**
         * @brief Find all cycles in support graph from LP solution
         * @param alpha_v Routing arc variables (size = n_routing_arcs)
         * @param beta_v Time variables (currently unused in path finding)
         * @param gamma_v Sync arc variables (size = n_sync_arcs)
         * @param[out] cycles Output vector of cycles (each cycle = vector of arc indices)
         * 
         * Main algorithm:
         * 1. Update support graph with active arcs (alpha > tol, gamma > tol)
         * 2. For each active sync arc, find all paths using DFS
         * 3. Close paths to form cycles
         * 4. Remove duplicate cycles
         * 
         * Each cycle is represented as vector of arc indices:
         * - Indices [0, n_routing_arcs) are routing arcs
         * - Indices [n_routing_arcs, ...) are sync arcs (offset by n_routing_arcs)
         */
        void find_paths(const vector<double> &alpha_v,
                        const vector<double> &beta_v,
                        const vector<double> &gamma_v,
                        vector<vector<int>> &cycles);

        /**
         * @brief Write all paths to output stream
         * @param os Output stream
         * @param alpha_beta_path Vector of paths (each path = vector of arc indices)
         * @return Reference to output stream
         * 
         * Writes each path on separate line using arc names.
         */
        ostream &write_paths(ostream &os, const vector<vector<int>> &alpha_beta_path) const
        {
            for (const vector<int> &path : alpha_beta_path)
            {
                write_path_(os, path);
                os << endl;
            }

            return os;
        }

    protected:
        /**
         * @brief Write single path to output stream using arc names
         * @param os Output stream
         * @param alpha_beta_path Path as vector of arc indices
         * @return Reference to output stream
         */
        ostream &write_path_(ostream &os, const vector<int> &alpha_beta_path) const;

        /**
         * @brief Convert vertex sequence to arc index sequence
         * @param sequence Vertex sequence (path through operations)
         * @param alpha_v Routing arc variables (unused)
         * @param beta_v Time variables (unused)
         * @param[out] cycle Output arc index sequence
         * 
         * For each consecutive vertex pair (s,t), finds corresponding arc:
         * - First checks routing_arc_map for routing arc
         * - If not found, checks sync_arc_map for sync arc
         * - Sync arc indices offset by n_routing_arcs
         */
        void sequence_2_path_(const vector<int> &sequence,
                              const vector<double> &alpha_v,
                              const vector<double> &beta_v,
                              vector<int> &cycle) const;

        /**
         * @brief Enumerate all cycles through active sync arcs
         * @param alpha_v Routing arc variables
         * @param beta_v Time variables (unused)
         * @param gamma_v Sync arc variables
         * @param active_sync_arcs Active sync arcs from support graph update
         * @param[out] cycles Output vector of cycles
         * 
         * For each active sync arc (i,j):
         * 1. Run DFS from i to j to find all paths
         * 2. Convert vertex sequences to arc sequences
         * 3. Close cycle by adding arc (j,i)
         * 4. Remove duplicate cycles
         */
        void find_full_paths_(const vector<double> &alpha_v,
                              const vector<double> &beta_v,
                              const vector<double> &gamma_v,
                              const vector<pair<int, int>> &active_sync_arcs,
                              vector<vector<int>> &cycles);

        /**
         * @brief Find closing arc index for sync arc
         * @param arc Sync arc (i,j)
         * @return Index of reverse sync arc (j,i) offset by n_routing_arcs
         * 
         * Used to close cycles: path from i to j is closed with arc (j,i).
         */
        int closing_arc_(const pair<int, int> &arc) const;

        /**
         * @brief Update support graph with active arcs from LP solution
         * @param alpha_v Routing arc variables
         * @param beta_v Time variables (unused)
         * @param gamma_v Sync arc variables
         * @param[out] active_sync_arcs Sync arcs with gamma > tolerance
         * 
         * Rebuilds support graph:
         * 1. Clear existing graph
         * 2. Add routing arcs where alpha > tolerance
         * 3. Add sync arcs where gamma > tolerance
         * 4. Collect active sync arcs for cycle detection
         * 
         * Active sync arcs selected based on:
         * - If arc connects non-depot operations: always include
         * - If arc connects depots: include only if both depots are active
         */
        void update_support_graph_(const vector<double> &alpha_v,
                                   const vector<double> &beta_v,
                                   const vector<double> &gamma_v,
                                   vector<pair<int, int>> &active_sync_arcs);

    public:
        /**
         * @brief Remove duplicate cycles from cycle list
         * @param[in,out] cycles Vector of cycles (modified to remove duplicates)
         * 
         * Two cycles are considered duplicates if they contain the same
         * routing arcs (ignoring sync arcs and arc order).
         * 
         * Algorithm:
         * 1. Convert each cycle to boolean vector of routing arcs
         * 2. Compare all pairs of cycles
         * 3. Mark duplicates for removal
         * 4. Rebuild cycle vector without duplicates
         * 
         * Note: Comment in code acknowledges inefficiency (O(n²×m) comparisons)
         */
        void remove_repeated_cycles_(vector<vector<int>> &cycles) const;
    };

}