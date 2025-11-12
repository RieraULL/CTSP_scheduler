/**
 * @file model_description.hpp
 * @brief Mathematical optimization model representation
 * 
 * Generic, solver-independent representation for LP/MIP models.
 */

#pragma once

#include "matrix.hpp"
#include <vector>
#include <string>
#include <utility>

using namespace std;

namespace GOMA
{
    enum ObjSen { Minimize, Maximize };

    enum VarBnd { L, G, U, Binary, LBounded, GBounded, UBounded };

    enum VarType { C, B, I, S };

    enum ProbType { LP, MIP };

    class model_description
    {
    public:
        string name_;

        size_t n_col_;
        size_t n_row_;

        vector<double> obj_;
        vector<int> bd_;
        vector<pair<double, double>> bounds_;
        vector<int> ctype_;

        vector<char> sense_;
        vector<double> rhs_;

        matrix<double> M_;
        int nz_;

        vector<string> var_labels_;
        vector<string> cons_labels_;

        int obj_sense_;
        int prob_type_;

    public:
        model_description(void);
        virtual ~model_description(void);

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
        void set_dual_(const model_description &primal);
        void set_cons_sense_rhs_(const model_description &primal);
        void set_obj_lb_ub(const model_description &primal);
    };
}
