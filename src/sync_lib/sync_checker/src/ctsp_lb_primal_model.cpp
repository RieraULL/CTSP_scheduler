#include "ctsp_lb_primal_model.hpp"

#define INF_MD_THRLD 1E6

namespace SYNC_LIB
{

    ctsp_lb_primal_model::ctsp_lb_primal_model(const sync_model_a_builder &builder) : ctsp_primal_model(builder)
    {
        init_model_(builder);

        GOMA::model_description::name_ = "SYNC_LIB_ctsp_lb_primal_model";
    }

    ctsp_lb_primal_model::~ctsp_lb_primal_model(void)
    {
    }

    void ctsp_lb_primal_model::compute_constraints_number_(const sync_model_a_builder &builder)
    {
        n_alpha_constraints_ = builder.get_n_routing_arcs();
        n_beta_constraints_ = 0;
        n_gamma_constraints_ = builder.get_n_sync_arcs();
    }
   
    ctsp_lb_dual_primal_model::ctsp_lb_dual_primal_model(const sync_model_a_builder &builder) : GOMA::model_description()
    {
        ctsp_lb_primal_model primal(builder);

        GOMA::model_description::set_dual_(primal);
        
        GOMA::model_description::name_ = "SYNC_LIB_ctsp_lb_dual_primal_model";

    }
}