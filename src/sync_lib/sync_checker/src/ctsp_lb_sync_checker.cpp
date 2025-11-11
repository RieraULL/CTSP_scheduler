#include "ctsp_lb_sync_checker.hpp"
#include "ctsp_lb_primal_model.hpp"

#include <cmath>

namespace SYNC_LIB
{
    ctsp_lb_sync_checker::ctsp_lb_sync_checker(const sync_model_a_builder &builder, const double tol) : ctsp_sync_checker(builder, ctsp_lb_dual_primal_model(builder), tol)
    {
        compute_constraints_number_(builder);
    }

    ctsp_lb_sync_checker::ctsp_lb_sync_checker(void) : ctsp_sync_checker()
    {
    }

    ctsp_lb_sync_checker::~ctsp_lb_sync_checker(void)
    {
    }

    void ctsp_lb_sync_checker::compute_constraints_number_(const sync_model_a_builder &builder)
    {
        n_alpha_var_ = n_routing_arcs_;
        n_beta_var_ = 0;
        n_gamma_var_ = n_sync_arcs_;

        base_alpha_var_ = 0;
        base_beta_var_ = base_alpha_var_ + n_alpha_var_;
        base_gamma_var_ = base_beta_var_ + n_beta_var_;
    }
}