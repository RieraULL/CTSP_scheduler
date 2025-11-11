#pragma once

#define EMPTY_VAR -1

#include <map>
#include <vector>
#include <utility>
#include <string>
#include <iostream>

#include "sync_types.hpp"
#include "matrix.hpp"

using namespace std;

/**
 * @file sync_mapping.hpp
 * @brief Mapping between pairs and indices for CTSP-like problems
 * 
 * This file provides efficient bidirectional mapping between operation pairs
 * (i,j) and their corresponding indices in the model representation.
 */

namespace SYNC_LIB
{
    /**
     * @class pair_map
     * @brief Efficient mapping from operation pairs to linear indices
     * 
     * This class provides a matrix-based mapping from pairs of operations (i,j)
     * to linear indices used in optimization models. This is particularly useful
     * for Model A formulations where decision variables are indexed by arcs.
     * 
     * The mapping uses 1-based indexing internally for compatibility with
     * mathematical formulations, but provides 0-based access methods.
     * 
     * Example:
     * - If arc from operation 0 to operation 5 is the 3rd arc, then:
     *   pair_map.at(0, 5) returns 3
     */
    class pair_map
    {
    private:
        GOMA::matrix<int> map_;  ///< Matrix storing the pair-to-index mapping

    public:
        /**
         * @brief Construct mapping for n_items operations
         * @param n_items Number of operations in the problem
         */
        pair_map(const size_t n_items);
        
        /**
         * @brief Default constructor
         */
        pair_map(void);

        ~pair_map(void);

        /**
         * @brief Initialize mapping from a list of arcs
         * @param arcs Vector of triplets (i, j, k_i, k_j) defining arcs
         * 
         * This method builds the mapping by assigning sequential indices
         * to each arc in the provided list.
         */
        void set(const vector<triplet> &arcs);

        /**
         * @brief Get the underlying mapping matrix
         * @return Constant reference to the mapping matrix
         */
        inline const GOMA::matrix<int> &get_map(void) const
        {
            return map_;
        }

        /**
         * @brief Get index for an operation pair
         * @param t Pair of operation indices
         * @return Linear index, or EMPTY_VAR (-1) if pair not in mapping
         */
        int at(const pair<int,int> &t) const
        {
            return map_(t.first + 1, t.second + 1);
        }

        /**
         * @brief Get index for an operation pair (two-argument version)
         * @param i First operation index
         * @param j Second operation index
         * @return Linear index, or EMPTY_VAR (-1) if pair not in mapping
         */
        int at(const int i, const int j) const
        {
            return map_(i + 1, j + 1);
        }
    };
}
