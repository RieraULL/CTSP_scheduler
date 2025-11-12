/**
 * @file sync_checker_solver.cpp
 * @brief Implementation of LP solver wrapper for CTSP synchronization checking
 * 
 * This file implements the sync_checker_solver class, which provides a
 * simplified interface to CPLEX for solving linear programming problems
 * in CTSP constraint checking and separation algorithms.
 */

#include "sync_checker_solver.hpp"
#ifdef USE_CLP
#include "CLP/CLP_solver.hpp"
#else
#include "CPX_solver.hpp"
#endif

namespace GOMA
{
    /**
     * Constructor: Creates solver with initial model description.
     * Internally instantiates a CPLEX solver with the provided model.
     */
    sync_checker_solver::sync_checker_solver(const model_description &model, const double tol) 
#ifdef USE_CLP
        : solver_(new CLP_solver(model, tol))
#else
        : solver_(new CPX_solver(model, tol))
#endif
    {
    }

    /**
     * Default constructor: Creates an empty solver.
     * Must call set() before using.
     */
    sync_checker_solver::sync_checker_solver(void) : solver_(NULL)
    {
    }    

    /**
     * Destructor: Properly releases CPLEX solver resources.
     */
    sync_checker_solver::~sync_checker_solver(void)
    {
        if (solver_)
        {
            delete solver_;
        }
    }

    /**
     * Initialize or reinitialize the solver with a new model.
     * Deletes any existing solver and creates a fresh instance.
     */
    void sync_checker_solver::set(const model_description &model, const double tol)
    {
        if (solver_)
        {
            delete solver_;
        }

        
#ifdef USE_CLP
    solver_ = new CLP_solver(model, tol);
#else
    solver_ = new CPX_solver(model, tol);
#endif
    }

    /**
     * Solve the LP problem using CPLEX's optimizer.
     * After solving, query solution via get_obj(), get_vars(), get_dual_vars().
     */
    void sync_checker_solver::solve(void)
    {
        solver_->solve();
    }

    /**
     * Get the number of non-zero entries in the constraint matrix.
     * Useful for analyzing model sparsity.
     */
    int sync_checker_solver::get_nz(void) const
    {
        return solver_->get_nz();
    }

    /**
     * Retrieve dual variable values (shadow prices).
     * Used for reduced cost computation and constraint generation.
     */
    void sync_checker_solver::get_dual_vars(double *alpha) const
    {
        solver_->get_dual_vars(alpha);
    }

    /**
     * Retrieve primal variable values from the solution.
     */
    void sync_checker_solver::get_vars(double *alpha) const
    {
        solver_->get_vars(alpha);
    }

    /**
     * Modify objective function coefficients.
     * Useful for parametric optimization and sensitivity analysis.
     */
    void sync_checker_solver::set_obj(double *obj_coef, int *obj_inx, int sz)
    {
        solver_->set_obj(obj_coef, obj_inx, sz);
    }

    /**
     * Modify variable bounds (lower, upper, or both).
     */
    void sync_checker_solver::set_bdn(double *obj_coef, char *sense, int *obj_inx, int sz)
    {
        solver_->set_bdn(obj_coef, sense, obj_inx, sz);
    }

    /**
     * Modify right-hand side values of constraints.
     * Used in branch-and-bound for node-specific constraints.
     */
    void sync_checker_solver::set_rhs(int cnt, const int *rhs_inx, const double *rhs_val)
    {
        solver_->set_rhs(cnt, rhs_inx, rhs_val);
    }

    /**
     * Get the optimal objective value from the solved LP.
     */
    double sync_checker_solver::get_obj(void) const
    {
        return solver_->get_obj();
    }

    /**
     * Get the LP solution status code.
     * Returns CPLEX status codes (1=optimal, 3=infeasible, etc.)
     */
    int sync_checker_solver::get_lp_stat(void) const
    {
        return solver_->get_lp_stat();
    }

    /**
     * Add a new linear constraint (cut) to the model dynamically.
     * This is the core method for implementing branch-and-cut algorithms.
     * 
     * The constraint is added in the form:
     * sum(lhs_coef[i] * x[lhs[i]]) {sense} rhs
     * 
     * where sense is 'L' (≤), 'G' (≥), or 'E' (=)
     */
    void sync_checker_solver::add_cut(const vector<int> &lhs, const vector<double> &lhs_coef, 
                                      const double rhs_v, const char sense_v, const string &name)
    {
        int nzcnt{(int)lhs.size()};
        double rhs[]{{rhs_v}};
        char sense[]{{sense_v}};
        int rmatbeg[]{{0}};
        int rmatind[nzcnt];
        double rmatval[nzcnt];
        char *rowname[]{{(char *)name.c_str()}};

        // Build constraint in CPLEX format
        for (int i{0}; i < nzcnt; i++)
        {
            rmatind[i] = lhs[i];
            rmatval[i] = lhs_coef[i];
        }

        solver_->add_cut(nzcnt, rhs, sense, rmatbeg, rmatind, rmatval, rowname);
    }

    /**
     * Write the current LP model to a file for debugging and analysis.
     * File format is determined by extension (.lp, .mps, .sav, etc.)
     */
    void sync_checker_solver::write_model(const char *filename) const
    {
        solver_->write_model(filename);
    }

    /**
     * Modify specific coefficients in the constraint matrix.
     * Used for dynamic model updates during branch-and-cut.
     */
    void sync_checker_solver::set_coef(int cnt, const int *row_inx, const int *col_inx, const double *coef_val)
    {
        solver_->set_coef(cnt, row_inx, col_inx, coef_val);
    }

}