#include "sync_model_a_builder.hpp"

#include <set>
#include <cmath>
#include <algorithm>
#include <limits>

namespace SYNC_LIB
{
    sync_model_a_builder::sync_model_a_builder(const int problem_type,
                                               const string &instance_name,
                                               const size_t n_vehicles,
                                               const size_t n_depots,
                                               const size_t n_customers,
                                               const vector<vector<int>> &demands,
                                               const double max_distance,
                                               const vector<double> &w,
                                               const GOMA::matrix<double> &distances,
                                               const bool triangle_inequality) : sync_model_builder(problem_type, instance_name, n_vehicles, n_depots, n_customers, demands, max_distance, w, distances), n_operations_(get_n_operations()),
                                                                                 problem_type_(problem_type),
                                                                                 n_customers_(n_customers),
                                                                                 n_vehicles_(n_vehicles),
                                                                                 n_depots_(n_depots),
                                                                                 max_distance_(max_distance),
                                                                                 time_windows_max_size_(w[1]),
                                                                                 routing_arcs_pair_map_(n_operations_),
                                                                                 routing_arcs_(),
                                                                                 routing_arc_names_(),
                                                                                 routing_arc_times_(),
                                                                                 sync_arcs_pair_map_(n_operations_),
                                                                                 sync_arcs_(),
                                                                                 sync_arc_names_(),
                                                                                 sync_arc_times_(),
                                                                                 operation_names_(),
                                                                                 operation_resources_(),
                                                                                 operations_map_(n_customers_ + 1, n_depots_)
    {
        init_routing_arcs_map_(routing_arcs_);
        init_sync_arcs_map_(sync_arcs_);

        sync_model_builder::get_routing_arc_names(routing_arc_names_);
        sync_model_builder::get_sync_arc_names(sync_arc_names_);

        init_routing_arc_resources_(routing_arc_times_);
        init_sync_arc_resources_(sync_arc_times_);

        init_operation_names_(operation_names_);
        init_operations_map_(operations_map_);
        init_operation_resources_(operation_resources_);
        init_operation_costs_(operation_costs_);

        get_routing_outbound_arcs_(routing_outbound_arcs_);
        get_routing_inbound_arcs_(routing_inbound_arcs_);

        set_routing_cost_matrix_(arc_time_matrix_);

        get_operation_2_customer_(operation_2_customer_);
        get_operation_2_depot_(operation_2_depot_);
    }

    sync_model_a_builder::~sync_model_a_builder(void) {}

    void sync_model_a_builder::init_routing_subset_maps_(vector<int> &ss_maps)
    {
        get_routing_subsets_maps(ss_maps);
    }

    void sync_model_a_builder::init_sync_subset_maps_(vector<int> &ss_maps)
    {
        get_sync_subsets_maps(ss_maps);
    }

    void sync_model_a_builder::init_routing_arcs_map_(vector<triplet> &arcs)
    {
        sync_model_builder::get_routing_arcs(arcs);
        routing_arcs_pair_map_.set(routing_arcs_);
    }

    void sync_model_a_builder::init_sync_arcs_map_(vector<triplet> &arcs)
    {
        sync_model_builder::get_sync_arcs(arcs);
        sync_arcs_pair_map_.set(sync_arcs_);
    }

    void sync_model_a_builder::init_routing_arc_resources_(vector<double> &costs)
    {
        vector<vector<double>> resources;

        sync_model_builder::get_routing_arc_resources(resources);

        const int n_arcs{(int)routing_arcs_.size()};
        costs.resize(n_arcs);

        for (int i{0}; i < n_arcs; i++)
        {
            costs[i] = resources[i][1];
        }
    }

    void sync_model_a_builder::init_sync_arc_resources_(vector<double> &costs)
    {
        vector<vector<double>> resources;

        sync_model_builder::get_sync_arc_resources(resources);

        const int n_arcs{(int)sync_arcs_.size()};
        costs.resize(n_arcs);

        for (int i{0}; i < n_arcs; i++)
        {
            costs[i] = resources[i][0];
        }
    }

    void sync_model_a_builder::init_routing_operations_subset(vector<vector<int>> &vertices)
    {
        sync_model_builder::get_routing_operations_subset(vertices);
    }

