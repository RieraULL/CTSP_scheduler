#include "CLP/CLP_model_structure.hpp"

#include <cmath>
#include <limits>

namespace GOMA
{
    CLP_model_structure::CLP_model_structure(const model_description &model, double tol) : probname_(model.get_name()),
                                                                                           ncol_(0),
                                                                                           nrow_(0),
                                                                                           obj_sense_(0),
                                                                                           prob_type_(0)
    {
        ncol_ = model.get_n_col();
        nrow_ = model.get_n_row();

        obj_.resize(ncol_);
        lb_.resize(ncol_);
        ub_.resize(ncol_);
        ctype_.resize(ncol_);

        prob_type_ = model.get_prob_type();

        const vector<double> &obj = model.get_obj();
        const vector<int> &bd = model.get_bd();
        const vector<int> &ctype = model.get_ctype();
        const vector<pair<double, double>> &bounds = model.get_bounds();

        // CLP uses numeric_limits for infinity
        const double CLP_INFINITY = std::numeric_limits<double>::infinity();

        for(int i{0}; i < ncol_; ++i)
        {
            const double var_lb = bounds[i].first;
            const double var_ub = bounds[i].second;

            obj_[i] = obj[i];

            if (bd[i] == VarBnd::U)
            {
                lb_[i] = -CLP_INFINITY;
                ub_[i] = CLP_INFINITY;
            }
            else if (bd[i] == VarBnd::L)
            {
                lb_[i] = -CLP_INFINITY;
                ub_[i] = 0.0;
            }
            else if (bd[i] == VarBnd::G)
            {
                lb_[i] = 0.0;
                ub_[i] = CLP_INFINITY;
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

        sense_.resize(nrow_);
        rhs_.resize(nrow_);

        for(int i{0}; i < nrow_; ++i)
        {
            sense_[i] = sense[i];
            rhs_[i] = rhs[i];
        }

        const int M_nz = model.get_nz();
        const matrix<double> &M = model.get_M();

        matval_.reserve(M_nz);
        matind_.reserve(M_nz);
        matcnt_.resize(ncol_);
        matbeg_.resize(ncol_ + 1);

        matbeg_[0] = 0;

        int nz = 0;

        for(int j{0}; j < ncol_; j++)
        {
            matcnt_[j] = 0;

            for(int i{0}; i < nrow_; i++)
            {
                if (fabs(M(i + 1, j + 1)) > tol)
                {
                    matval_.push_back(M(i + 1, j + 1));
                    matind_.push_back(i);
                    nz++;

                    matcnt_[j]++;
                }
            }
            matbeg_[j + 1] = matbeg_[j] + matcnt_[j];
        }

        if (nz != M_nz)
        {
            cerr << "Error: nz != M_nz (" << nz << " != " << M_nz << ")" << endl;
            exit(1);
        }

        const vector<string> &colname = model.get_var_labels();

        colname_.resize(ncol_);

        const int colname_size = colname.size();
        const int ncol = colname_size < ncol_ ? colname_size : ncol_;

        for(int i{0}; i < ncol; ++i)
        {
            colname_[i] = colname[i];
        }

        for (int i{ncol}; i < ncol_; ++i)
        {
            colname_[i] = "aux_" + std::to_string(i);
        }

        const vector<string> &rowname = model.get_cons_labels();

        rowname_.resize(nrow_);

        for(int i{0}; i < nrow_; ++i)
        {
            rowname_[i] = rowname[i];
        }

        if (model.get_obj_sense() == ObjSen::Minimize)
            obj_sense_ = 1;  // CLP: 1=minimize, -1=maximize
        else
            obj_sense_ = -1;
    }

    CLP_model_structure::~CLP_model_structure(void)
    {
        // Vectors handle their own memory cleanup
    }
}
