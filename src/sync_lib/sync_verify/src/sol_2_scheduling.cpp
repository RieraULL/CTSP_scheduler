/**
 * @file sol_2_scheduling.cpp
 * @brief Implementation of the CTSP solution to scheduling converter
 */

#include "sol_2_scheduling.hpp"

#include "ctsp_lb_primal_model.hpp"

#include <algorithm>
#include <iostream>
#include <cassert>

namespace SYNC_LIB
{
    conTSP2_scheduling::conTSP2_scheduling(const sync_model_a_builder &builder, double tol)
        : checker_(builder, tol),
          path_finder_(builder),
          n_depots_(builder.get_n_depots()),
          n_customers_(builder.get_n_customers()),
          n_operations_(builder.get_n_operations()),
          max_time_windows_size_(builder.get_time_windows_max_size()),
          max_distance_(builder.get_max_distance()),
          operation_2_depot_(builder.get_operation_2_depot()),
          operation_2_customer_(builder.get_operation_2_customer()),
          arc_time_matrix_(builder.get_arc_time_matrix()),
          operation_names_(builder.get_operation_names())
    {
    }

    conTSP2_scheduling::~conTSP2_scheduling(void)
    {
    }

    bool conTSP2_scheduling::solve(const string &instance_name, const vector<double> &x,
                                   sync_scheduling &scheduling, sync_infeasible &infeasible)
    {
        // Set the instance name in the output
        scheduling.instance_name_ = instance_name;
        infeasible.instance_name_ = instance_name;

        // Initialize start time variables
        vector<double> s(n_operations_, 0.0);

        // Verify synchronization constraints and compute start times
        // The checker solves an LP to find feasible start times if they exist
        const bool is_feasible = checker_.is_feasible(x, s, infeasible.alpha(), infeasible.beta(), infeasible.gamma());
        if (is_feasible)
        {
            // Normalize start times to begin from t=0
            refine_solution_(s);

            // Convert start times to depot schedules
            var_2_schedule_(s, scheduling);
        }
        else
        {
            vector<vector<int>> &cycles = infeasible.violated_cycles();

            path_finder_.find_paths(infeasible.alpha(), infeasible.beta(), infeasible.gamma(), cycles);
            cout << "Solution is infeasible in synchronization constraints." << endl;
        }

        return is_feasible;
    }

    void conTSP2_scheduling::refine_solution_(vector<double> &s)
    {
        // Find the minimum start time among all depot operations (first n_depots operations)
        double s_min{0.0};

        for (size_t k{0}; k < n_depots_; ++k)
        {
            if (s_min > s[k])
            {
                s_min = s[k];
            }
        }

        // Shift all start times so that the earliest depot operation starts at t=0
        // This normalization doesn't affect feasibility since all temporal differences are preserved
        for (size_t i{0}; i < n_operations_; ++i)
        {
            s[i] -= s_min;
        }
    }

