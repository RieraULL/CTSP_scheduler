#pragma once

#include "ctsp_sync_checker.hpp"

/**
 * @file ctsp_lb_sync_checker.hpp
 * @brief Synchronization checker for lower bound computation
 * 
 * This file defines a specialized synchronization checker used in
 * lower bound computations for CTSP problems.
 */

namespace SYNC_LIB
{
    /**
     * @class ctsp_lb_sync_checker
     * @brief Synchronization checker for CTSP lower bounds
     * 
     * This class extends the base synchronization checker with specialized
     * functionality for lower bound computation. It is typically used in:
     * 
     * - **Lagrangian relaxation**: Computing bounds by relaxing synchronization
     * - **Column generation**: Pricing out new routes with sync constraints
     * - **Bounding procedures**: Certifying solution quality in branch-and-bound
     * 
     * The checker uses a modified constraint counting scheme that may
     * reduce or strengthen certain constraint sets to obtain valid bounds.
     * 
     * Usage in branch-and-bound:
     * 1. Solve relaxed routing problem (ignore some sync constraints)
     * 2. Use this checker to verify if remaining constraints are satisfied
     * 3. If infeasible, extract dual variables to generate cuts
     * 4. Add cuts to strengthen formulation and improve bounds
     */
    class ctsp_lb_sync_checker : public ctsp_sync_checker
    {
    public:
        /**
         * @brief Construct lower bound synchronization checker
         * @param builder Model A builder containing problem structure
         * @param tol Numerical tolerance for feasibility checks
         */
        ctsp_lb_sync_checker(const sync_model_a_builder &builder, double tol);
        
        /**
         * @brief Default constructor
         */
        ctsp_lb_sync_checker(void);
        
        virtual ~ctsp_lb_sync_checker(void);

    protected:
        /**
         * @brief Compute constraint counts for lower bound model
         * @param builder Model builder
         * 
         * Overrides base implementation to set appropriate numbers of
         * α, β, and γ constraints for the lower bound formulation.
         */
        void compute_constraints_number_(const sync_model_a_builder &builder);
    };

}