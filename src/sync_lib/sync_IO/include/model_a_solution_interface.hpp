#pragma once

#include "sync_model_a_builder.hpp"
#include "sync_solution.hpp"

#include "matrix.hpp"

#include <vector>

using namespace std;

/**
 * @file model_a_solution_interface.hpp
 * @brief Interface for converting between solution representations
 * 
 * This file provides conversion utilities between the high-level sync_solution
 * representation (routes) and the low-level Model A representation (decision
 * variable vectors) used in mathematical programming formulations.
 */

namespace SYNC_LIB
{
    /**
     * @class model_a_solution_interface
     * @brief Bidirectional converter between sync_solution and Model A variables
     * 
     * This class acts as a bridge between:
     * 1. High-level representation: sync_solution (routes as sequences of customers)
     * 2. Low-level representation: Model A decision variables (arc selection variables)
     * 
     * Model A is an arc-based formulation where binary variables x[a] indicate
     * whether arc 'a' is used in the solution. This interface maintains the mapping
     * between arcs and their corresponding variable indices.
     * 
     * Reference: Riera-Ledesma et al., "Dual-driven path elimination for vehicle
     * routing with idle times and arrival-time consistency", C&OR, 2025.
     */
    class model_a_solution_interface
    {
    private:
        string instance_name_;                          ///< Problem instance name
        SYNC_LIB::pair_map routing_arcs_pair_map_;     ///< Maps (i,j) pairs to arc indices
        vector<SYNC_LIB::triplet> routing_arcs_;       ///< List of all routing arcs
        vector<double> routing_arc_times_;              ///< Travel time for each routing arc
        vector<SYNC_LIB::sync_operation> operations_;   ///< All operations in the problem
        GOMA::matrix<int> operations_map_;              ///< Maps (customer, depot) to operation index
        size_t n_depots_;                               ///< Number of depots/vehicles
        vector<string> routing_arc_names_;              ///< Human-readable arc names

    public:
        model_a_solution_interface(void);
        virtual ~model_a_solution_interface(void);

        /**
         * @brief Initialize the interface from a Model A builder
         * @param model_builder The Model A builder containing problem structure
         * 
         * This method extracts all necessary mapping information from the
         * model builder to enable bidirectional conversion.
         */
        void set(const sync_model_a_builder &model_builder);

        /**
         * @brief Convert sync_solution to Model A variable vector
         * @param sol Input solution (routes)
         * @param x Output: binary decision variables (1 if arc used, 0 otherwise)
         * 
         * This converts a high-level routing solution into the arc-based
         * representation used in optimization models.
         */
        void sync_solution_2_model_a(const SYNC_LIB::sync_solution &sol, vector<double> &x) const;
        
        /**
         * @brief Convert Model A variable vector to sync_solution
         * @param x Input: binary decision variables
         * @param sol Output: solution as routes
         * 
         * This reconstructs a routing solution from the arc selection variables
         * produced by an optimization solver.
         */
        void model_a_2_sync_solution(const vector<double> &x, SYNC_LIB::sync_solution &sol) const;
        
        /**
         * @brief Map operation index to (customer, depot) pair
         * @param operation Operation index
         * @param sync_pair Output: (customer_id, depot_id) pair
         */
        void operation_2_sync_solution_pair(const int operation, pair<int, int> &sync_pair) const;
    };
}