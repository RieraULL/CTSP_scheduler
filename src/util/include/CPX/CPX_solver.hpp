/**
 * @file CPX_solver.hpp
 * @brief CPLEX solver implementation
 * 
 * This module provides a concrete implementation of the LP_solver interface
 * using IBM ILOG CPLEX Optimization Studio.
 * 
 * @note Requires CPLEX installation (commercial license)
 * @note Can be replaced with CLP (open-source) for open-source distribution
 */

#pragma once

#include "CPX_model_structure.hpp"
#include "LP_solver.hpp"

#pragma GCC diagnostic ignored "-Wignored-attributes"

#define IL_STD
#include <ilcplex/ilocplex.h>

namespace GOMA
{
    /**
     * @class CPX_solver
     * @brief CPLEX-based LP/MIP solver implementation
     * 
     * This class provides a complete implementation of the LP_solver interface
     * using IBM ILOG CPLEX. It wraps the CPLEX C API to provide:
     * 
     * - **LP solving**: Simplex and barrier methods
     * - **MIP solving**: Branch-and-cut with advanced presolve
     * - **Dual variables**: Shadow prices for sensitivity analysis
     * - **Model modification**: Dynamic changes to coefficients/bounds/RHS
     * - **Cutting planes**: Add constraints during solve
     * - **Model export**: Write to .lp, .mps, .sav formats
     * 
     * **CPLEX vs. Open-Source Alternatives:**
     * 
     * CPLEX is a commercial solver requiring a license. For open-source distribution,
     * consider replacing with:
     * - **COIN-OR CLP**: Open-source LP solver (good performance)
     * - **GLPK**: GNU Linear Programming Kit (easy to install)
     * - **HiGHS**: Modern open-source solver (excellent performance)
     * 
     * To migrate to CLP, implement a `CLP_solver` class following the same
     * `LP_solver` interface (see sync_checker_solver README for guidance).
     * 
     * @note Memory management: Automatically frees CPLEX environment on destruction
     * @note Thread safety: Each instance has its own CPLEX environment
     * 
     * @see LP_solver for interface documentation
     * @see CPX_model_structure for CPLEX data format
     */
    class CPX_solver: public LP_solver
    {
    protected:
        CPXENVptr env_;     ///< CPLEX environment pointer
        CPXLPptr problem_;  ///< CPLEX problem pointer

    public:
        /**
         * @brief Construct CPLEX solver from model
         * @param model Problem description
         * @param tol Numerical tolerance (default 1e-6)
         * @note Initializes CPLEX environment and builds problem
         */
        CPX_solver(const model_description &model, const double tol = 1e-6);
        
        /**
         * @brief Destructor - frees CPLEX environment and problem
         */
        virtual ~CPX_solver(void);

        /**
         * @brief Solve the optimization problem
         * @note Calls solve_LP() or solve_MIP() based on problem type
         */
        void solve(void);

        /**
         * @brief Get dual variable values (shadow prices)
         * @param alpha [output] Array to store dual values
         * @note Only valid after LP solve with optimal solution
         */
        void get_dual_vars(double *alpha) const;
        
        /**
         * @brief Get primal variable values
         * @param alpha [output] Array to store variable values
         */
        void get_vars(double *alpha) const;

        void set_obj(double *obj_coef, int *obj_inx, int sz);
        void set_bdn(double *obj_coef, char *sense, int *obj_inx, int sz);
        void set_rhs(int cnt, const int *rhs_inx, const double *rhs_val);
        void set_coef(int cnt, const int *row_inx, const int *col_inx, const double *coef_val);

        void add_cut(int nzcnt, double const *rhs, char const *sense, int const *rmatbeg, int const *rmatind, double const *rmatval, char **rowname);

        void set_rhs(const int row, const double val);
        
        /**
         * @brief Disable CPLEX linear preprocessing
         * @note Useful for debugging or when presolve causes issues
         */
        void disable_prep_linear(void);

        void del_rows(int begin, int end);

        /**
         * @brief Get number of non-zero coefficients in constraint matrix
         * @return Number of non-zeros
         */
        int get_nz(void) const;

        /**
         * @brief Get objective function value
         * @return Objective value (-1e20 if unbounded, 1e20 if infeasible)
         */
        double get_obj(void) const;

        /**
         * @brief Initialize CPLEX solver environment
         * @note Called automatically by constructor
         */
        void init_solver(void);

        /**
         * @brief Clear solver and free memory
         * @note Closes CPLEX environment
         */
        void clear(void);

        /**
         * @brief Build CPLEX model from structure
         * @param model CPLEX-specific model structure
         */
        void build_model(const CPX_model_structure &model);

        /**
         * @brief Solve as Linear Program (simplex/barrier)
         */
        void solve_LP(void);
        
        /**
         * @brief Solve as Mixed Integer Program (branch-and-cut)
         */
        void solve_MIP(void);

        /**
         * @brief Get current number of rows in problem
         * @return Number of constraints (may change if cuts added)
         */
        int get_n_rows(void) const;

        /**
         * @brief Write model to file
         * @param filename Output file (.lp, .mps, .sav, etc.)
         * @note Format determined by file extension
         */
        void write_model(const char *filename) const;
    };
}