    void conTSP2_scheduling::var_2_schedule_(const vector<double> &s, sync_scheduling &scheduling)
    {
        // Initialize scheduling structure with one list per depot
        scheduling.resize(n_depots_);

        for (size_t k{0}; k < n_depots_; ++k)
        {
            scheduling[k].clear();
        }

        // Assign each operation to its depot with initial timing (arrival=0, start=s[i])
        for (size_t i{0}; i < n_operations_; ++i)
        {
            const int depot_i{operation_2_depot_[i]};
            scheduling[depot_i].push_back(operation_info(i, operation_times(0, s[i])));
        }

        // For each depot, sort operations by start time and ensure depot pickup is last
        for (size_t k{0}; k < n_depots_; ++k)
        {
            vector<operation_info> &op_times_v{scheduling[k]};

            // Sort operations by start time (ascending)
            sort(op_times_v.begin(), op_times_v.end(),
                 [](const operation_info &a, const operation_info &b)
                 {
                     return a.second.second < b.second.second;
                 });

            // Find and move depot pickup to the last position
            // Depot pickups have IDs in range [n_depots, 2*n_depots)
            {
                const size_t n_ops_k{op_times_v.size()};

                size_t delivery_pos{0};

                for (size_t i{0}; i < n_ops_k; ++i)
                {
                    const int op_i{op_times_v[i].first};

                    if ((op_i >= (int)n_depots_) && (op_i < (int)(2 * n_depots_)))
                    {
                        delivery_pos = i;
                        break;
                    }
                }

                // Swap depot pickup with the last position
                if (delivery_pos < n_ops_k - 1)
                {
                    std::swap(op_times_v[delivery_pos], op_times_v[n_ops_k - 1]);
                }
            }
        }

        // Compute actual arrival times based on travel times between consecutive operations
        for (size_t k{0}; k < n_depots_; ++k)
        {
            vector<operation_info> &op_times_v{scheduling[k]};

            const size_t n_ops_k{op_times_v.size()};

            // First operation starts at time 0 (depot departure)
            op_times_v[0].second.second = 0.0;

            // For each subsequent operation, compute arrival time
            for (size_t i{1}; i < n_ops_k; ++i)
            {
                const int op_i_prev{op_times_v[i - 1].first};
                const int op_i_curr{op_times_v[i].first};

                // Get travel time between consecutive operations (using 1-based indexing)
                const double travel_time{arc_time_matrix_(op_i_prev + 1, op_i_curr + 1)};

                // Verify temporal feasibility: current start ≥ previous start + travel time
                assert(s[op_i_curr] >= s[op_i_prev] + travel_time - 1E-6);

                // Arrival time = previous start time + travel time
                op_times_v[i].second.first = op_times_v[i - 1].second.second + travel_time;
            }
        }

        // Convert operation IDs to customer IDs for output format
        for (size_t k{0}; k < n_depots_; ++k)
        {
            vector<operation_info> &op_times_v{scheduling[k]};
            const size_t n_ops_k{op_times_v.size()};

            // Convert customer visit operations to customer IDs (1-based)
            for (size_t i{1}; i < n_ops_k - 1; ++i)
            {
                op_times_v[i].first = operation_2_customer_[op_times_v[i].first] + 1;
            }

            // Mark first and last operations as depot operations (ID = 1)
            op_times_v[0].first = 1;           // Depot delivery (departure)
            op_times_v[n_ops_k - 1].first = 1; // Depot pickup (return)
        }
    }

    void conTSP2_scheduling::var_2_time_windows_(const vector<double> &s, sync_time_windows &time_windows)
    {
        // Allocate space for customer time windows (index 0 = depot, 1..n_customers = customers)
        time_windows.resize(n_customers_ + 1);

        // Track the earliest and latest start times for each customer across all vehicles
        vector<double> min_start_times(n_customers_ + 1, std::numeric_limits<double>::max());
        vector<double> max_start_times(n_customers_ + 1, std::numeric_limits<double>::lowest());

        // Collect min/max start times for each customer
        // Customer operations have IDs >= 2*n_depots (first n_depots = depot departures, next n_depots = depot returns)
        for (size_t i{2 * n_depots_}; i < n_operations_; ++i)
        {
            const int customer_i{operation_2_customer_[i]};

            min_start_times[customer_i] = std::min(min_start_times[customer_i], s[i]);
            max_start_times[customer_i] = std::max(max_start_times[customer_i], s[i]);
        }

        // Verify that the time span for each customer doesn't exceed the maximum allowed
        for (size_t i{1}; i <= n_customers_; ++i)
        {
            const double diff = max_start_times[i] - min_start_times[i];

            // All visits to customer i must occur within a time window of max_time_windows_size_
            assert(diff <= max_time_windows_size_ + 1E-6);
        }

        // Compute time windows centered around the average visit time
        for (size_t i{1}; i <= n_customers_; ++i)
        {
            // Center the time window around the midpoint of min and max visit times
            const double time_windows_center{0.5 * (min_start_times[i] + max_start_times[i])};

            // Compute time window bounds (ensuring non-negative start time)
            const double min_start_t = std::max(0.0, time_windows_center - 0.5 * max_time_windows_size_);
            const double max_start_t = min_start_t + max_time_windows_size_;

            // Verify that the computed window contains all actual visit times
            assert(min_start_t <= min_start_times[i] + 1E-6);
            assert(max_start_t >= max_start_times[i] - 1E-6);

            // Store the computed time window
            time_windows[i] = tw_info(min_start_t, max_start_t);
        }

        // Set depot time window to [0, max_distance] (vehicles can return anytime before max route duration)
        time_windows[0] = tw_info(0.0, max_distance_);
    }
}