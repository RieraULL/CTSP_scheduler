/**
 * @file CLP_solver.hpp
 * @brief COIN-OR CLP solver implementation
 * 
 * This module provides a concrete implementation of the LP_solver interface
 * using COIN-OR CLP (COIN-OR Linear Programming).
 * 
 * @note CLP is open-source (Eclipse Public License)
 * @note Suitable replacement for CPLEX in academic/open-source projects
 */

#pragma once

#include "CLP/CLP_model_structure.hpp"
#include "LP_solver.hpp"

// Forward declarations for CLP classes to avoid heavy includes in header
class ClpSimplex;
class CoinPackedMatrix;

namespace GOMA
{
    /**
     * @class CLP_solver
     * @brief CLP-based LP solver implementation
     * 
     * This class provides a complete implementation of the LP_solver interface
     * using COIN-OR CLP. It wraps the CLP C++ API to provide:
     * 
     * - **LP solving**: Simplex (primal and dual) and barrier methods
     * - **Dual variables**: Shadow prices for sensitivity analysis
     * - **Model modification**: Dynamic changes to coefficients/bounds/RHS
     * - **Cutting planes**: Add constraints during solve
     * - **Model export**: Write to .lp, .mps formats
     * 
     * **CLP vs. CPLEX:**
     * 
     * CLP is an open-source LP solver from COIN-OR. Key differences:
     * - **License**: Open-source (EPL) vs commercial
     * - **Performance**: CPLEX typically faster, but CLP competitive
     * - **MIP**: CLP is LP-only; use CBC (COIN-OR Branch and Cut) for MIP
     * - **Memory**: CLP uses C++ STL containers vs raw pointers
     * 
     * **Installation:**
     * ```bash
     * # Ubuntu/Debian
     * sudo apt-get install coinor-libclp-dev
     * 
     * # From source
     * git clone https://github.com/coin-or/Clp
     * cd Clp && mkdir build && cd build
     * cmake .. && make && sudo make install
     * ```
     * 
     * **Linking:**
     * Add to CMakeLists.txt:
     * ```cmake
     * find_package(Clp REQUIRED)
     * target_link_libraries(your_target Clp::Clp)
     * ```
     * 
     * @note Memory management: Uses RAII with smart pointers
     * @note Thread safety: Each instance has its own CLP model
     * 
     * @see LP_solver for interface documentation
     * @see CLP_model_structure for CLP data format
     * @see https://github.com/coin-or/Clp for CLP documentation
     */
    class CLP_solver: public LP_solver
    {
    protected:
        ClpSimplex *model_;  ///< CLP model pointer (unique ownership)

    public:
        /**
         * @brief Construct CLP solver from model
         * @param model Problem description
         * @param tol Numerical tolerance (default 1e-6)
         * @note Initializes CLP model and loads problem
         */
        CLP_solver(const model_description &model, const double tol = 1e-6);
        
        /**
         * @brief Destructor - frees CLP model
         */
        virtual ~CLP_solver(void);

        /**
         * @brief Solve the optimization problem
         * @note Calls solve_LP() (CLP is LP-only; use CBC for MIP)
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
         * @brief Disable CLP preprocessing
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
         * @brief Initialize CLP solver
         * @note Called automatically by constructor
         */
        void init_solver(void);

        /**
         * @brief Clear solver and free memory
         */
        void clear(void);

        /**
         * @brief Build CLP model from structure
         * @param model CLP-specific model structure
         */
        void build_model(const CLP_model_structure &model);

        /**
         * @brief Solve as Linear Program
         * @note Uses dual simplex by default (usually fastest)
         */
        void solve_LP(void);
        
        /**
         * @brief Solve as Mixed Integer Program
         * @note NOT IMPLEMENTED in CLP - use CBC solver instead
         * @throw std::runtime_error Always throws - MIP not supported
         */
        void solve_MIP(void);

        /**
         * @brief Get current number of rows in problem
         * @return Number of constraints (may change if cuts added)
         */
        int get_n_rows(void) const;

        /**
         * @brief Write model to file
         * @param filename Output file (.lp, .mps, etc.)
         * @note Format determined by file extension
         */
        void write_model(const char *filename) const;

    private:
        /**
         * @brief Convert row lower/upper bounds from sense/rhs format
         * @param sense Constraint sense ('L', 'E', 'G')
         * @param rhs Right-hand side value
         * @param row_lower [output] Row lower bound
         * @param row_upper [output] Row upper bound
         */
        void convert_row_bounds(char sense, double rhs, double &row_lower, double &row_upper) const;
    };
}
