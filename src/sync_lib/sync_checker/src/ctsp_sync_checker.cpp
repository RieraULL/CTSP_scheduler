#include "ctsp_sync_checker.hpp"
#include <cassert>

#include "sync_checker_solver.hpp"

namespace SYNC_LIB
{

    ctsp_sync_checker::ctsp_sync_checker(const sync_model_a_builder &builder, const GOMA::model_description &model, const double tol) : sync_checker_solver(model, tol),
                                                                                                                                        tol_(tol),
                                                                                                                                        n_operations_(builder.get_n_operations()),
                                                                                                                                        n_routing_arcs_(builder.get_n_routing_arcs()),
                                                                                                                                        n_sync_arcs_(builder.get_n_sync_arcs()),
                                                                                                                                        n_customers_(builder.get_n_customers()),
                                                                                                                                        n_depots_(builder.get_n_depots()),
                                                                                                                                        n_col_(model.get_n_col()),
                                                                                                                                        routing_arc_resources_(new double[builder.get_routing_arc_times().size()]),
                                                                                                                                        operations_resources_(new double[n_operations_]),
                                                                                                                                        max_distance_(builder.get_max_distance()),
                                                                                                                                        routing_outbound_arcs_(builder.get_routing_outbound_arcs()),
                                                                                                                                        routing_inbound_arcs_(builder.get_routing_inbound_arcs()),
                                                                                                                                        operation_2_customer_(n_operations_, -1),
                                                                                                                                        routing_arcs_(builder.get_routing_arcs()),
                                                                                                                                        routing_arcs_pair_map_(builder.get_routing_arcs_pair_map()),
                                                                                                                                        col_inx_(new int[get_nz()]),
                                                                                                                                        row_inx_(new int[get_nz()]),
                                                                                                                                        coef_val_(new double[get_nz()]),
                                                                                                                                        sense_(new char[get_nz()]),
                                                                                                                                        alpha_(new double[model.get_n_col()]),
                                                                                                                                        s_(new double[model.get_n_row()]),
                                                                                                                                        n_alpha_var_(0),
                                                                                                                                        n_beta_var_(0),
                                                                                                                                        n_gamma_var_(0),
                                                                                                                                        base_alpha_var_(0),
                                                                                                                                        base_beta_var_(0),
                                                                                                                                        base_gamma_var_(0)

    {
        const vector<double> &routing_arc_resources{builder.get_routing_arc_times()};
        const size_t sz{routing_arc_resources.size()};

        for (size_t i{0}; i < sz; i++)
        {
            routing_arc_resources_[i] = routing_arc_resources[i];
        }

        const vector<vector<double>> &operations_resources{builder.get_operation_resources()};

        assert(operations_resources.size() == n_operations_);

        operations_resources_[0] = 0;

        for (size_t i{1}; i < n_operations_; i++)
        {
            operations_resources_[i] = operations_resources[i][1];
        }

        const vector<sync_operation> &operations{builder.get_operations()};

        for (size_t i{0}; i < n_operations_; i++)
        {
            const sync_operation &op{operations[i]};
            operation_2_customer_[i] = op.get_customer();
        }
    }

    ctsp_sync_checker::ctsp_sync_checker(void) : sync_checker_solver(),
                                                 tol_(1E-3),
                                                 n_operations_(0),
                                                 n_routing_arcs_(0),
                                                 n_sync_arcs_(0),
                                                 n_customers_(0),
                                                 n_col_(0),
                                                 routing_arc_resources_(nullptr),
                                                 operations_resources_(nullptr),
                                                 max_distance_(0),
                                                 col_inx_(nullptr),
                                                 row_inx_(nullptr),
                                                 coef_val_(nullptr),
                                                 sense_(nullptr),
                                                 alpha_(nullptr),
                                                 s_(nullptr)
    {
    }

    ctsp_sync_checker::~ctsp_sync_checker(void)
    {
        if (routing_arc_resources_ != nullptr)
        {
            delete[] routing_arc_resources_;
        }

        if (operations_resources_ != nullptr)
        {
            delete[] operations_resources_;
        }

        if (col_inx_ != nullptr)
        {
            delete[] col_inx_;
        }

        if (row_inx_ != nullptr)
        {
            delete[] row_inx_;
        }

        if (coef_val_ != nullptr)
        {
            delete[] coef_val_;
        }

        if (sense_ != nullptr)
        {
            delete[] sense_;
        }

        if (alpha_ != nullptr)
        {
            delete[] alpha_;
        }

        if (s_ != nullptr)
        {
            delete[] s_;
        }
    }

