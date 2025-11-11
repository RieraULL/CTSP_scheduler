#pragma once

#include <vector>
#include <utility>

using namespace std;

/**
 * @file sync_types.hpp
 * @brief Common type definitions for CTSP-like problems
 * 
 * This file contains fundamental type aliases and structures used throughout
 * the synchronization library for representing CTSP (Consistent Traveling
 * Salesman Problem) and related vehicle routing problems.
 */

namespace SYNC_LIB
{
    /// Successor list representation for routes (adjacency list format)
    typedef vector<vector<int>> succ_list;
    
    /// Set of operation indices
    typedef vector<int> subset;
    
    /// Resource consumption vector for operation subsets
    typedef vector<double> subset_resource_vector;
    
    /// Resource consumption vector for arcs
    typedef vector<double> arcs_resource_vector;
    
    /// General resource consumption vector
    typedef vector<double> resource_vector;
    
    /// Coordinates in 2D space (x, y)
    typedef pair<double, double> coordinates;
    
    /// Pair of operation indices (from, to)
    typedef pair<int, int> operation_pair;
    
    /// Pair of subset indices (from_subset, to_subset)
    typedef pair<int, int> subset_pair;
    
    /// Arc identifier: pair of operations and their respective subsets
    typedef pair<operation_pair, subset_pair> arc_pair;

    /**
     * @class triplet
     * @brief Represents an arc in the routing graph with operation and subset information
     * 
     * A triplet defines an arc from operation i to operation j, where:
     * - i belongs to subset k_i
     * - j belongs to subset k_j
     * 
     * This structure is used in Model A representation for CTSP problems.
     */
    class triplet
    {
    public:
        int i_;    ///< Origin operation index
        int j_;    ///< Destination operation index
        int k_i_;  ///< Subset index containing operation i
        int k_j_;  ///< Subset index containing operation j

        /**
         * @brief Construct a triplet with full information
         * @param i Origin operation
         * @param j Destination operation
         * @param k_i Subset of origin operation
         * @param k_j Subset of destination operation
         */
        triplet(const int i, const int j, const int k_i, const int k_j) : i_(i),
                                                                          j_(j),
                                                                          k_i_(k_i),
                                                                          k_j_(k_j) {}

        /**
         * @brief Default constructor initializing all fields to -1
         */
        triplet(void): i_(-1), j_(-1), k_i_(-1), k_j_(-1) {}

        ~triplet(void) {}

        /**
         * @brief Less-than comparison operator (compares by origin operation)
         * @param rhs Right-hand side triplet
         * @return true if this triplet's origin is less than rhs
         */
        bool operator<(const triplet &rhs) const
        {
            if (i_ < rhs.i_)
            {
                return true;
            }

            return false;
        }
    };
}