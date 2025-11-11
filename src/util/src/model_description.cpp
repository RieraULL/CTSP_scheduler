#include "model_description.hpp"

namespace GOMA
{
    model_description::model_description(void) : name_("CBC"),
                                                 n_col_(0),
                                                 n_row_(0),
                                                 obj_(),
                                                 bd_(),
                                                 bounds_(),
                                                 ctype_(),
                                                 sense_(),
                                                 rhs_(),
                                                 M_(),
                                                 nz_(0),
                                                 var_labels_(),
                                                 cons_labels_(),
                                                 obj_sense_(Minimize),
                                                 prob_type_(ProbType::LP)
    {
    }

    model_description::~model_description(void)
    {
    }

    void model_description::set_dual_(const model_description &primal)
    {
        set_n_col(primal.get_n_row());
        set_n_row(primal.get_n_col());

        matrix<double> M;
        primal.get_M().transpose(M);
        set_M(M);
        set_nz(primal.get_nz());

        set_var_labels(primal.get_cons_labels());
        set_cons_labels(primal.get_var_labels());

        set_obj_sense((primal.get_obj_sense() == Minimize) ? Maximize : Minimize);

        set_prob_type(LP);

        set_cons_sense_rhs_(primal);

        set_obj_lb_ub(primal);
    }

    void model_description::set_cons_sense_rhs_(const model_description &primal)
    {
        const int n_model_col = primal.get_n_col();

        const vector<int> &BD = primal.get_bd();
        const vector<double> &obj = primal.get_obj();

        sense_.resize(n_model_col);
        rhs_.resize(n_model_col);

        const int obj_sense = primal.obj_sense_;

        for (int i{0}; i < n_model_col; i++)
        {
            rhs_[i] = obj[i];

            if ((BD[i] == VarBnd::U) || (BD[i] == VarBnd::UBounded))
            {
                sense_[i] = 'E';
            }
            else if ((BD[i] == VarBnd::L) || (BD[i] == VarBnd::LBounded))
            {
                if (obj_sense == Minimize)
                {
                    sense_[i] = 'G';
                }
                else
                {
                    sense_[i] = 'L';
                }
            }
            else if ((BD[i] == VarBnd::G) || (BD[i] == VarBnd::GBounded))
            {
                if (obj_sense == Minimize)
                {
                    sense_[i] = 'L';
                }
                else
                {
                    sense_[i] = 'G';
                }
            }
        }
    }

    void model_description::set_obj_lb_ub(const model_description &primal)
    {
        const double bound{1};

        const int n_model_row = primal.get_n_row();

        obj_.resize(n_model_row);
        bd_.resize(n_model_row);

        bounds_.resize(n_model_row);

        const vector<double> &rhs = primal.get_rhs();
        const vector<char> &sense = primal.get_sense();
        const int obj_sense = primal.get_obj_sense();

        for (int i{0}; i < n_model_row; i++)
        {
            obj_[i] = rhs[i];

            if (sense[i] == 'E')
            {
                bd_[i] = VarBnd::UBounded;
                bounds_[i] = make_pair(-bound, bound);
            }
            else if (sense[i] == 'L')
            {
                if (obj_sense == Minimize)
                {
                    bd_[i] = VarBnd::LBounded;
                    bounds_[i] = make_pair(-bound, 0.0);
                }
                else
                {
                    bd_[i] = VarBnd::GBounded;
                    bounds_[i] = make_pair(0.0, bound);
                }
            }
            else if (sense[i] == 'G')
            {
                if (obj_sense_ == Minimize)
                {
                    bd_[i] = VarBnd::GBounded;
                    bounds_[i] = make_pair(0.0, bound);
                }
                else
                {
                    bd_[i] = VarBnd::L;
                    bounds_[i] = make_pair(-bound, 0.0);
                }
            }
        }
    }

}