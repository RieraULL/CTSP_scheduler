/**
 * @file LP_solver.cpp
 * @brief Implementation of LP_solver base class
 */

#include "LP_solver.hpp"

namespace GOMA
{
    LP_solver::LP_solver(const model_description &model, const double tol) 
        : tol_(tol),
          n_col_(model.get_n_col()),
          n_row_(model.get_n_row()),
          prob_type_(model.get_prob_type()),
          lpstat_(0)
    {
        // Base class constructor - initializes common solver state
    }

    LP_solver::~LP_solver(void)
    {
        // Base class destructor - concrete classes handle cleanup
    }
}