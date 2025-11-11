#include "CTSP_instance.hpp"
#include "TSPLIB_instance.hpp"

namespace CTSP
{
    instance::instance(void) : PTSP::instance(), T_(0), max_distance_(0), optimal_values_()
    {
    }

    instance::instance(const string &input_file) : PTSP::instance(), T_(0), max_distance_(0), optimal_values_()
    {
        read(input_file);
    }

    instance::~instance(void)
    {
    }

    void instance::read(const string &input_file)
    {
        TSP::TSPLIB_instance tsplib_instance;

        tsplib_instance.read(input_file);

        id_ = tsplib_instance.get_instance_name();
        type_ = tsplib_instance.get_instance_type();
        comment_ = tsplib_instance.get_instance_comment();

        n_customers_ = tsplib_instance.get_dimension() - 1;

        tsplib_instance.get_distances(distances_);
        // distances_.write_raw(cout);
        // cout << endl;

        demands_ = tsplib_instance.get_demands();

        max_distance_ = tsplib_instance.get_max_distance();

        n_days_ = tsplib_instance.get_num_days();

        T_.resize(n_customers_);

        for (size_t i{0}; i < n_customers_; i++)
        {
            T_[i] = tsplib_instance.get_maximum_allowable_differencial() ;
        }

        optimal_values_ = tsplib_instance.get_optimal_values();

        triangle_inequality_ = check_triangle_inequality_();

        if (!triangle_inequality_)
        {
            cerr << "Warning: Triangle inequality violated" << endl;
        }

        symmetry_ = check_symmetry_();

        if (!symmetry_)
        {
            cerr << "Warning: Distances are not symmetric" << endl;
        }
    }

    void instance::disable_max_distance(void)
    {
        max_distance_ = 999999999;
    }

    size_t instance::get_n_customer_operations(void) const
    {
        size_t n_customer_operations{0};

        const size_t n_locations{demands_.size()};

        for (size_t i{0}; i < n_locations; i++)
        {
            for (size_t j{0}; j < n_days_; j++)
            {
                if (demands_[i][j] > 0)
                {
                    n_customer_operations++;
                }
            }
        }
        return n_customer_operations;
    }

    ostream &instance::write_line(ostream &os) const
    {
        const size_t n_customer_operations{get_n_customer_operations()};

        os << setw(20) << left << id_ << '\t';
        os << setw(5) << right << T_[0] << '\t';
        os << setw(5) << max_distance_ << '\t';
        os << setw(5) << n_days_ << '\t';
        os << setw(5) << n_customers_ << '\t';
        os << setw(5) << n_customer_operations << '\t';
        os << setw(9) << fixed << setprecision(1) << optimal_values_[0] << '\t';
        os << setw(9) << fixed << setprecision(1) << optimal_values_[1] << '\t';
        return os;
    }

}