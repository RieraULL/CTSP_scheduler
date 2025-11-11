#pragma once

#include "model_description.hpp"
#include "sync_model_a_builder.hpp"

/**
 * @file ctsp_primal_model.hpp
 * @brief Primal LP model for CTSP synchronization checking
 * 
 * This file defines the base class for building primal LP models used
 * to verify synchronization feasibility in CTSP problems. The primal model
 * includes variables for arrival times, route durations, and synchronization
 * coupling, along with constraints that enforce temporal consistency.
 */

namespace SYNC_LIB
{
    /**
     * @class ctsp_primal_model
     * @brief Base primal LP model for CTSP synchronization verification
     * 
     * This class builds an LP model whose feasibility indicates whether
     * a routing solution can be synchronized according to CTSP requirements.
     * 
     * The model includes three types of constraints:
     * 
     * 1. **Alpha constraints**: Arrival time consistency
     *    - Ensure that arrival times respect travel times
     *    - Link arrival times across consecutive operations
     * 
     * 2. **Beta constraints**: Route duration limits
     *    - Enforce maximum route duration constraints
     *    - Bound total travel/service time per vehicle
     * 
     * 3. **Gamma constraints**: Synchronization coupling
     *    - Ensure vehicles arrive within allowed time windows (CTSP1)
     *    - OR ensure simultaneous arrivals (CTSP2)
     *    - Link arrival times of different vehicles to same customer
     * 
     * Variables:
     * - α: Arrival times at each operation
     * - β: Route durations per vehicle
     * - γ: Synchronization slack variables
     * 
     * The model is parametric in the routing solution x, meaning the
     * RHS and/or objective coefficients depend on which arcs are selected.
     */
    class ctsp_primal_model : public GOMA::model_description
    {
    protected:
        size_t n_alpha_constraints_;     ///< Number of α constraints
        size_t n_beta_constraints_;      ///< Number of β constraints
        size_t n_gamma_constraints_;     ///< Number of γ constraints

        size_t base_alpha_constraints_;  ///< Base index for α constraints
        size_t base_beta_constraints_;   ///< Base index for β constraints
        size_t base_gamma_constraints_;  ///< Base index for γ constraints

    public:
        /**
         * @brief Construct primal model from builder
         * @param builder Model A builder containing problem structure
         */
        ctsp_primal_model(const sync_model_a_builder &builder);
        
        virtual ~ctsp_primal_model(void);

        /**
         * @brief Get number of β constraints
         * @return Count of route duration constraints
         */
        inline size_t get_n_beta_constraints(void) const { return n_beta_constraints_; }
        
        /**
         * @brief Get base index for β constraints
         * @return Starting index of β constraints
         */
        inline size_t get_base_beta_constraints(void) const { return base_beta_constraints_; }

    protected:
        /**
         * @brief Build complete LP model
         * @param builder Model builder
         */
        void build_model_(const sync_model_a_builder &builder);

        /**
         * @brief Define LP variables (α, β, γ)
         * @param builder Model builder
         */
        void build_variables_(const sync_model_a_builder &builder);
        
        /**
         * @brief Build all constraint rows
         * @param builder Model builder
         */
        void build_constraints_(const sync_model_a_builder &builder);
        
        /**
         * @brief Set optimization sense (min/max)
         * @param builder Model builder
         */
        void build_objective_sense_(const sync_model_a_builder &builder);

        /**
         * @brief Build α constraints (arrival time consistency)
         * @param builder Model builder
         */
        void build_alpha_constraints_(const sync_model_a_builder &builder);
        
        /**
         * @brief Build β constraints (route duration limits)
         * @param builder Model builder
         */
        void build_beta_constraints_(const sync_model_a_builder &builder);
        
        /**
         * @brief Build γ constraints (synchronization coupling)
         * @param builder Model builder
         */
        void build_gamma_constraints_(const sync_model_a_builder &builder);

        /**
         * @brief Build constraint matrix for primal model
         * @param builder Model builder
         * @param M Output: constraint matrix
         * @param nz Output: number of non-zeros
         */
        void build_primal_matrix_(const sync_model_a_builder &builder, GOMA::matrix<double> &M, int &nz);

        /**
         * @brief Build α portion of constraint matrix
         * @param builder Model builder
         * @param M Output: matrix to populate
         * @param nz Input/Output: non-zero counter
         */
        void build_alpha_primal_matrix_(const sync_model_a_builder &builder, GOMA::matrix<double> &M, int &nz);
        
        /**
         * @brief Build β portion of constraint matrix
         * @param builder Model builder
         * @param M Output: matrix to populate
         * @param nz Input/Output: non-zero counter
         */
        void build_beta_primal_matrix_(const sync_model_a_builder &builder, GOMA::matrix<double> &M, int &nz);
        
        /**
         * @brief Build γ portion of constraint matrix
         * @param builder Model builder
         * @param M Output: matrix to populate
         * @param nz Input/Output: non-zero counter
         */
        void build_gamma_primal_matrix_(const sync_model_a_builder &builder, GOMA::matrix<double> &M, int &nz);

        /**
         * @brief Set variable names for debugging
         * @param builder Model builder
         */
        void set_var_labels_(const sync_model_a_builder &builder);
        
        /**
         * @brief Set constraint names for debugging
         * @param builder Model builder
         */
        void set_cons_labels_(const sync_model_a_builder &builder);

        /**
         * @brief Initialize model structure
         * @param builder Model builder
         */
        void init_model_(const sync_model_a_builder &builder);

        /**
         * @brief Compute number of each constraint type (pure virtual)
         * @param builder Model builder
         */
        virtual void compute_constraints_number_(const sync_model_a_builder &builder) = 0;

        /**
         * @brief Compute number of LP columns
         * @param builder Model builder
         */
        void compute_n_col_(const sync_model_a_builder &builder);
        
        /**
         * @brief Compute number of LP rows
         * @param builder Model builder
         */
        void compute_n_row_(const sync_model_a_builder &builder);
    };

}