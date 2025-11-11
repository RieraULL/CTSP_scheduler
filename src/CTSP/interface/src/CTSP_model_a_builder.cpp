#include "CTSP_model_a_builder.hpp"

namespace CTSP
{
    CTSP_model_a_builder::CTSP_model_a_builder(const CTSP::CTSP_problem_type &problem_type, const CTSP::instance &instance) : SYNC_LIB::sync_model_a_builder(problem_type == CTSP::CTSP_problem_type::CTSP1?1:2,instance.get_instance_name(), 1, instance.get_n_days(), instance.get_n_customers(), instance.get_demands(), instance.get_max_distance(), instance.get_T(), instance.get_distances(), instance.triangle_inequality())
    {
    }

    CTSP_model_a_builder::~CTSP_model_a_builder(void) {}
}