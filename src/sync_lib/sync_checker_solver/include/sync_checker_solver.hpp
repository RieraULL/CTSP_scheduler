#pragma once

#include "LP_solver.hpp"

/**
 * @file sync_checker_solver.hpp
 * @brief LP solver wrapper for CTSP synchronization constraint checking
 * 
 * This file provides a high-level interface to LP solvers for solving linear
 * programming problems that arise in CTSP constraint checking, dual variable
 * computation, and separation algorithms.
 * 
 * Current Implementation: Uses CPLEX (IBM ILOG CPLEX Optimization Studio)
 * 
 * Alternative Solvers: The design supports switching to open-source alternatives
 * such as CLP (COIN-OR Linear Programming) by implementing a CLP_solver class
 * that follows the same LP_solver interface. This would eliminate the need for
 * a commercial CPLEX license.
 * 
 * The solver is primarily used for:
 * - Checking feasibility of synchronization constraints
 * - Computing dual variables for constraint generation
 * - Solving separation problems in branch-and-cut algorithms
 */

namespace GOMA
{
    /**
     * @class sync_checker_solver
     * @brief Wrapper class for LP solver operations in CTSP algorithms
     * 
     * This class provides a simplified interface to LP solvers (currently CPLEX)
     * for solving LP problems related to CTSP synchronization checking. It encapsulates:
     * - Model creation and modification
     * - Solving LP/relaxations
     * - Retrieving primal and dual solutions
     * - Dynamic constraint addition (cut generation)
     * 
     * The class uses PIMPL pattern to hide solver implementation details and
     * enable easy switching between different LP solvers (CPLEX, CLP, Gurobi, etc.).
     * 
     * To use an alternative solver like CLP (COIN-OR):
     * 1. Implement a CLP_solver class with the same interface as CPX_solver
     * 2. Replace CPX_solver instantiation in the .cpp file
     * 3. Update CMakeLists.txt to link against CLP instead of CPLEX
     * 
     * Typical usage:
     * 1. Create solver with initial model description
     * 2. Solve the LP
     * 3. Extract dual variables for constraint generation
     * 4. Add violated cuts dynamically
     * 5. Resolve until convergence
     */
    class sync_checker_solver 
    {
    private:
        LP_solver *solver_;  ///< Pointer to underlying LP solver (currently CPX_solver, can be CLP_solver)

    public:
        /**
         * @brief Construct solver with model description
         * @param model Complete LP model description (variables, constraints, objective)
         * @param tol Numerical tolerance for optimality and feasibility (default: 1e-6)
         */
        sync_checker_solver(const model_description &model, const double tol = 1e-6);
        
        /**
         * @brief Default constructor (creates empty solver)
         */
        sync_checker_solver(void);
        
        virtual ~sync_checker_solver(void);

        /**
         * @brief Initialize or reinitialize solver with new model
         * @param model LP model description
         * @param tol Numerical tolerance (default: 1e-6)
         */
        void set(const model_description &model, const double tol = 1e-6);

        /**
         * @brief Solve the current LP problem
         * 
         * Solves the linear program using the simplex or barrier method.
         * After solving, use get_obj(), get_vars(), and get_dual_vars()
         * to retrieve the solution.
         */
        void solve(void);

        /**
         * @brief Get dual variable values (constraint multipliers)
         * @param alpha Output array for dual variables (must be pre-allocated)
         * 
         * Dual variables are used in:
         * - Reduced cost computation
         * - Constraint generation
         * - Lagrangian relaxations
         */
        void get_dual_vars(double *alpha) const;
        
        /**
         * @brief Get primal variable values
         * @param s Output array for variable values (must be pre-allocated)
         */
        void get_vars(double *s) const;

        /**
         * @brief Modify right-hand side values of constraints
         * @param cnt Number of constraints to modify
         * @param rhs_inx Indices of constraints to modify
         * @param rhs_val New RHS values
         * 
         * Useful for:
         * - Parametric analysis
         * - Dynamic constraint tightening
         * - Branch-and-bound node processing
         */
        void set_rhs(int cnt, const int *rhs_inx, const double *rhs_val);
        
        /**
         * @brief Modify objective function coefficients
         * @param obj_coef New objective coefficients
         * @param obj_inx Variable indices to modify
         * @param sz Number of variables to modify
         */
        void set_obj(double *obj_coef, int *obj_inx, int sz);
        
        /**
         * @brief Modify variable bounds
         * @param obj_coef New bound values
         * @param sense Bound type ('L'=lower, 'U'=upper, 'B'=both)
         * @param obj_inx Variable indices
         * @param sz Number of variables to modify
         */
        void set_bdn(double *obj_coef, char *sense, int *obj_inx, int sz);

        /**
         * @brief Get number of non-zero coefficients in the constraint matrix
         * @return Total number of non-zeros
         */
        int get_nz(void) const;

        /**
         * @brief Get optimal objective function value
         * @return Objective value of current solution
         */
        double get_obj(void) const;
        
        /**
         * @brief Get LP solution status
         * @return Status code (e.g., optimal, infeasible, unbounded)
         * 
         * Status codes depend on the underlying solver (CPLEX).
         * Common values:
         * - 1: Optimal
         * - 3: Infeasible
         * - 2: Unbounded
         */
        int get_lp_stat(void) const;

        /**
         * @brief Add a new linear constraint (cut) to the model
         * @param lhs Variable indices in the constraint
         * @param lhs_coef Coefficients for variables in lhs
         * @param rhs Right-hand side value
         * @param sense Constraint sense ('L'=≤, 'G'=≥, 'E'==)
         * @param name Human-readable constraint name
         * 
         * This is the primary method for dynamic constraint generation in
         * branch-and-cut algorithms. New cuts are added on-the-fly when
         * violated inequalities are identified.
         */
        void add_cut(const vector<int> &lhs, const vector<double> &lhs_coef, 
                     const double rhs, const char sense, const string &name);
        
        /**
         * @brief Modify constraint matrix coefficients
         * @param cnt Number of coefficients to change
         * @param row_inx Row (constraint) indices
         * @param col_inx Column (variable) indices
         * @param coef_val New coefficient values
         */
        void set_coef(int cnt, const int *row_inx, const int *col_inx, const double *coef_val);

        /**
         * @brief Write current model to file for debugging
         * @param filename Output file path (format determined by extension: .lp, .mps, etc.)
         * 
         * Useful for:
         * - Debugging infeasible models
         * - Analyzing constraint structure
         * - External validation
         */
        void write_model(const char *filename) const;
    };
}
