/**
 * @file PTSP_instance.hpp
 * @brief Periodic Traveling Salesman Problem instance representation
 * 
 * This module defines the PTSP (Periodic TSP) instance class, which extends
 * the classic TSP to multi-day scenarios where customers may require visits
 * on multiple days with varying demands.
 * 
 * PTSP is characterized by:
 * - Multiple planning periods (days)
 * - Day-dependent customer demands
 * - Same distance matrix across all days
 * - No synchronization constraints between days
 */

#pragma once

#include <vector>
#include <iostream>
#include <string>
#include <utility>

#include "matrix.hpp"

using namespace std;

namespace PTSP
{
    /**
     * @class instance
     * @brief Periodic TSP instance data container
     * 
     * This class represents a PTSP problem instance with:
     * - A set of customers to be visited
     * - Multiple planning days/periods
     * - Day-specific demand patterns
     * - A single distance matrix shared across all days
     * 
     * The PTSP problem involves planning routes for multiple days where:
     * - Each customer has demands on specific days
     * - Routes can be different on each day
     * - No coordination between different days is required
     * 
     * This class serves as the base for CTSP (Consistent TSP), which adds
     * synchronization constraints requiring coordination between days.
     * 
     * **Problem Structure:**
     * ```
     * - n customers
     * - m days
     * - demands[i][j] = demand of customer i on day j
     * - distances[i][j] = travel distance between customers i and j
     * ```
     * 
     * @note This class validates distance matrix properties (symmetry, triangle inequality)
     * @note Derived from TSPLIB format but with multi-day extensions
     * 
     * @see CTSP::instance for the consistent variant with synchronization
     * @see TSP::TSPLIB_instance for the underlying file format parser
     */
    class instance
    {
    protected:
        string id_;      ///< Instance identifier/name
        string comment_; ///< Instance description
        string type_;    ///< Problem type (e.g., "PTSP", "CTSP")

        size_t n_customers_; ///< Number of customers (excluding depot)
        size_t n_days_;      ///< Number of planning days/periods

        GOMA::matrix<double> distances_; ///< Distance matrix (1-based indexing)

        bool triangle_inequality_; ///< True if distances satisfy triangle inequality
        bool symmetry_;            ///< True if distance matrix is symmetric

        /// Demand matrix: demands_[customer][day]
        /// demands_[i][j] > 0 means customer i requires service on day j
        vector<vector<int>> demands_;

    public:
        /**
         * @brief Default constructor
         */
        instance(void);
        
        /**
         * @brief Virtual destructor for inheritance
         */
        virtual ~instance(void);

        /**
         * @brief Check if triangle inequality holds
         * @return true if d(i,k) + d(k,j) >= d(i,j) for all i,j,k
         */
        inline bool triangle_inequality(void) const
        {
            return triangle_inequality_;
        }

        /**
         * @brief Check if distance matrix is symmetric
         * @return true if d(i,j) = d(j,i) for all i,j
         */
        inline bool symmetry(void) const
        {
            return symmetry_;
        }

        inline const string &get_instance_name(void) const
        {
            return id_;
        }

        inline const string &get_instance_comment(void) const
        {
            return comment_;
        }

        inline const string &get_instance_type(void) const
        {
            return type_;
        }

        inline const GOMA::matrix<double> &get_distances(void) const
        {
            return distances_;
        }

        inline const size_t &get_n_days(void) const
        {
            return n_days_;
        }

        inline size_t get_n_customers(void) const
        {
            return n_customers_;
        }

        /**
         * @brief Get demand matrix
         * @return demands_[i][j] = demand of customer i on day j (0 = no visit needed)
         */
        inline const vector<vector<int>> &get_demands(void) const
        {
            return demands_;
        }

    protected:
        /**
         * @brief Verify triangle inequality property
         * @return true if d(i,k) + d(k,j) >= d(i,j) - ε for all distinct i,j,k
         * @note Uses tolerance of 1.01 to account for floating-point errors
         */
        bool check_triangle_inequality_(void) const;
        
        /**
         * @brief Verify distance matrix symmetry
         * @return true if d(i,j) = d(j,i) for all i,j
         */
        bool check_symmetry_(void) const;
    };
}