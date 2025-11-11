/**
 * @file CTSP_model_a_builder.hpp
 * @brief CTSP-specific model builder adapter
 * 
 * This module provides a bridge between CTSP problem instances and the
 * generic synchronization model builder used for constraint verification
 * and optimization.
 * 
 * It adapts CTSP-specific data structures to the sync_model_a_builder
 * interface, allowing CTSP problems to be solved using the synchronization
 * constraint framework.
 */

#pragma once

#include "sync_model_a_builder.hpp"
#include "CTSP_instance.hpp"

/// Sentinel value indicating max_distance constraint is not set
#define MAX_DISTANCE_NOT_SET 999999999

namespace CTSP
{
    /**
     * @enum CTSP_problem_type
     * @brief CTSP problem variants
     * 
     * Distinguishes between different CTSP formulations:
     * - **CTSP1**: Single-depot variant (one depot for all vehicles)
     * - **CTSP2**: Multi-depot variant (each vehicle has its own depot)
     * 
     * @note CTSP2 is more general and includes CTSP1 as a special case
     */
    enum class CTSP_problem_type
    {
        CTSP1, ///< Single-depot Consistent TSP
        CTSP2  ///< Multi-depot Consistent TSP
    };

    /**
     * @class CTSP_model_a_builder
     * @brief Builds synchronization models from CTSP instances
     * 
     * This class adapts CTSP problem instances to the sync_model_a_builder
     * interface, which is used throughout the solver framework for:
     * - Constraint verification (sync_checker)
     * - Solution validation (sync_verify)
     * - Model construction for optimization
     * 
     * **Conversion Process:**
     * 
     * CTSP instances contain:
     * - Customer demands per day
     * - Distance matrices
     * - Time window constraints (T)
     * - Maximum route distance
     * 
     * These are transformed into:
     * - Operation sets (pickups, deliveries, customer visits)
     * - Arc time matrices
     * - Synchronization constraint parameters
     * - Depot configurations
     * 
     * **Problem Type Selection:**
     * - `CTSP1`: All vehicles share a single depot
     * - `CTSP2`: Each vehicle operates from its own depot
     * 
     * @note This is a thin adapter; actual model building logic is in sync_model_a_builder
     * @note The choice between CTSP1 and CTSP2 affects depot operation modeling
     * 
     * @see SYNC_LIB::sync_model_a_builder for the underlying model builder
     * @see CTSP::instance for the problem instance representation
     */
    class CTSP_model_a_builder: public SYNC_LIB::sync_model_a_builder
    {
        public:
            /**
             * @brief Construct model builder from CTSP instance
             * @param problem_type Problem variant (CTSP1 or CTSP2)
             * @param instance CTSP instance data
             * 
             * @note Automatically extracts all necessary data from instance
             * @note Sets n_depots based on problem_type:
             *       - CTSP1: n_depots = 1
             *       - CTSP2: n_depots = n_days (one depot per day/vehicle)
             */
            CTSP_model_a_builder(const CTSP::CTSP_problem_type &problem_type, 
                                 const CTSP::instance &instance);
            
            /**
             * @brief Destructor
             */
            virtual ~CTSP_model_a_builder(void);
    };

    /**
     * @class CTSP_model_a_builder_solution
     * @brief Solution representation for CTSP models
     * 
     * @note Currently placeholder for future solution-specific functionality
     * @todo Implement solution storage and manipulation methods
     */
    class CTSP_model_a_builder_solution
    {

    };
}