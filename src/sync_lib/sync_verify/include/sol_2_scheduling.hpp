/**
 * @file sol_2_scheduling.hpp
 * @brief Solution to Scheduling Converter for Consistent TSP
 * 
 * This module provides functionality to convert CTSP solutions into temporal schedules
 * with time windows. It verifies synchronization constraints and generates operation
 * schedules for each depot along with customer time windows.
 */

#pragma once

#include <vector>
#include <utility>

#include "sync_iterative_checker.hpp"
#include "ctsp_lb_sync_checker.hpp"
#include "sync_scheduling.hpp"
#include "sync_tw.hpp"

using namespace std;

namespace SYNC_LIB
{
    /**
     * @brief Pair representing operation start and completion times
     * @details First element: arrival time, Second element: start time
     */
    typedef pair<double, double> operation_times;
    
    /**
     * @brief Pair representing operation identifier and its timing information
     * @details First element: operation ID, Second element: operation_times
     */
    typedef pair<int, operation_times> operation_info;

    /**
     * @class conTSP2_scheduling
     * @brief Converts CTSP solutions to temporal schedules with time windows
     * 
     * This class takes a CTSP solution (represented as decision variables) and:
     * 1. Verifies synchronization constraints using sync_iterative_checker
     * 2. Computes operation start times for all operations
     * 3. Generates a feasible schedule for each depot
     * 4. Computes customer time windows that satisfy all constraints
     * 
     * The conversion process ensures:
     * - Temporal precedence in routes (operations ordered by start time)
     * - Synchronization constraints between vehicles
     * - Depot pickup/delivery operations placed correctly
     * - Customer time windows respect maximum width constraints
     * 
     * @note This class is essential for validating CTSP solutions and generating
     *       practical schedules for implementation
     */
    class conTSP2_scheduling
    {
    protected:
        /// Synchronization constraint checker (uses ctsp_lb_sync_checker internally)
        sync_iterative_checker<ctsp_lb_sync_checker> checker_;

        const size_t n_depots_;              ///< Number of depots in the problem
        const size_t n_customers_;           ///< Number of customers to serve
        const size_t n_operations_;          ///< Total number of operations (pickups + deliveries + customer visits)
        const double max_time_windows_size_; ///< Maximum allowed time window width
        const double max_distance_;          ///< Maximum route distance/duration
        const vector<int> operation_2_depot_;    ///< Maps each operation to its depot
        const vector<int> operation_2_customer_; ///< Maps each operation to its customer

        /// Travel time matrix between operations (i+1, j+1 indexing)
        const GOMA::matrix<double> arc_time_matrix_;

        const vector<string> operation_names_; ///< Human-readable operation names

    public:
        /**
         * @brief Construct a new conTSP2_scheduling converter
         * @param builder Model builder containing problem data
         * @param tol Numerical tolerance for constraint verification
         */
        conTSP2_scheduling(const sync_model_a_builder &builder, double tol);
        
        /**
         * @brief Destructor
         */
        virtual ~conTSP2_scheduling(void);

        /**
         * @brief Convert a CTSP solution to a temporal schedule with time windows
         * 
         * This method performs the complete conversion process:
         * 1. Verifies synchronization constraints (computes start times via LP)
         * 2. Normalizes start times (shifts to start from time 0)
         * 3. Generates operation schedules for each depot
         * 4. Computes customer time windows
         * 
         * @param instance_name Name of the instance (stored in scheduling output)
         * @param x CTSP decision variables (arc usage indicators)
         * @param scheduling [out] Generated schedule for each depot
         * @param time_windows [out] Computed time windows for each customer
         * @return true if the solution satisfies all synchronization constraints
         * @return false if synchronization constraints are violated
         */
        bool solve(const string &instance_name, const vector<double> &x, sync_scheduling &scheduling, sync_time_windows &time_windows);

    protected:
        /**
         * @brief Normalize start times to begin from time 0
         * @param s [in/out] Start time variables (modified in place)
         * @details Finds the minimum start time among depot operations and shifts
         *          all start times so the earliest depot operation starts at t=0
         */
        void refine_solution_(vector<double> &s);
        
        /**
         * @brief Convert start time variables to depot schedules
         * @param s Start time variables for all operations
         * @param scheduling [out] Schedule for each depot
         * @details Generates ordered lists of operations for each depot with:
         *          - Arrival times (earliest possible arrival)
         *          - Start times (actual operation start, may include waiting)
         *          - Depot pickup/delivery operations placed appropriately
         */
        void var_2_schedule_(const vector<double> &s, sync_scheduling &scheduling);
        
        /**
         * @brief Compute customer time windows from start time variables
         * @param s Start time variables for all operations
         * @param time_windows [out] Time windows for each customer
         * @details For each customer, computes a time window [a, b] such that:
         *          - All vehicle visits occur within [a, b]
         *          - Window width (b - a) ≤ max_time_windows_size_
         *          - Window is centered around the average visit time
         */
        void var_2_time_windows_(const vector<double> &s, sync_time_windows &time_windows);
    };

}