    void ctsp_sync_checker::set(const sync_model_a_builder &builder, const GOMA::model_description &model, const double tol)
    {
        sync_checker_solver::set(model, tol);

        n_operations_ = builder.get_n_operations();
        n_routing_arcs_ = builder.get_n_routing_arcs();
        n_sync_arcs_ = builder.get_n_sync_arcs();
        n_customers_ = builder.get_n_customers();
        n_col_ = model.get_n_col();

        routing_arcs_pair_map_ = builder.get_routing_arcs_pair_map();

        const vector<double> &routing_arc_resources{builder.get_routing_arc_times()};
        const size_t sz{routing_arc_resources.size()};

        if (routing_arc_resources_ != nullptr)
        {
            delete[] routing_arc_resources_;
        }

        routing_arc_resources_ = new double[sz];

        for (size_t i{0}; i < sz; i++)
        {
            routing_arc_resources_[i] = routing_arc_resources[i];
        }

        const vector<vector<double>> &operations_resources{builder.get_operation_resources()};

        if (operations_resources_ != nullptr)
        {
            delete[] operations_resources_;
        }

        operations_resources_ = new double[n_operations_];

        assert(operations_resources.size() == n_operations_);

        max_distance_ = builder.get_max_distance();

        for (size_t i{0}; i < n_operations_; i++)
        {
            operations_resources_[i] = operations_resources[i][1];
        }

        const vector<sync_operation> &operations{builder.get_operations()};

        if (operation_2_customer_.size() != n_operations_)
        {
            operation_2_customer_.resize(n_operations_);
        }

        for (size_t i{0}; i < n_operations_; i++)
        {
            const sync_operation &op{operations[i]};
            operation_2_customer_[i] = op.get_customer();
        }

        routing_outbound_arcs_ = builder.get_routing_outbound_arcs();
        routing_inbound_arcs_ = builder.get_routing_inbound_arcs();
        routing_arcs_ = builder.get_routing_arcs();

        if (col_inx_ != nullptr)
        {
            delete[] col_inx_;
        }

        if (row_inx_ != nullptr)
        {
            delete[] row_inx_;
        }

        if (coef_val_ != nullptr)
        {
            delete[] coef_val_;
        }

        if (sense_ != nullptr)
        {
            delete[] sense_;
        }

        if (alpha_ != nullptr)
        {
            delete[] alpha_;
        }

        if (s_ != nullptr)
        {
            delete[] s_;
        }

        col_inx_ = new int[get_nz()];
        row_inx_ = new int[get_nz()];
        coef_val_ = new double[get_nz()];
        sense_ = new char[get_nz()];
        alpha_ = new double[model.get_n_col()];
        s_ = new double[model.get_n_row()];

        compute_constraints_number_(builder);
    }

    bool ctsp_sync_checker::is_feasible(const vector<double> &x,
                                        vector<double> &alpha,
                                        vector<double> &beta,
                                        vector<double> &gamma)
    {
        const bool feasible{ctsp_sync_checker::is_feasible_(x)};

        if (!feasible)
        {
            get_alpha_beta_gamma_(alpha_, alpha, beta, gamma);
        }

        return feasible;
    }

    void ctsp_sync_checker::get_alpha_beta_gamma(vector<double> &alpha, vector<double> &beta, vector<double> &gamma) const
    {
        get_alpha_beta_gamma_(alpha_, alpha, beta, gamma);
    }

    void ctsp_sync_checker::get_s(vector<double> &s) const
    {
        get_s_(s_, s);
    }

    void ctsp_sync_checker::write(const char *filename) const
    {
        write_model(filename);
    }

    bool ctsp_sync_checker::is_feasible_(void)
    {
        bool feasible{false};

        solve();

        const int lp_stat = get_lp_stat();

        if (lp_stat == 1)
        {
            const double obj_val{get_obj()};

            // cout << "Obj: " << obj_val << endl;

            if (obj_val > -0.001)
            {
                feasible = true;
            }
        }
        else if (lp_stat == 2)
        {
            assert(false);
            cout << "Unbounded" << endl;
            exit(0);
        }
        else
        {
            assert(false);
            cout << "Error" << endl;
            exit(0);
        }

        return feasible;
    }

    bool ctsp_sync_checker::is_feasible_(double &obj_val)
    {
        bool feasible{false};

        obj_val = 0.0;

        solve();

        const int lp_stat = get_lp_stat();

        if (lp_stat == 1)
        {
            obj_val = get_obj();

            // cout << "Obj: " << obj_val << endl;

            if (obj_val > -0.001)
            {
                feasible = true;
            }
        }
        else if (lp_stat == 2)
        {
            assert(false);
            cout << "Unbounded" << endl;
            exit(0);
        }
        else
        {
            feasible = true;
            //write_model("error_model.lp");
            //assert(false);
            cout << "Error: " << lp_stat << endl;
            //exit(0);
        }

        return feasible;
    }

    bool ctsp_sync_checker::is_feasible_(const vector<double> &x)
    {

        x_2_obj_(x);
        x_2_coef_(x);

        const bool feasible{is_feasible_()};

        if (!feasible)
        {
            get_vars(alpha_);
        }
        else
        {
            get_dual_vars(s_);
        }

        return feasible;
    }

    bool ctsp_sync_checker::is_feasible_(const vector<double> &x, double &obj_val)
    {

        x_2_obj_(x);
        x_2_coef_(x);

        const bool feasible{is_feasible_(obj_val)};

        if (!feasible)
        {
            get_vars(alpha_);
        }
        else
        {
            get_dual_vars(s_);
        }

        return feasible;
    }

