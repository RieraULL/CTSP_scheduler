#pragma once

#include "ctsp_primal_model.hpp"
#include "sync_model_a_builder.hpp"

/**
 * @file ctsp_lb_primal_model.hpp
 * @brief Lower bound primal models for CTSP synchronization
 * 
 * This file defines specialized primal models for computing lower bounds
 * in CTSP problems. These models are used to certify solution quality
 * and guide search in branch-and-bound algorithms.
 */

namespace SYNC_LIB
{
    /**
     * @class ctsp_lb_primal_model
     * @brief Primal model for CTSP lower bound computation
     * 
     * This class specializes the base primal model for lower bound calculations.
     * The model can be used to:
     * - Compute Lagrangian bounds on the optimal solution
     * - Verify optimality of incumbent solutions
     * - Generate lower bounding cuts
     * 
     * The difference from the base model lies in the constraint counting
     * and potentially in the objective function formulation.
     */
    class ctsp_lb_primal_model : public ctsp_primal_model
    {
    public:
        /**
         * @brief Construct lower bound primal model
         * @param builder Model A builder
         */
        ctsp_lb_primal_model(const sync_model_a_builder &builder);
        
        virtual ~ctsp_lb_primal_model(void);

    protected:
        /**
         * @brief Compute constraint counts for lower bound model
         * @param builder Model builder
         * 
         * Overrides base implementation to set appropriate constraint
         * numbers for the lower bound formulation.
         */
        void compute_constraints_number_(const sync_model_a_builder &builder);
    };

    /**
     * @class ctsp_lb_dual_primal_model
     * @brief Dual of the lower bound primal model
     * 
     * This class builds the dual LP of the lower bound primal model.
     * The dual formulation can be more efficient to solve in some cases
     * and provides complementary information for bound computation.
     * 
     * Dual variables correspond to primal constraints:
     * - Primal α constraints → Dual variables for arrival times
     * - Primal β constraints → Dual variables for durations
     * - Primal γ constraints → Dual variables for synchronization
     * 
     * The dual optimal value equals the primal optimal value (strong duality),
     * but the dual may have fewer rows when the primal has many variables.
     */
    class ctsp_lb_dual_primal_model : public GOMA::model_description
    {
    public:
        /**
         * @brief Construct dual of lower bound primal
         * @param builder Model A builder
         */
        ctsp_lb_dual_primal_model(const sync_model_a_builder &builder);
        
        virtual ~ctsp_lb_dual_primal_model(void) {}
    };
}