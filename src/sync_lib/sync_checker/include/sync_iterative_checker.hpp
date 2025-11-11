#pragma once

#include "ctsp_sync_checker.hpp"
#include "sync_model_a_builder.hpp"

/**
 * @file sync_iterative_checker.hpp
 * @brief Template wrapper for iterative synchronization checking
 * 
 * This file provides a template class that wraps any synchronization checker
 * (e.g., ctsp_sync_checker, ctsp_lb_sync_checker) to provide convenient
 * methods for iterative feasibility checking in branch-and-cut algorithms.
 */

namespace SYNC_LIB
{
    /**
     * @class sync_iterative_checker
     * @brief Template wrapper for iterative constraint checking
     * 
     * This template class extends any synchronization checker type T with
     * convenient interfaces for use in iterative optimization algorithms.
     * It provides multiple overloaded versions of `is_feasible()` that:
     * 
     * - Check feasibility and extract dual variables if infeasible
     * - Check feasibility and extract slack variables if feasible
     * - Work with pre-set solutions (no x parameter needed)
     * 
     * The template pattern allows the same iterative interface to be used
     * with different underlying checker types (standard checker, lower bound
     * checker, etc.) without code duplication.
     * 
     * Typical usage in branch-and-cut:
     * ```cpp
     * sync_iterative_checker<ctsp_sync_checker> checker(builder, tol);
     * 
     * if (!checker.is_feasible(x, alpha, beta, gamma)) {
     *     // Infeasible - use duals to generate cut
     *     add_cut_from_duals(alpha, beta, gamma);
     * }
     * ```
     * 
     * @tparam T The base synchronization checker type (must derive from ctsp_sync_checker)
     */
    template <class T>
    class sync_iterative_checker : public T
    {
    public:
        /**
         * @brief Construct iterative checker
         * @param builder Model A builder
         * @param tol Numerical tolerance
         */
        sync_iterative_checker(const sync_model_a_builder &builder, double tol) : T(builder, tol)
        {
        }

        /**
         * @brief Default constructor
         */
        sync_iterative_checker(void) : T()
        {
        }

        virtual ~sync_iterative_checker(void)
        {
        }

        /**
         * @brief Check feasibility and extract dual variables if infeasible
         * @param x Routing solution (arc variables)
         * @param alpha Output: α dual variables (if infeasible)
         * @param beta Output: β dual variables (if infeasible)
         * @param gamma Output: γ dual variables (if infeasible)
         * @return true if synchronization is feasible, false otherwise
         * 
         * This method is useful when you want to generate cuts immediately
         * upon detecting infeasibility, as it provides the dual variables
         * needed for cut generation in a single call.
         */
        bool is_feasible(const vector<double> &x, vector<double> &alpha, vector<double> &beta, vector<double> &gamma)
        {
            const bool feasible{T::is_feasible_(x)};

            if (!feasible)
            {
                T::get_alpha_beta_gamma(alpha, beta, gamma);
            }

            return feasible;
        }

        /**
         * @brief Check feasibility and extract slack variables if feasible
         * @param x Routing solution
         * @param s Output: slack variable values (if feasible)
         * @return true if feasible
         * 
         * This variant extracts slack variables from feasible solutions,
         * which can be useful for:
         * - Analyzing tightness of constraints
         * - Warm-starting subsequent solves
         * - Sensitivity analysis
         */
        bool is_feasible(const vector<double> &x, vector<double> &s)
        {
            const bool feasible{T::is_feasible_(x)};

            if (feasible)
            {
                T::get_s(s);
            }

            return feasible;
        }

        /**
         * @brief Check feasibility using pre-set solution and extract duals
         * @param inx Index parameter (currently unused, for future extensions)
         * @param alpha Output: α dual variables (if infeasible)
         * @param beta Output: β dual variables (if infeasible)
         * @param gamma Output: γ dual variables (if infeasible)
         * @return true if feasible
         * 
         * This variant assumes the solution has been set previously
         * (e.g., via RHS modification) and checks feasibility without
         * needing to pass x explicitly.
         */
        bool is_feasible(const int inx, vector<double> &alpha, vector<double> &beta, vector<double> &gamma)
        {
            const bool feasible{T::is_feasible_()};

            if (!feasible)
            {
                T::get_vars(T::alpha_);
                T::get_alpha_beta_gamma_(T::alpha_, alpha, beta, gamma);
            }

            return feasible;
        }
    };
}