    void sync_model_a_builder::init_routing_subsets_names_(vector<string> &names)
    {
        get_routing_subset_names(names);
    }

    void sync_model_a_builder::init_sync_subsets_names_(vector<string> &names)
    {
        get_sync_subset_names(names);
    }

    void sync_model_a_builder::init_routing_subsets_resources_(vector<vector<double>> &resources)
    {
        get_routing_subset_resources(resources);
    }

    void sync_model_a_builder::init_sync_subsets_resources_(vector<vector<double>> &resources)
    {
        get_sync_subset_resources(resources);
    }

    void sync_model_a_builder::init_operation_names_(vector<string> &names)
    {
        sync_model_builder::get_operation_names(names);
    }

    void sync_model_a_builder::init_operation_resources_(vector<vector<double>> &resources)
    {
        sync_model_builder::get_operation_resources(resources);
    }

    void sync_model_a_builder::init_operation_costs_(vector<double> &costs) const
    {
        const size_t n_operations{n_operations_};

        costs.resize(n_operations);

        for (size_t j{0}; j < n_operations; j++)
        {
            costs[j] = operation_resources_[j][1];
        }
    }

    void sync_model_a_builder::init_operations_map_(GOMA::matrix<int> &operations_map)
    {
        const size_t n_operations{n_operations_};

        operations_map.init(-1);

        const size_t n_depot_operations{n_depots_ * 2};

        for (size_t j{n_depot_operations}; j < n_operations; j++)
        {
            const sync_operation &operation{operations_[j]};
            const int c_i{operation.get_customer()};
            const int d_i{operation.get_depot()};

            operations_map.at(c_i + 1, d_i + 1) = j;
        }
    }

    void sync_model_a_builder::get_routing_inbound_arcs_(vector<vector<int>> &arc_inxs) const
    {
        const size_t n_operations{n_operations_};

        arc_inxs.resize(n_operations);

        const size_t routing_arcs_sz{routing_arcs_.size()};

        for (size_t i{0}; i < routing_arcs_sz; i++)
        {
            const triplet &arc{routing_arcs_[i]};

            const int jj{arc.j_};

            arc_inxs[jj].push_back(i);
        }
    }

    void sync_model_a_builder::get_routing_outbound_arcs_(vector<vector<int>> &arc_inxs) const
    {
        const size_t n_operations{n_operations_};

        arc_inxs.resize(n_operations);

        const size_t routing_arcs_sz{routing_arcs_.size()};

        for (size_t i{0}; i < routing_arcs_sz; i++)
        {
            const triplet &arc{routing_arcs_[i]};

            const int ii{arc.i_};

            arc_inxs[ii].push_back(i);
        }
    }

    void sync_model_a_builder::get_operation_2_customer_(vector<int> &operation_2_customer) const
    {
        operation_2_customer.resize(n_operations_);

        for (size_t j{0}; j < n_operations_; j++)
        {
            operation_2_customer[j] = operations_[j].get_customer();
        }
    }

    void sync_model_a_builder::get_operation_2_depot_(vector<int> &operation_2_depot) const
    {
        operation_2_depot.resize(n_operations_);

        for (size_t j{0}; j < n_operations_; j++)
        {
            operation_2_depot[j] = operations_[j].get_depot();
        }
    }

    void sync_model_a_builder::set_routing_cost_matrix_(GOMA::matrix<double> &cost_mat) const
    {
        const size_t n_operations{n_operations_};

        cost_mat.resize(n_operations, n_operations);
        cost_mat.init(1E9);

        const vector<triplet> &arcs{get_routing_arcs()};

        int arc_inx{0};

        for (const triplet &arc : arcs)
        {
            const int i{arc.i_};
            const int j{arc.j_};

            const double cost{routing_arc_times_[arc_inx]};

            cost_mat(i + 1, j + 1) = cost;

            arc_inx++;
        }
    }

    void sync_model_a_builder::build_cluster_arcs_(vector<int> &clusters) const
    {
        clusters.clear();

        const size_t n_arcs{routing_arc_times_.size()};

        for (size_t i{0}; i < n_arcs; i++)
        {
            const double c_ij{routing_arc_times_[i]};

            if (fabs(c_ij) < 1E-2)
            {
                clusters.push_back(i);
            }
        }
    }
}