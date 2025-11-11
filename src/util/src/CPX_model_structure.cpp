#include "CPX_model_structure.hpp"

#include <cmath>
#include <cstring>

#pragma GCC diagnostic ignored "-Wignored-attributes"

#define IL_STD
#include <ilcplex/ilocplex.h>

namespace GOMA
{
    CPX_model_structure::CPX_model_structure(const model_description &model, double tol) : probname_p_(NULL),
                                                                                           ncol_(0),
                                                                                           nrow_(0),
                                                                                           rhs_(NULL),
                                                                                           sense_(NULL),
                                                                                           matbeg_(NULL),
                                                                                           matind_(NULL),
                                                                                           matcnt_(NULL),
                                                                                           matval_(NULL),
                                                                                           obj_(NULL),
                                                                                           lb_(NULL),
                                                                                           ub_(NULL),
                                                                                           ctype_(NULL),
                                                                                           colname_(NULL),
                                                                                           rowname_(NULL),
                                                                                           obj_sense_(0),
                                                                                           prob_type_(0)
    {
        ncol_ = model.get_n_col();
        nrow_ = model.get_n_row();

        obj_ = new double[ncol_];
        lb_ = new double[ncol_];
        ub_ = new double[ncol_];
        ctype_ = new char[ncol_];

        prob_type_ = model.get_prob_type();

        const vector<double> &obj = model.get_obj();
        const vector<int> &bd = model.get_bd();
        const vector<int> &ctype = model.get_ctype();
        const vector<pair<double, double>> &bounds = model.get_bounds();

        //const double var_ub = 1.0 / tol;
        

        for(int i{0}; i < ncol_; ++i)
        {
            const double var_lb = bounds[i].first;
            const double var_ub = bounds[i].second;

            obj_[i] = obj[i];

            if (bd[i] == VarBnd::U)
            {
                lb_[i] = -IloInfinity;
                ub_[i] = IloInfinity;
            }
            else if (bd[i] == VarBnd::L)
            {
                lb_[i] = -IloInfinity;
                ub_[i] = 0.0;
            }
            else if (bd[i] == VarBnd::G)
            {
                lb_[i] = 0.0;
                ub_[i] = IloInfinity;
            }
            else if (bd[i] == VarBnd::Binary)
            {
                lb_[i] = 0.0;
                ub_[i] = 1.0;
            }
            else if (bd[i] == VarBnd::LBounded)
            {
                lb_[i] = var_lb;
                ub_[i] = var_ub;
            }
            else if (bd[i] == VarBnd::GBounded)
            {
                lb_[i] = var_lb;
                ub_[i] = var_ub;
            }
            else if (bd[i] == VarBnd::UBounded)
            {
                lb_[i] = var_lb;
                ub_[i] = var_ub;
            }

            if (ctype.size() == 0)
                ctype_[i] = 'C';
            else if (ctype[i] == VarType::C)
                ctype_[i] = 'C';
            else if (ctype[i] == VarType::B)
                ctype_[i] = 'B';
            else if (ctype[i] == VarType::I)
                ctype_[i] = 'I';
            else if (ctype[i] == VarType::S)
                ctype_[i] = 'S';
        }

        const vector<char> &sense = model.get_sense();
        const vector<double> &rhs = model.get_rhs();

        sense_ = new char[nrow_];
        rhs_ = new double[nrow_];

        for(int i{0}; i < nrow_; ++i)
        {
            sense_[i] = sense[i];
            rhs_[i] = rhs[i];
        }

        const int M_nz = model.get_nz();
        const matrix<double> &M = model.get_M();

        matval_ = new double[M_nz];
        matind_ = new int[M_nz];
        matcnt_ = new int[ncol_ + 1];
        matbeg_ = new int[ncol_ + 1];

        matbeg_[0] = 0;

        int nz = 0;

        for(int j{0}; j < ncol_; j++)
        {
            matcnt_[j] = 0;

            for(int i{0}; i < nrow_; i++)
            {
                if (fabs(M(i + 1, j + 1)) > tol)
                {
                    matval_[nz] = M(i + 1, j + 1);
                    matind_[nz] = i;
                    nz++;

                    matcnt_[j]++;
                }
            }
            matbeg_[j + 1] = matbeg_[j] + matcnt_[j];
        }

        if (nz != M_nz)
        {
            cerr << "Error: nz != M_nz" << endl;
            exit(1);
        }

        const vector<string> &colname = model.get_var_labels();

        colname_ = new char *[ncol_];

        const int colname_size = colname.size();

        const int ncol = colname_size < ncol_ ? colname_size : ncol_;

        for(int i{0}; i < ncol; ++i)
        {
            colname_[i] = new char[colname[i].size() + 1];
            strcpy(colname_[i], colname[i].c_str());
        }

        for (int i{ncol}; i < ncol_; ++i)
        {
            colname_[i] = new char[10];
            sprintf(colname_[i], "aux_%d", i);
        }

        const vector<string> &rowname = model.get_cons_labels();

        rowname_ = new char *[nrow_];

        for(int i{0}; i < nrow_; ++i)
        {
            rowname_[i] = new char[rowname[i].size() + 1];
            strcpy(rowname_[i], rowname[i].c_str());
        }

        probname_p_ = new char[model.get_name().size() + 1];

        if (model.get_obj_sense() == ObjSen::Minimize)
            obj_sense_ = CPX_MIN;
        else
            obj_sense_ = CPX_MAX;

        strcpy(probname_p_, model.get_name().c_str());
    }

    CPX_model_structure::~CPX_model_structure(void)
    {
        if (obj_)
            delete[] obj_;

        if (lb_)
            delete[] lb_;

        if (ub_)
            delete[] ub_;

        if (ctype_)
            delete[] ctype_;

        if (sense_)
            delete[] sense_;

        if (rhs_)
            delete[] rhs_;

        if (matbeg_)
            delete[] matbeg_;

        if (matind_)
            delete[] matind_;

        if (matcnt_)
            delete[] matcnt_;

        if (matval_)
            delete[] matval_;

        if (probname_p_)
            delete[] probname_p_;

        if (colname_)
        {
            for(int i{0}; i < ncol_; ++i)
                delete[] colname_[i];

            delete[] colname_;
        }

        if (rowname_)
        {
            for(int i{0}; i < nrow_; ++i)
                delete[] rowname_[i];

            delete[] rowname_;
        }
    }
}