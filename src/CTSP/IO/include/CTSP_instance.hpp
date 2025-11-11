/**
 * @file CTSP_instance.hpp
 * @brief Consistent Traveling Salesman Problem instance representation
 * 
 * This module defines the CTSP (Consistent TSP) instance class, extending PTSP
 * with synchronization constraints that require temporal coordination between
 * different vehicles/days visiting the same customer.
 * 
 * **CTSP Definition:**
 * 
 * The Consistent TSP is a multi-day vehicle routing problem where:
 * - Multiple vehicles visit customers over multiple days
 * - Customers may require visits on multiple days
 * - **Consistency constraint**: All visits to the same customer must occur
 *   within a time window of maximum width T[i]
 * 
 * This ensures predictable service times for customers, which is important
 * in applications like:
 * - Home delivery services (consistent delivery times)
 * - Healthcare routing (patient preference for consistent visit times)
 * - Waste collection (residents expect consistent pickup times)
 * 
 * **Mathematical Formulation:**
 * ```
 * Given:
 * - n customers, m vehicles, d days
 * - demands[i][j] = 1 if customer i needs service on day j, 0 otherwise
 * - T[i] = maximum time window width for customer i
 * - max_distance = maximum route duration per vehicle
 * 
 * Find routes minimizing total distance such that:
 * - Each customer visit requirement is satisfied
 * - For each customer i, max_visit_time[i] - min_visit_time[i] ≤ T[i]
 * - Each route duration ≤ max_distance
 * ```
 * 
 * @note This problem is NP-hard and requires specialized algorithms
 * @see PTSP::instance for the base multi-day problem without consistency
 */

#pragma once


#include "PTSP_instance.hpp"

using namespace std;

namespace CTSP
{
    /**
     * @class instance
     * @brief CTSP instance with synchronization constraints
     * 
     * Extends PTSP::instance with:
     * - **Time window constraints** T[i]: Maximum spread of visit times to customer i
     * - **Route duration constraint** max_distance: Maximum distance per vehicle per day
     * - **Known solutions**: Optimal or best-known objective values for benchmarking
     * 
     * **Instance Format Example:**
     * ```
     * NAME: bayg29_p5_f90_lL
     * TYPE: CTSP
     * DIMENSION: 30 (1 depot + 29 customers)
     * NUM_DAYS: 5
     * DISTANCE: 900 (max route duration)
     * MAXIMUM_ALLOWABLE_DIFFERENTIAL: 90 (time window width)
     * DEMAND_SECTION:
     * 1 0 0 0 0 0  (depot, no demand)
     * 2 1 0 1 0 1  (customer 1 needs visits on days 1,3,5)
     * ...
     * ```
     * 
     * @note Inherits distance matrix and demand structure from PTSP
     * @note All customers share the same time window width in basic version
     */
    class instance: public PTSP::instance
    class instance: public PTSP::instance
    {
    protected:
        /// Time window width per customer: max_time - min_time ≤ T[i]
        vector<double> T_;
        
        /// Maximum route distance/duration per vehicle
        double max_distance_;

        /// Known optimal/best-known objective values for benchmarking
        vector<double> optimal_values_;

    public:
        /**
         * @brief Default constructor
         */
        instance(void);
        
        /**
         * @brief Constructor with file loading
         * @param input_file Path to CTSP instance file (TSPLIB format with extensions)
         */
        instance(const string &input_file);
        
        /**
         * @brief Virtual destructor
         */
        virtual ~instance(void);

        /**
         * @brief Read CTSP instance from file
         * @param input_file Path to instance file
         * @note Automatically parses TSPLIB format with CTSP extensions
         * @note Validates triangle inequality and symmetry properties
         */
        virtual void read(const string &input_file);

        /**
         * @brief Disable maximum distance constraint
         * @note Sets max_distance to a very large value (999999999)
         * @note Useful for testing or focusing on time window constraints only
         */
        void disable_max_distance(void);

        /**
         * @brief Get known optimal/best solutions
         * @return Vector of optimal values (empty if unknown)
         */
        inline const vector<double> &get_optimal_values(void) const
        {
            return optimal_values_;
        }

        /**
         * @brief Get maximum route distance constraint
         * @return Maximum allowed distance per vehicle per day
         */
        inline const double &get_max_distance(void) const
        {
            return max_distance_;
        }

        /**
         * @brief Get time window width constraints
         * @return T[i] = maximum time spread for customer i
         * @note All visits to customer i must occur within a window of width T[i]
         */
        inline const vector<double> &get_T(void) const
        {
            return T_;
        }

        /**
         * @brief Get demand matrix
         * @return demands[i][j] > 0 means customer i needs service on day j
         */
        inline const vector<vector<int>> &get_demands(void) const
        {
            return demands_;
        }

        /**
         * @brief Count total number of customer visit operations
         * @return Sum of all non-zero demands across all customers and days
         * @note Each visit is a separate operation in the optimization model
         */
        size_t get_n_customer_operations(void) const;

        /**
         * @brief Write instance summary to output stream
         * @param os Output stream
         * @return Output stream reference
         * @note Outputs instance name, dimensions, and key parameters
         */
        ostream &write_line(ostream &os) const;

        /**
         * @brief Set maximum route distance constraint
         * @param max_distance New maximum distance value
         */
        inline void set_max_distance(double max_distance)
        {
            max_distance_ = max_distance;
        }
    };
}