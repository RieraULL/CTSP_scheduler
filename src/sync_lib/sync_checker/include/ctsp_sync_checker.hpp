#pragma once

#include "model_description.hpp"
#include "sync_checker_solver.hpp"
#include "sync_model_a_builder.hpp"

#include <vector>
#include <cmath>

using namespace std;

/**
 * @file ctsp_sync_checker.hpp
 * @brief Synchronization constraint checker for CTSP (Consistent TSP)
 * 
 * This file defines the base class for checking synchronization constraints
 * in Consistent Traveling Salesman Problems. The checker verifies whether
 * a given routing solution satisfies the temporal synchronization requirements
 * between vehicles visiting the same customers.
 * 
 * The checker uses LP formulations to:
 * - Verify constraint feasibility
 * - Compute dual variables for cut generation
 * - Extract violated inequalities for branch-and-cut algorithms
 */

namespace SYNC_LIB
{
    /**
     * @class ctsp_sync_checker
     * @brief Base class for CTSP synchronization constraint verification
     * 
     * This class provides functionality to check whether a routing solution
     * satisfies synchronization constraints in CTSP problems. It builds and
     * solves LP models whose feasibility indicates whether vehicles can visit
     * customers in a synchronized manner (within time windows or simultaneously).
     * 
     * Key responsibilities:
     * - Build primal LP model for synchronization checking
     * - Verify solution feasibility against synchronization constraints
     * - Extract dual variables (α, β, γ) for constraint generation
     * - Compute slack variables for primal feasible solutions
     * 
     * The class solves a parametric LP where the RHS depends on the current
     * routing solution x. The LP is feasible iff synchronization is achievable.
     * 
     * Dual variables:
     * - α: Multipliers for arrival time constraints
     * - β: Multipliers for route duration constraints
     * - γ: Multipliers for synchronization coupling constraints
     * 
     * These duals are used to generate valid inequalities (cuts) in
     * branch-and-cut algorithms for solving CTSP.
     */
    class ctsp_sync_checker : public GOMA::sync_checker_solver
    {
    protected:
        const double tol_;          ///< Numerical tolerance for feasibility checks
        const double precision_;    ///< Precision for value truncation (1E3)

        size_t n_operations_;       ///< Total number of operations (tasks)
        size_t n_routing_arcs_;     ///< Number of routing arcs
        size_t n_sync_arcs_;        ///< Number of synchronization arcs
        size_t n_customers_;        ///< Number of customers
        size_t n_depots_;           ///< Number of depots/vehicles

        size_t n_col_;              ///< Number of columns (variables) in LP

        double *routing_arc_resources_;   ///< Travel times for routing arcs
        double *operations_resources_;    ///< Service times for operations

        double max_distance_;       ///< Maximum route duration

        vector<vector<int>> routing_outbound_arcs_;  ///< Outbound arcs per operation
        vector<vector<int>> routing_inbound_arcs_;   ///< Inbound arcs per operation
        vector<int> operation_2_customer_;           ///< Maps operation to customer
        vector<triplet> routing_arcs_;               ///< List of routing arcs
        pair_map routing_arcs_pair_map_;             ///< Maps arc pairs to indices

        int *col_inx_;      ///< Column indices for sparse matrix updates
        int *row_inx_;      ///< Row indices for sparse matrix updates
        double *coef_val_;  ///< Coefficient values for sparse updates
        char *sense_;       ///< Constraint senses for updates

        double *alpha_;     ///< Buffer for dual variable values
        double *s_;         ///< Buffer for slack variable values

    protected:
        size_t n_alpha_var_;     ///< Number of α variables
        size_t n_beta_var_;      ///< Number of β variables
        size_t n_gamma_var_;     ///< Number of γ variables

        size_t base_alpha_var_;  ///< Base index for α variables
        size_t base_beta_var_;   ///< Base index for β variables
        size_t base_gamma_var_;  ///< Base index for γ variables

    public:
        /**
         * @brief Construct synchronization checker with model and builder
         * @param builder Model A builder containing problem structure
         * @param model LP model description for synchronization checking
         * @param tol Numerical tolerance for feasibility (default: 1e-3)
         */
        ctsp_sync_checker(const sync_model_a_builder &builder, const GOMA::model_description &model, double tol);
        
        /**
         * @brief Default constructor (creates uninitialized checker)
         */
        ctsp_sync_checker(void);
        
        virtual ~ctsp_sync_checker(void);

