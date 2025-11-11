/**
 * @file CPX_model_structure.hpp
 * @brief CPLEX-compatible model data structure
 * 
 * This module converts a generic model_description into CPLEX-specific
 * data structures required by the CPLEX C API.
 */

#pragma once

#include "model_description.hpp"

namespace GOMA
{
    /**
     * @class CPX_model_structure
     * @brief CPLEX C API data structure wrapper
     * 
     * This class converts a model_description into the specific arrays and
     * pointers required by CPLEX's C API. CPLEX uses a column-oriented
     * sparse matrix format where:
     * 
     * - `matbeg[j]` = index where column j starts in matind/matval
     * - `matcnt[j]` = number of non-zeros in column j
     * - `matind[k]` = row index of coefficient k
     * - `matval[k]` = value of coefficient k
     * 
     * All string data is converted to C-style char* pointers for CPLEX compatibility.
     * 
     * @note Memory is managed by this class - all pointers are freed in destructor
     * @note This class is specific to CPLEX; use model_description for solver-independent code
     * 
     * @see model_description for generic model representation
     * @see CPX_solver for the solver that uses this structure
     */
    class CPX_model_structure
    {
    public:
        char *probname_p_; ///< Problem name (C-string)
        
        int ncol_;     ///< Number of columns (variables)
        int nrow_;     ///< Number of rows (constraints)
        
        double *rhs_;  ///< Right-hand side values
        char *sense_;  ///< Constraint senses ('L', 'E', 'G')
        
        int *matbeg_;  ///< Column start indices in sparse matrix
        int *matind_;  ///< Row indices of non-zero coefficients
        int *matcnt_;  ///< Number of non-zeros per column
        double *matval_; ///< Non-zero coefficient values
        
        double *obj_;  ///< Objective function coefficients
        double *lb_;   ///< Lower bounds on variables
        double *ub_;   ///< Upper bounds on variables
        char* ctype_;  ///< Variable types ('C', 'B', 'I', 'S')
        
        char** colname_; ///< Column names (array of C-strings)
        char** rowname_; ///< Row names (array of C-strings)
        
        int obj_sense_;  ///< Objective sense (CPX_MIN or CPX_MAX)
        int prob_type_;  ///< Problem type (LP or MIP)

    public:
        /**
         * @brief Construct CPLEX model structure from generic model
         * @param model Generic model description
         * @param tol Numerical tolerance for constraint bounds
         */
        CPX_model_structure(const model_description& model, double tol = 1e-3);
        
        /**
         * @brief Destructor - frees all allocated memory
         */
        virtual ~CPX_model_structure(void);  

        // Getters for CPLEX C API
        inline char *get_probname_p(void) const {return probname_p_;}

        inline int ncol(void) const {return ncol_;}
        inline int nrow(void) const {return nrow_;}
        inline double *get_rhs(void) const {return rhs_;}
        inline char *get_sense(void) const {return sense_;}
        inline int *get_matbeg(void) const {return matbeg_;}
        inline int *get_matind(void) const {return matind_;}
        inline int *get_matcnt(void) const {return matcnt_;}
        inline double *get_matval(void) const {return matval_;}
        inline double *get_obj(void) const {return obj_;}
        inline double *get_lb(void) const {return lb_;}
        inline double *get_ub(void) const {return ub_;}
        inline char *get_ctype(void) const {return ctype_;}
        inline char **get_colname(void) const {return colname_;}
        inline char **get_rowname(void) const {return rowname_;}
        inline int get_obj_sense(void) const {return obj_sense_;}
        inline int get_prob_type(void) const {return prob_type_;}
    };    
}