/**
 * @file CLP_model_structure.hpp
 * @brief COIN-OR CLP-compatible model data structure
 * 
 * This module converts a generic model_description into CLP-specific
 * data structures required by the CLP C++ API.
 * 
 * CLP (COIN-OR Linear Programming) is an open-source LP solver that
 * can serve as a replacement for CPLEX in academic/open-source projects.
 */

#pragma once

#include "model_description.hpp"

namespace GOMA
{
    /**
     * @class CLP_model_structure
     * @brief CLP C++ API data structure wrapper
     * 
     * This class converts a model_description into the specific arrays
     * required by CLP's C++ API. CLP uses a column-oriented sparse matrix
     * format similar to CPLEX:
     * 
     * - Column-oriented sparse storage (CoinPackedMatrix)
     * - Separate arrays for lower/upper bounds
     * - Row/column names as std::string vectors
     * 
     * Unlike CPLEX which uses raw pointers, CLP uses STL containers and
     * COIN utility classes, making memory management safer.
     * 
     * @note Memory is managed by this class
     * @note This class is specific to CLP; use model_description for solver-independent code
     * 
     * @see model_description for generic model representation
     * @see CLP_solver for the solver that uses this structure
     */
    class CLP_model_structure
    {
    public:
        string probname_;  ///< Problem name
        
        int ncol_;     ///< Number of columns (variables)
        int nrow_;     ///< Number of rows (constraints)
        
        vector<double> rhs_;   ///< Right-hand side values
        vector<char> sense_;   ///< Constraint senses ('L', 'E', 'G')
        
        vector<int> matbeg_;    ///< Column start indices in sparse matrix
        vector<int> matind_;    ///< Row indices of non-zero coefficients
        vector<int> matcnt_;    ///< Number of non-zeros per column
        vector<double> matval_; ///< Non-zero coefficient values
        
        vector<double> obj_;   ///< Objective function coefficients
        vector<double> lb_;    ///< Lower bounds on variables
        vector<double> ub_;    ///< Upper bounds on variables
        vector<char> ctype_;   ///< Variable types ('C', 'B', 'I', 'S')
        
        vector<string> colname_; ///< Column names
        vector<string> rowname_; ///< Row names
        
        int obj_sense_;  ///< Objective sense (minimize=-1, maximize=+1)
        int prob_type_;  ///< Problem type (LP or MIP)

    public:
        /**
         * @brief Construct CLP model structure from generic model
         * @param model Generic model description
         * @param tol Numerical tolerance for constraint bounds
         */
        CLP_model_structure(const model_description& model, double tol = 1e-3);
        
        /**
         * @brief Destructor - frees all allocated memory
         */
        virtual ~CLP_model_structure(void);  

        // Getters for CLP API
        inline const string& get_probname(void) const {return probname_;}

        inline int ncol(void) const {return ncol_;}
        inline int nrow(void) const {return nrow_;}
        inline const vector<double>& get_rhs(void) const {return rhs_;}
        inline const vector<char>& get_sense(void) const {return sense_;}
        inline const vector<int>& get_matbeg(void) const {return matbeg_;}
        inline const vector<int>& get_matind(void) const {return matind_;}
        inline const vector<int>& get_matcnt(void) const {return matcnt_;}
        inline const vector<double>& get_matval(void) const {return matval_;}
        inline const vector<double>& get_obj(void) const {return obj_;}
        inline const vector<double>& get_lb(void) const {return lb_;}
        inline const vector<double>& get_ub(void) const {return ub_;}
        inline const vector<char>& get_ctype(void) const {return ctype_;}
        inline const vector<string>& get_colname(void) const {return colname_;}
        inline const vector<string>& get_rowname(void) const {return rowname_;}
        inline int get_obj_sense(void) const {return obj_sense_;}
        inline int get_prob_type(void) const {return prob_type_;}
    };    
}