        /**
         * @brief Initialize checker with model and builder
         * @param builder Model A builder
         * @param model LP model description
         * @param tol Numerical tolerance
         */
        void set(const sync_model_a_builder &builder, const GOMA::model_description &model, double tol);

        /**
         * @brief Get number of LP columns (variables)
         * @return Number of variables in the LP
         */
        inline size_t get_n_col(void) const { return n_col_; }

        /**
         * @brief Check if routing solution satisfies synchronization constraints
         * @param x Routing solution (arc variables)
         * @return true if synchronization is feasible, false otherwise
         * 
         * This method updates the LP's RHS based on solution x and checks feasibility.
         * If infeasible, synchronization cannot be achieved for this routing.
         */
        bool is_feasible_(const vector<double> &x);

        /**
         * @brief Check feasibility and return objective value
         * @param x Routing solution
         * @param obj_val Output: LP objective value
         * @return true if feasible
         */
        bool is_feasible_(const vector<double> &x, double &obj_val);

        /**
         * @brief Check feasibility and extract dual variables
         * @param x Routing solution
         * @param alpha Output: α dual variables (arrival time multipliers)
         * @param beta Output: β dual variables (duration multipliers)
         * @param gamma Output: γ dual variables (synchronization multipliers)
         * @return true if feasible
         * 
         * When infeasible, the dual variables can be used to generate
         * valid inequalities (cuts) to exclude the current solution.
         */
        bool is_feasible(const vector<double> &x,
                         vector<double> &alpha,
                         vector<double> &beta,
                         vector<double> &gamma);

        /**
         * @brief Extract dual variables from last solve
         * @param alpha Output: α dual variables
         * @param beta Output: β dual variables
         * @param gamma Output: γ dual variables
         */
        void get_alpha_beta_gamma(vector<double> &alpha, vector<double> &beta, vector<double> &gamma) const;
        
        /**
         * @brief Extract slack variables from feasible solution
         * @param s Output: slack variable values
         */
        void get_s(vector<double> &s) const;

    protected:
        /**
         * @brief Check feasibility without updating RHS (uses current state)
         * @return true if current LP is feasible
         */
        bool is_feasible_(void);
        
        /**
         * @brief Check feasibility and get objective value
         * @param obj_val Output: objective value
         * @return true if feasible
         */
        bool is_feasible_(double &obj_val);

        /**
         * @brief Write current LP model to file for debugging
         * @param filename Output file path
         */
        void write(const char *filename) const;

        /**
         * @brief Extract and partition dual variables
         * @param a Raw dual variable array
         * @param alpha Output: α variables
         * @param beta Output: β variables
         * @param gamma Output: γ variables
         */
        void get_alpha_beta_gamma_(double *a, vector<double> &alpha, vector<double> &beta, vector<double> &gamma) const;
        
        /**
         * @brief Extract slack variables
         * @param s Raw slack array
         * @param s_vec Output: slack vector
         */
        void get_s_(double *s, vector<double> &s_vec) const;

        /**
         * @brief Compute number of constraints (pure virtual, implemented in derived classes)
         * @param builder Model builder
         */
        virtual void compute_constraints_number_(const sync_model_a_builder &builder) = 0;

        /**
         * @brief Convert routing solution x to LP constraint coefficients
         * @param x Routing solution
         */
        void x_2_coef_(const vector<double> &x);
        
        /**
         * @brief Build α constraint coefficients from solution x
         * @param x Routing solution
         * @param row_i Row index
         * @param nz Number of non-zeros
         */
        void x_2_alpha_coef_(const vector<double> &x, const size_t row_i, int &nz);
        
        /**
         * @brief Build β constraint coefficients from solution x
         * @param x Routing solution
         * @param row_i Row index
         * @param nz Number of non-zeros
         */
        void x_2_beta_coef_(const vector<double> &x, const size_t row_i, int &nz);

        /**
         * @brief Convert routing solution x to LP objective coefficients
         * @param x Routing solution
         */
        void x_2_obj_(const vector<double> &x);
        
        /**
         * @brief Build α objective coefficients from solution x
         * @param x Routing solution
         * @param nz Number of non-zeros
         */
        void x_2_alpha_obj_(const vector<double> &x, int &nz);
        
        /**
         * @brief Build β objective coefficients from solution x
         * @param x Routing solution
         * @param nz Number of non-zeros
         */
        void x_2_beta_obj_(const vector<double> &x, int &nz);

        /**
         * @brief Truncate value to specified precision
         * @param val Input value
         * @return Truncated value
         */
        inline double truncate_(const double val) const {return round(val * precision_) / precision_;}
    };

}