    void ctsp_sync_checker::x_2_alpha_coef_(const vector<double> &x, const size_t row_i, int &nz)
    {
        if (n_alpha_var_ == 0)
            return;

        const vector<int> &routing_outbound_arcs_o{routing_outbound_arcs_[row_i]};
        const size_t n_routing_outbound_arcs{routing_outbound_arcs_o.size()};

        for (size_t j{0}; j < n_routing_outbound_arcs; j++)
        {
            const int arc = routing_outbound_arcs_o[j];

            assert(arc < (int)n_routing_arcs_);

            row_inx_[nz] = row_i;
            col_inx_[nz] = base_alpha_var_ + arc;

            const double val{x[arc]};

            if (fabs(val) > tol_)
            {
                const double x_val{truncate_(val)};
                coef_val_[nz] = x_val;
            }
            else
            {
                coef_val_[nz] = 0.0;
            }

            nz++;
        }
    }

    void ctsp_sync_checker::x_2_beta_coef_(const vector<double> &x, const size_t row_i, int &nz)
    {
        if (n_beta_var_ == 0)
            return;

        const vector<int> &routing_in_arcs_o{routing_inbound_arcs_[row_i]};
        const size_t n_routing_in_arcs{routing_in_arcs_o.size()};

        for (size_t j{0}; j < n_routing_in_arcs; j++)
        {
            const int arc = routing_in_arcs_o[j];

            assert(arc < (int)n_routing_arcs_);

            row_inx_[nz] = row_i;
            col_inx_[nz] = base_beta_var_ + arc;

            const double val{x[arc]};

            if (fabs(val) > tol_)
            {
                const double x_val{truncate_(val)};
                coef_val_[nz] = x_val;
            }
            else
            {
                coef_val_[nz] = 0.0;
            }

            nz++;
        }
    }

    void ctsp_sync_checker::x_2_coef_(const vector<double> &x)
    {
        int nz{0};

        for (size_t i{0}; i < n_operations_; i++)
        {
            x_2_alpha_coef_(x, i, nz);
            x_2_beta_coef_(x, i, nz);
        }

        set_coef(nz, row_inx_, col_inx_, coef_val_);
    }

    void ctsp_sync_checker::x_2_obj_(const vector<double> &x)
    {
        int nz{0};

        x_2_alpha_obj_(x, nz);
        x_2_beta_obj_(x, nz);

        set_obj(coef_val_, col_inx_, nz);
    }

    void ctsp_sync_checker::x_2_alpha_obj_(const vector<double> &x, int &nz)
    {
        if (n_alpha_var_ == 0)
            return;

        for (size_t i{0}; i < n_routing_arcs_; i++)
        {
            const double val{x[i]};
            col_inx_[nz] = (int)(base_alpha_var_ + i);

            if (fabs(val) > tol_)
            {
                const double x_val{truncate_(val)};
                const double coef{truncate_(-routing_arc_resources_[i] * x_val)};
                coef_val_[nz] = coef;
            }
            else
            {
                coef_val_[nz] = 0.0;
            }

            nz++;
        }
    }

    void ctsp_sync_checker::x_2_beta_obj_(const vector<double> &x, int &nz)
    {
        if (n_beta_var_ == 0)
            return;

        const int depot_thrld{2 * static_cast<int>(n_depots_)};

        for (size_t i{0}; i < n_routing_arcs_; i++)
        {
            const triplet &arc{routing_arcs_[i]};
            const int t{arc.j_};

            col_inx_[nz] = (int)(base_beta_var_ + i);

            if (t >= depot_thrld)
            {

                const double val{x[i]};

                if (fabs(val) > tol_)
                {
                    const double x_val{truncate_(val)};
                    const double coef{truncate_(routing_arc_resources_[i] * x_val)};
                    coef_val_[nz] = coef;
                }
                else
                {
                    coef_val_[nz] = 0.0;
                }
            }
            else
            {
                coef_val_[nz] = 1E100;
            }

            nz++;
        }
    }

    void ctsp_sync_checker::get_s_(double *s, vector<double> &s_vec) const
    {
        s_vec.resize(n_operations_);

        for (size_t i{0}; i < n_operations_; i++)
        {
            s_vec[i] = s[i];
        }
    }      

    void ctsp_sync_checker::get_alpha_beta_gamma_(double *a, vector<double> &alpha, vector<double> &beta, vector<double> &gamma) const
    {
        if (n_alpha_var_ > 0)
        {
            if (alpha.size() != n_alpha_var_)
            {
                alpha.resize(n_alpha_var_);
            }

            for (size_t i{0}; i < n_alpha_var_; i++)
            {
                alpha[i] = a[base_alpha_var_ + i];
            }
        }

        if (n_beta_var_ > 0)
        {
            if (beta.size() != n_beta_var_)
            {
                beta.resize(n_beta_var_);
            }

            for (size_t i{0}; i < n_beta_var_; i++)
            {
                beta[i] = a[base_beta_var_ + i];
            }
        }

        if (gamma.size() != n_gamma_var_)
        {
            gamma.resize(n_gamma_var_);
        }

        for (size_t i{0}; i < n_gamma_var_; i++)
        {
            gamma[i] = a[base_gamma_var_ + i];
        }
    }
}