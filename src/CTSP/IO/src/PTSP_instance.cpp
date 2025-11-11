#include "PTSP_instance.hpp"
#include <cmath>

namespace PTSP
{
    instance::instance(void) : id_(""), n_customers_(0), n_days_(0), distances_(), triangle_inequality_(false), symmetry_(false)
    {
    }

    instance::~instance(void)
    {
    }

    bool instance::check_triangle_inequality_(void) const
    {
        const size_t n_locations{distances_.get_n_rows()};

        for (size_t i{0}; i < n_locations; i++)
        {
            for (size_t j{0}; j < n_locations; j++)
            {
                for (size_t k{0}; k < n_locations; k++)
                {
                    if (i != j && j != k && i != k)
                    {
                        const double dist_ij{distances_(i + 1, j + 1)};
                        const double dist_ik{distances_(i + 1, k + 1)};
                        const double dist_kj{distances_(k + 1, j + 1)};

                        const double dist_ik_kj{ dist_ik + dist_kj };

                        if (dist_ik_kj < dist_ij - (1 + 1E-2))
                        {
                            return false;
                        }

                    }
                }
            }
        }
        return true;
    }

    bool instance::check_symmetry_(void) const
    {
        const size_t n_locations{distances_.get_n_rows()};

        for (size_t i{0}; i < n_locations; i++)
        {
            for (size_t j{0}; j < n_locations; j++)
            {
                if (i != j)
                {
                    const double dist_ij{distances_(i + 1, j + 1)};
                    const double dist_ji{distances_(j + 1, i + 1)};

                    if (fabs(dist_ij - dist_ji) > 1E-2)
                    {
                        return false;
                    }
                }
            }
        }
        return true;
    }
}