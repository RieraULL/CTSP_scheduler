/**
 * @file LP_solver.hpp
 * @brief Abstract interface for LP/MIP solvers
 * 
 * This module defines a solver-independent interface for Linear Programming
 * and Mixed Integer Programming solvers. Concrete implementations can be
 * provided for different solvers (CPLEX, CLP, Gurobi, etc.).
 */

#pragma once

#include "CPX_model_structure.hpp"


namespace GOMA
{
    /**
     * @class LP_solver
     * @brief Abstract base class for optimization solvers
     * 
     * This class provides a unified interface for solving LP and MIP problems,
     * independent of the underlying solver implementation. It supports:
     * 
     * - **Model building**: From model_description objects
     * - **Solving**: LP and MIP problems
     * - **Solution retrieval**: Primal and dual variable values
     * - **Model modification**: Changing coefficients, bounds, RHS
     * - **Cutting planes**: Adding constraints during solve
     * 
     * Concrete implementations (e.g., CPX_solver for CPLEX) provide the
     * actual solver bindings.
     * 
     * @note All methods are pure virtual except constructor/destructor
     * @note Index conventions match those of the underlying solver
     * 
     * @see CPX_solver for CPLEX implementation
     * @see model_description for model representation
     */
    class LP_solver
    {
    protected:
        const double tol_; ///< Numerical tolerance for constraints

        int n_col_; ///< Number of decision variables
        int n_row_; ///< Number of constraints

        int prob_type_; ///< Problem type (LP or MIP)

        int lpstat_; ///< LP solver status (e.g., optimal, infeasible, unbounded)

    public:
        /**
         * @brief Construct solver from model description
         * @param model Model to solve
         * @param tol Numerical tolerance (default 1e-6)
         */
        LP_solver(const model_description &model, const double tol = 1e-6);
        
        /**
         * @brief Virtual destructor
         */
        virtual ~LP_solver(void);

        /**
         * @brief Solve the optimization problem
         * @note Automatically detects LP vs MIP based on model type
         */
        virtual void solve(void) = 0;

        /**
         * @brief Retrieve dual variable values (shadow prices)
         * @param alpha [output] Array to store dual values (size = n_row)
         * @note Only valid for LP problems and optimal solutions
         */
        virtual void get_dual_vars(double *alpha) const = 0;
        
        /**
         * @brief Retrieve primal variable values
         * @param alpha [output] Array to store variable values (size = n_col)
         */
        virtual void get_vars(double *alpha) const = 0;

        /**
         * @brief Set objective function coefficients
         * @param obj_coef Array of new coefficients
         * @param obj_inx Array of variable indices
         * @param sz Number of coefficients to change
         */
        virtual void set_obj(double *obj_coef, int *obj_inx, int sz) = 0;
        
        /**
         * @brief Set variable bounds
         * @param obj_coef Array of new bound values
         * @param sense Array of bound types ('L', 'U', 'B')
         * @param obj_inx Array of variable indices
         * @param sz Number of bounds to change
         */
        virtual void set_bdn(double *obj_coef, char *sense, int *obj_inx, int sz) = 0;
        
        /**
         * @brief Set right-hand side values
         * @param cnt Number of RHS values to change
         * @param rhs_inx Array of constraint indices
         * @param rhs_val Array of new RHS values
         */
        virtual void set_rhs(int cnt, const int *rhs_inx, const double *rhs_val) = 0;
        
        /**
         * @brief Set constraint matrix coefficients
         * @param cnt Number of coefficients to change
         * @param row_inx Array of row indices
         * @param col_inx Array of column indices
         * @param coef_val Array of new coefficient values
         */
        virtual void set_coef(int cnt, const int *row_inx, const int *col_inx, const double *coef_val) = 0;

        /**
         * @brief Add cutting plane constraints
         * @param nzcnt Number of constraints to add
         * @param rhs Right-hand side values
         * @param sense Constraint senses ('L', 'E', 'G')
         * @param rmatbeg Row start indices
         * @param rmatind Column indices of non-zeros
         * @param rmatval Non-zero coefficient values
         * @param rowname Row names (can be NULL)
         * @note Used in branch-and-cut algorithms
         */
        virtual void add_cut(int nzcnt, double const *rhs, char const *sense, int const *rmatbeg, int const *rmatind, double const *rmatval, char **rowname) = 0;

        /**
         * @brief Set single RHS value
         * @param row Constraint index
         * @param val New RHS value
         */
        virtual void set_rhs(const int row, const double val) = 0;
        
        /**
         * @brief Disable linear preprocessing
         * @note May be needed for debugging or special algorithms
         */
        virtual void disable_prep_linear(void) = 0;

        /**
         * @brief Delete constraint rows
         * @param begin First row to delete
         * @param end Last row to delete (inclusive)
         */
        virtual void del_rows(int begin, int end) = 0;

        /**
         * @brief Get objective function value
         * @return Objective value of current solution
         * @note Returns special values for infeasible/unbounded problems
         */
        virtual double get_obj(void) const = 0;

        /**
         * @brief Clear/reset the solver
         * @note Frees solver-specific data structures
         */
        virtual void clear(void) = 0;

        /**
         * @brief Solve as Linear Program
         * @note Forces LP solve even if model has integer variables
         */
        virtual void solve_LP(void) = 0;
        
        /**
         * @brief Solve as Mixed Integer Program
         * @note Only valid if model has integer variables
         */
        virtual void solve_MIP(void) = 0;

        /**
         * @brief Get number of non-zero coefficients
         * @return Number of non-zeros in constraint matrix
         */
        virtual int get_nz(void) const = 0;

        // Non-virtual accessors
        inline int get_n_col(void) const { return n_col_; }
        inline int get_n_row(void) const { return n_row_; }

        /**
         * @brief Get current number of rows (may differ from initial if cuts added)
         * @return Current number of constraint rows
         */
        virtual int get_n_rows(void) const = 0;

        /**
         * @brief Get LP solver status
         * @return Status code (optimal, infeasible, unbounded, etc.)
         */
        inline int get_lp_stat(void) const { return lpstat_; }

        /**
         * @brief Write model to file
         * @param filename Output file path (.lp, .mps, etc.)
         * @note Format determined by file extension
         */
        virtual void write_model(const char *filename) const = 0;
    };
}