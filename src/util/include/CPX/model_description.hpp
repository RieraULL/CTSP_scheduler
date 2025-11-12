/**
 * @file model_description.hpp
 * @brief Mathematical optimization model representation
 * 
 * This module defines a generic representation for Linear Programming (LP)
 * and Mixed Integer Programming (MIP) models, independent of any specific solver.
 */

#pragma once

#include "matrix.hpp"
#include <vector>
#include <string>
#include <utility>

using namespace std;

namespace GOMA
{
    /**
     * @enum ObjSen
     * @brief Optimization objective sense
     */
    enum ObjSen
    {
        Minimize, ///< Minimize the objective function
        Maximize  ///< Maximize the objective function
    };

    /**
     * @enum VarBnd
     * @brief Variable bound types
     */
    enum VarBnd
    {
        L,        ///< Lower bound only (x ≥ L)
        G,        ///< Greater than (x ≥ G)
        U,        ///< Upper bound only (x ≤ U)
        Binary,   ///< Binary variable (x ∈ {0, 1})
        LBounded, ///< Lower bounded
        GBounded, ///< Greater or equal bounded
        UBounded  ///< Upper bounded
    };

    /**
     * @enum VarType
     * @brief Variable types for optimization
     */
    enum VarType
    {
        C, ///< Continuous variable
        B, ///< Binary variable
        I, ///< Integer variable
        S  ///< Semi-continuous variable
    };

    /**
     * @enum ProbType
     * @brief Problem types
     */
    enum ProbType
    {
        LP,  ///< Linear Programming
        MIP  ///< Mixed Integer Programming
    };

    /**
     * @class model_description
     * @brief Generic representation of an optimization model
     * 
     * This class provides a solver-independent representation of LP/MIP models.
     * It stores:
     * - Decision variables with bounds and types
     * - Constraints with coefficients and right-hand sides
     * - Objective function
     * - Variable and constraint labels
     * 
     * The model can represent problems in the form:
     * ```
     * Minimize/Maximize:  c^T x
     * Subject to:         Ax {≤,=,≥} b
     *                     l ≤ x ≤ u
     *                     x_i ∈ {C, B, I, S}
     * ```
     * 
     * @note This class can be used to construct primal/dual pairs
     * @note Compatible with CPLEX and CLP solvers through adapter classes
     */
    class model_description
    {
    public:
        string name_; ///< Model name

        size_t n_col_; ///< Number of decision variables (columns)
        size_t n_row_; ///< Number of constraints (rows)

        vector<double> obj_;                     ///< Objective function coefficients
        vector<int> bd_;                         ///< Variable bound types (VarBnd enum)
        vector<pair<double, double>> bounds_;    ///< Variable bounds (lower, upper)
        vector<int> ctype_;                      ///< Variable types (VarType enum)

        vector<char> sense_; ///< Constraint senses ('L', 'E', 'G' for ≤, =, ≥)
        vector<double> rhs_; ///< Right-hand side values

        matrix<double> M_; ///< Constraint matrix (A in Ax {≤,=,≥} b)
        int nz_;           ///< Number of non-zero coefficients in M_

        vector<string> var_labels_;  ///< Variable names
        vector<string> cons_labels_; ///< Constraint names

        int obj_sense_; ///< Objective sense (ObjSen enum)

        int prob_type_; ///< Problem type (ProbType enum)

    public:
        /**
         * @brief Default constructor
         */
        model_description(void);
        
        /**
         * @brief Destructor
         */
        virtual ~model_description(void);

        // Getters
        inline int get_n_col(void) const { return n_col_; }
        inline int get_n_row(void) const { return n_row_; }
        inline const vector<double> &get_obj(void) const { return obj_; }
        inline const vector<int> &get_bd(void) const { return bd_; }
        inline const vector<int> &get_ctype(void) const { return ctype_; }
        inline const vector<char> &get_sense(void) const { return sense_; }
        inline const vector<double> &get_rhs(void) const { return rhs_; }
        inline const matrix<double> &get_M(void) const { return M_; }
        inline int get_nz(void) const { return nz_; }
        inline const vector<string> &get_var_labels(void) const { return var_labels_; }
        inline const vector<string> &get_cons_labels(void) const { return cons_labels_; }
        inline int get_obj_sense(void) const { return obj_sense_; }
        inline int get_prob_type(void) const { return prob_type_; }
        inline string get_name(void) const { return name_; }
        inline const vector<pair<double, double>> &get_bounds(void) const { return bounds_; }

        // Setters
        void set_n_col(int n_col) { n_col_ = n_col; }
        void set_n_row(int n_row) { n_row_ = n_row; }
        void set_obj(const vector<double> &obj) { obj_ = obj; }
        void set_bd(const vector<int> &bd) { bd_ = bd; }
        void set_sense(const vector<char> &sense) { sense_ = sense; }
        void set_rhs(const vector<double> &rhs) { rhs_ = rhs; }
        void set_M(const matrix<double> &M) { M_ = M; }
        void set_nz(int nz) { nz_ = nz; }
        void set_var_labels(const vector<string> &var_labels) { var_labels_ = var_labels; }
        void set_cons_labels(const vector<string> &cons_labels) { cons_labels_ = cons_labels; }
        void set_obj_sense(int obj_sense) { obj_sense_ = obj_sense; }
        void set_prob_type(int prob_type) { prob_type_ = prob_type; }

    protected:
        /**
         * @brief Construct dual model from primal model
         * @param primal Primal model
         * @note Automatically transposes constraints and swaps objective/RHS
         */
        void set_dual_(const model_description &primal);
        
        /**
         * @brief Set constraint senses and RHS from primal model
         * @param primal Primal model
         */
        void set_cons_sense_rhs_(const model_description &primal);
        
        /**
         * @brief Set objective and bounds from primal model
         * @param primal Primal model
         */
        void set_obj_lb_ub(const model_description &primal);

    };
}