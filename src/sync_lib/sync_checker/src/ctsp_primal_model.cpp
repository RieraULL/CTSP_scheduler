#include "ctsp_primal_model.hpp"

#define INF_MD_THRLD 1E6

namespace SYNC_LIB
{

    ctsp_primal_model::ctsp_primal_model(const sync_model_a_builder &builder) : GOMA::model_description(),
                                                                                n_alpha_constraints_(0),
                                                                                n_beta_constraints_(0),
                                                                                n_gamma_constraints_(0),
                                                                                base_alpha_constraints_(0),
                                                                                base_beta_constraints_(0),
                                                                                base_gamma_constraints_(0)
    {
    }

    void ctsp_primal_model::init_model_(const sync_model_a_builder &builder)
    {
        compute_constraints_number_(builder);

        base_alpha_constraints_ = 0;
        base_beta_constraints_ = base_alpha_constraints_ + n_alpha_constraints_;
        base_gamma_constraints_ = base_beta_constraints_ + n_beta_constraints_;

        compute_n_col_(builder);
        compute_n_row_(builder);

        build_model_(builder);
        build_primal_matrix_(builder, GOMA::model_description::M_, GOMA::model_description::nz_);

        set_var_labels_(builder);
        set_cons_labels_(builder);
    }

    ctsp_primal_model::~ctsp_primal_model(void)
    {
    }

    void ctsp_primal_model::compute_n_col_(const sync_model_a_builder &builder)
    {
        n_col_ = builder.get_n_operations();
    }

    void ctsp_primal_model::compute_n_row_(const sync_model_a_builder &builder)
    {
        n_row_ = n_alpha_constraints_ + n_beta_constraints_ + n_gamma_constraints_;
    }

    void ctsp_primal_model::build_variables_(const sync_model_a_builder &builder)
    {
        GOMA::model_description::obj_.resize(n_col_);
        GOMA::model_description::bd_.resize(n_col_);
        GOMA::model_description::ctype_.resize(n_col_);
        GOMA::model_description::bounds_.resize(n_col_);

        const double u_bound{1E10};

        const size_t n_operations{builder.get_n_operations()};

        for (size_t i{0}; i < n_operations; i++)
        {
            GOMA::model_description::obj_[i] = 0;
            GOMA::model_description::bd_[i] = GOMA::VarBnd::UBounded;
            GOMA::model_description::ctype_[i] = GOMA::VarType::C;
            GOMA::model_description::bounds_[i] = make_pair(0, u_bound);
        }
    }

    void ctsp_primal_model::build_constraints_(const sync_model_a_builder &builder)
    {
        GOMA::model_description::sense_.resize(n_row_);
        GOMA::model_description::rhs_.resize(n_row_);

        build_alpha_constraints_(builder);
        build_beta_constraints_(builder);
        build_gamma_constraints_(builder);
    }

    void ctsp_primal_model::build_alpha_constraints_(const sync_model_a_builder &builder)
    {
        if (n_alpha_constraints_ == 0)
            return;

        const size_t n_routing_arcs{builder.get_n_routing_arcs()};
        const vector<double> &routing_arc_times{builder.get_routing_arc_times()};

        for (size_t i{0}; i < n_routing_arcs; i++)
        {
            const double t_ij = routing_arc_times[i];

            GOMA::model_description::sense_[base_alpha_constraints_ + i] = 'L';
            GOMA::model_description::rhs_[base_alpha_constraints_ + i] = -t_ij;
        }
    }

    void ctsp_primal_model::build_beta_constraints_(const sync_model_a_builder &builder)
    {
        if (n_beta_constraints_ == 0)
            return;

        const size_t n_routing_arcs{builder.get_n_routing_arcs()};
        const vector<double> &routing_arc_times{builder.get_routing_arc_times()};

        for (size_t i{0}; i < n_routing_arcs; i++)
        {
            const double t_ij = routing_arc_times[i];

            GOMA::model_description::sense_[base_beta_constraints_ + i] = 'L';
            GOMA::model_description::rhs_[base_beta_constraints_ + i] = t_ij;
        }
    }

    void ctsp_primal_model::build_gamma_constraints_(const sync_model_a_builder &builder)
    {
        const size_t n_sync_arcs{builder.get_n_sync_arcs()};

        const vector<double> &sync_arc_times{builder.get_sync_arc_times()};

        for (size_t i{0}; i < n_sync_arcs; i++)
        {
            const double w_h_p_j = sync_arc_times[i];

            GOMA::model_description::sense_[base_gamma_constraints_ + i] = 'L';

            if (w_h_p_j < INF_MD_THRLD)
            {
                GOMA::model_description::rhs_[base_gamma_constraints_ + i] = w_h_p_j + 0.00;
            }
            else
            {
                GOMA::model_description::rhs_[base_gamma_constraints_ + i] = 0.0 + 0.00;
            }
        }
    }

    void ctsp_primal_model::build_objective_sense_(const sync_model_a_builder &builder)
    {
        obj_sense_ = GOMA::ObjSen::Maximize;
    }

    void ctsp_primal_model::build_model_(const sync_model_a_builder &builder)
    {
        build_variables_(builder);
        build_constraints_(builder);
        build_objective_sense_(builder);
    }

    void ctsp_primal_model::build_primal_matrix_(const sync_model_a_builder &builder, GOMA::matrix<double> &M, int &nz)
    {
        M.resize(n_row_, n_col_);
        M.init(0.0);

        nz = 0;

        build_alpha_primal_matrix_(builder, M, nz);
        build_beta_primal_matrix_(builder, M, nz);
        build_gamma_primal_matrix_(builder, M, nz);
    }

    void ctsp_primal_model::build_alpha_primal_matrix_(const sync_model_a_builder &builder, GOMA::matrix<double> &M, int &nz)
    {
        if (n_alpha_constraints_ == 0)
            return;

        const vector<triplet> &routing_arcs{builder.get_routing_arcs()};
        const size_t n_routing_arcs{routing_arcs.size()};

        for (size_t i{0}; i < n_routing_arcs; i++)
        {
            const triplet &arc{routing_arcs[i]};
            const int o_i = arc.i_;
            const int o_j = arc.j_;

            M(base_alpha_constraints_ + i + 1, o_i + 1) = 1;
            nz++;

            M(base_alpha_constraints_ + i + 1, o_j + 1) = -1;
            nz++;
        }
    }

    void ctsp_primal_model::build_beta_primal_matrix_(const sync_model_a_builder &builder, GOMA::matrix<double> &M, int &nz)
    {
        if (n_beta_constraints_ == 0)
            return;

        const vector<triplet> &routing_arcs{builder.get_routing_arcs()};
        const size_t n_routing_arcs{routing_arcs.size()};

        for (size_t i{0}; i < n_routing_arcs; i++)
        {
            const triplet &arc{routing_arcs[i]};
            const int o_i = arc.i_;
            const int o_j = arc.j_;

            M(base_beta_constraints_ + i + 1, o_i + 1) = -1;
            nz++;

            M(base_beta_constraints_ + i + 1, o_j + 1) = 1;
            nz++;
        }
    }

    void ctsp_primal_model::build_gamma_primal_matrix_(const sync_model_a_builder &builder, GOMA::matrix<double> &M, int &nz)
    {
        const vector<triplet> &sync_arcs{builder.get_sync_arcs()};
        const size_t n_sync_arcs{sync_arcs.size()};

        for (size_t i{0}; i < n_sync_arcs; i++)
        {
            const triplet &arc{sync_arcs[i]};
            const int o_i = arc.i_;
            const int o_j = arc.j_;

            M(base_gamma_constraints_ + i + 1, o_j + 1) = -1;
            nz++;

            M(base_gamma_constraints_ + i + 1, o_i + 1) = 1;
            nz++;
        }
    }

    void ctsp_primal_model::set_var_labels_(const sync_model_a_builder &builder)
    {
        const vector<string> &operation_names{builder.get_operation_names()};

        for (const string &s : operation_names)
        {
            GOMA::model_description::var_labels_.push_back("Operation_" + s);
        }
    }

    void ctsp_primal_model::set_cons_labels_(const sync_model_a_builder &builder)
    {
        const vector<string> &routing_arc_names{builder.get_routing_arc_names()};

        if (n_alpha_constraints_ > 0)
        {
            for (const string &s : routing_arc_names)
            {
                GOMA::model_description::cons_labels_.push_back("alpha" + s);
            }
        }

        if (n_beta_constraints_ > 0)
        {
            for (const string &s : routing_arc_names)
            {
                GOMA::model_description::cons_labels_.push_back("beta" + s);
            }
        }

        const vector<string> &sync_arc_names{builder.get_sync_arc_names()};

        for (const string &s : sync_arc_names)
        {
            GOMA::model_description::cons_labels_.push_back("gamma" + s);
        }
    }

}