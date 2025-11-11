#include "sync_model_builder.hpp"

#include <algorithm>

namespace SYNC_LIB
{
    sync_model_builder::sync_model_builder(const int problem_type, const string &instance_name, const size_t n_vehicles, const size_t n_depots, const size_t n_customers, const vector<vector<int>> &demands, const double max_distance, const vector<double> &w, const GOMA::matrix<double> &distances) : problem_type_(problem_type), instance_name_(instance_name), routing_("Routing"), synchronization_("Synchronization"), n_vehicles_(n_vehicles), n_customers_(n_customers)
    {
        build_instance(n_depots, n_customers, demands, max_distance, w, distances);
    }

    sync_model_builder::~sync_model_builder(void) {}

    void sync_model_builder::build_instance(const size_t n_depots, const size_t n_customers, const vector<vector<int>> &demands, const double max_distance, const vector<double> &w, const GOMA::matrix<double> &distances)
    {
        build_operations(n_depots, n_customers, w, max_distance, demands);
        build_routing_partition(n_depots, max_distance, distances);
        build_synchronization_partition(n_depots, n_customers, w, max_distance);
    }

    void sync_model_builder::build_operations(const size_t n_depots, const size_t n_customers, const vector<double> &w, const double max_distance, const vector<vector<int>> &demands)
    {
        build_operations(n_depots, n_customers, w, max_distance, demands, operations_);
    }

    void sync_model_builder::build_routing_partition(const size_t n_depots, const double max_distance, const GOMA::matrix<double> &distances)
    {
        build_routing_partition(n_depots, max_distance, distances, operations_, routing_);
    }

    void sync_model_builder::build_synchronization_partition(const size_t n_depots, const size_t n_customers, const vector<double> &w, const double max_distance)
    {
        build_synchronization_partition(n_depots, n_customers, w, max_distance, operations_, synchronization_);
    }

    size_t sync_model_builder::get_n_vehicles(void) const
    {
        return 1;
    }

    size_t sync_model_builder::get_n_depots(void) const
    {
        return routing_.size() - 1;
    }

    void sync_model_builder::build_operations(const size_t n_depots, const size_t n_customers, const vector<double> &w, const double max_distance, const vector<vector<int>> &demands, vector<sync_operation> &operations)
    {
        // Demand table
        vector<vector<int>> Deliveries(n_customers);

        for (size_t l{0}; l < n_depots; ++l)
        {
            for (size_t j{0}; j < n_customers; ++j)
            {
                if (demands[j + 1][l] > 0)
                {
                    Deliveries[j].push_back(l);
                }
            }
        }

        for (size_t j{0}; j < n_customers; ++j)
        {
            sort(Deliveries[j].begin(), Deliveries[j].end());
        }

        // Operations

        int n_operations{0};

        for (size_t k{0}; k < n_depots; k++)
        {
            const operation_pair op_pair{1, k + 1};

            operations_map_[op_pair] = n_operations;
            operations_map_inv_[n_operations] = op_pair;

            {
                sync_operation o("Op_" + to_string(k + 1), {0, max_distance}, pair<int, int>(0, k + 1));
                operations.push_back(o);
            }

            n_operations++;
        }

        const size_t n_vertices{n_customers + 1};

        for (size_t k{0}; k < n_depots; k++)
        {
            const operation_pair op_pair{n_vertices + 1, k + 1};

            operations_map_[op_pair] = n_operations;
            operations_map_inv_[n_operations] = op_pair;

            {
                sync_operation o("Om_" + to_string(k + 1), {0, max_distance}, pair<int, int>(n_vertices, k + 1));
                operations.push_back(o);
            }

            n_operations++;
        }

        for (size_t i{0}; i < n_customers; i++)
        {
            const vector<int> &deliveries{Deliveries[i]};

            const size_t n_deliveries{deliveries.size()};

            for (size_t l{0}; l < n_deliveries; l++)
            {
                const operation_pair op_pair{i + 2, deliveries[l] + 1};

                operations_map_[op_pair] = n_operations;
                operations_map_inv_[n_operations] = op_pair;

                {
                    sync_operation o("c" + to_string(i + 1 + 1) + "_" + to_string(deliveries[l] + 1) + "", {1, w[i]}, pair<int, int>(i + 1, deliveries[l] + 1));
                    operations.push_back(o);
                }

                n_operations++;
            }
        }
    }

    void sync_model_builder::build_routing_partition(const size_t n_depots, const double max_distance, const GOMA::matrix<double> &distances, const vector<sync_operation> &operations, operations_partition &routing)
    {
        const size_t n_operations{operations.size()};
        const int n_vertices{distances.get_n_rows()};

        vector<operations_subset> routing_subset(n_depots);

        for (size_t l{0}; l < n_depots; l++)
        {
            operations_subset S("Day " + to_string(l + 1), {(double)n_operations, max_distance}, {});
            routing_subset[l] = S;
        }

        for (size_t j{0}; j < n_operations; ++j)
        {
            const auto &operation = operations_map_inv_[j];
            const int l = operation.second - 1;

            assert(l >= 0 && l < (int)n_depots);

            routing_subset[l].push_back(j);
        }

        // Intra set arcs

        for (int l{0}; l < n_depots; l++)
        {
            const int operation_depot_s{l};
            const int operation_depot_t{n_depots + l};

            operations_subset &routing_subset_l = routing_subset[l];
            const vector<int> &r_subset = routing_subset_l.get_operations_id();

            const size_t n_subset_operations{r_subset.size()};

            for (size_t i{0}; i < n_subset_operations; i++)
            {
                const int operation_i{r_subset[i]};

                if (operation_i != operation_depot_t)
                {

                    const auto &operation_pair_i{operations_map_inv_[operation_i]};
                    const int p_i{operation_pair_i.first - 1};

#ifndef NDEBUG
                    const int p_i_l{operation_pair_i.second - 1};
                    assert(p_i_l == (int)(l));
#endif

                    const double processing_time_i{0};

                    for (size_t j{0}; j < n_subset_operations; j++)
                        if (i != j)
                        {
                            const int operation_j{r_subset[j]};

                            // Evitamos que 0_s sea destino y que 0_t sea origen
                            if ((operation_j != operation_depot_s) && (!(operation_j == operation_depot_t && operation_i == operation_depot_s)))
                            {
                                const auto &operation_pair_j{operations_map_inv_[operation_j]};
                                const int p_j{operation_pair_j.first - 1};

#ifndef NDEBUG
                                const int p_j_l{operation_pair_j.second - 1};
                                assert(p_j_l == (int)(l));
#endif

                                double distance{distances(p_i + 1, (p_j % n_vertices) + 1)};
                                double time{distance + processing_time_i};

                                if (distance > 10000)
                                {
                                    cerr << "Distance between " << p_i + 1 << " and " << p_j + 1 << " is too large: " << distance << endl;

                                    distance = 0;
                                    time =  0;
                                }

                                const operation_arc arc{operation_arc(operation_pair(operation_i, operation_j), subset_pair(l, l), {distance, time})};

                                routing_subset_l.add_arc(arc);
                            }
                        }
                }
            }
        }

        routing.insert(routing.end(), routing_subset.begin(), routing_subset.end());
    }

    void sync_model_builder::build_synchronization_partition(const size_t n_depots, const size_t n_customers, const vector<double> &w, const double max_distance, const vector<sync_operation> &operations, operations_partition &synchronization)
    {
        vector<operations_subset> synchronization_subset(n_customers + 1);

        // Add customer synchronization subsets
        for (size_t i{0}; i < n_customers; i++)
        {
            operations_subset S("c_" + to_string(i + 1), {w}, {});
            synchronization_subset[i + 1] = S;
        }
        // Add depot synchronization subset
        {
            operations_subset S("Depots", {max_distance}, {});
            synchronization_subset[0] = S;
        }

        const size_t n_operations{operations.size()};
        const size_t n_vertices{n_customers + 1};

        for (size_t j{0}; j < n_operations; ++j)
        {
            const auto &operation = operations_map_inv_[j];
            const int i{(operation.first - 1) % n_vertices};

            synchronization_subset[i].push_back(j);
        }

        for (size_t c{0}; c < n_customers; c++)
        {
            operations_subset &synchronization_subset_l{synchronization_subset[c + 1]};
            const vector<int> &s_subset{synchronization_subset_l.get_operations_id()};
            const double w_c{w[c]};

            const size_t n_subset_operations{s_subset.size()};

            for (size_t i{0}; i < n_subset_operations; i++)
            {
                const int operation_i{s_subset[i]};

                for (size_t j{0}; j < n_subset_operations; j++)
                    if (i != j)
                    {
                        const int operation_j{s_subset[j]};

                        // const auto &operation_pair_j{operations_map_inv_[operation_j]};

                        // const int p_j{operation_pair_j.first - 1};
                        // const int l_j{operation_pair_j.second - 1};

                        const double processing_time_j{0};

                        const double resource_j = w_c - processing_time_j;

                        const operation_arc arc{operation_arc(operation_pair(operation_i, operation_j), subset_pair(c, c), {resource_j})};

                        synchronization_subset_l.add_arc(arc);
                    }
            }
        }

        {
            operations_subset &synchronization_subset_l{synchronization_subset[0]};
            const vector<int> &s_subset{synchronization_subset_l.get_operations_id()};

            const size_t n_departure_operations{n_depots};

            if (problem_type_ == 1)
            {

                // Arcos entre los depósitos de partida
                for (size_t i{0}; i < n_departure_operations; i++)
                {
                    const int operation_i{s_subset[i]};

                    for (size_t j{0}; j < n_departure_operations; j++)
                        if (i != j)
                        {
                            const int operation_j{s_subset[j]};

                            // const auto &operation_pair_j{operations_map_inv_[operation_j]};

                            // const int p_j{operation_pair_j.first - 1};
                            // const int l_j{operation_pair_j.second - 1};

                            const double resource_j = 0.00;

                            const operation_arc arc{operation_arc(operation_pair(operation_i, operation_j), subset_pair(0, 0), {resource_j})};

                            synchronization_subset_l.add_arc(arc);
                        }
                }
            }

            // Arcos entre los depósitos de llegada y cada uno de partida
            if (problem_type_ == 2)
            {
                for (size_t i{0}; i < n_departure_operations; i++)
                {
                    const int operation_i{s_subset[i + n_depots]};

                    // DEBUG
                    const auto &operation_pair_i{operations_map_inv_[operation_i]};
                    // const int p_i{operation_pair_i.first - 1};
                    const int l_i{operation_pair_i.second - 1};

                    for (size_t j{0}; j < n_departure_operations; j++)
                    {
                        const int operation_j{s_subset[j]};

                        // DEBUG
                        const auto &operation_pair_j{operations_map_inv_[operation_j]};
                        // const int p_j{operation_pair_j.first - 1};
                        const int l_j{operation_pair_j.second - 1};

                        const double resource_j = max_distance;

                        const operation_arc arc{operation_arc(operation_pair(operation_i, operation_j), subset_pair(l_i, l_j), {resource_j})};

                        synchronization_subset_l.add_arc(arc);
                    }
                }
            }
        }

        synchronization.insert(synchronization.end(), synchronization_subset.begin(), synchronization_subset.end());
    }

    void sync_model_builder::get_arcs_(const operations_partition &P, vector<triplet> &arcs) const
    {
        const int partition_subsets{(int)P.size()};

        init_arcs_(P.get_arcs(), arcs);

        for (int k{0}; k < partition_subsets; k++)
        {
            const operations_subset &subset_k{P.at(k)};

            init_arcs_(subset_k.get_arcs(), arcs);
        }
    }

    void sync_model_builder::init_arcs_(const operation_arc_list &A, vector<triplet> &arcs) const
    {
        for (const operation_arc &arc : A)
        {
            const int from{arc.first.first.first};
            const int to{arc.first.first.second};

            const int k_from{arc.first.second.first};
            const int k_to{arc.first.second.second};

            arcs.push_back(triplet(from, to, k_from, k_to));
        }
    }

    void sync_model_builder::init_arc_labels_(const vector<sync_operation> &operations, const operation_arc_list &A, vector<string> &labels) const
    {
        for (const operation_arc &arc : A)
        {
            const int from{arc.first.first.first};
            const int to{arc.first.first.second};

            labels.push_back("(" + operations[from].get_name() + "_" + operations[to].get_name() + ")");
        }
    }

    void sync_model_builder::init_arc_resources_(const operation_arc_list &A, vector<vector<double>> &resources) const
    {
        const int arc_sz{(int)A.size()};

        for (int i{0}; i < arc_sz; i++)
        {
            const resource_vector &r{A[i].second};
            resources.push_back(r);
        }
    }

    double sync_model_builder::get_max_distance(void) const
    {
        return synchronization_.back().get_resources().at(0);
    }

    void sync_model_builder::get_routing_operations(const size_t i, vector<int> &operations) const
    {
        const operations_subset &subset_i{routing_.at(i)};

        const size_t sz{subset_i.size()};

        for (size_t j{0}; j < sz; j++)
        {
            operations.push_back(subset_i.at(j));
        }
    }

    void sync_model_builder::get_routing_operations_subset(vector<vector<int>> &vertices) const
    {
        const size_t sz{routing_.size()};

        for (size_t i{0}; i < sz - 1; i++)
        {
            const operations_subset &subset_i{routing_.at(i)};

            const vector<int> &subset_i_v{subset_i.get_operations_id()};

            vertices.push_back(subset_i_v);
        }
    }

    void sync_model_builder::get_sync_operations(const size_t i, vector<int> &operations) const
    {
        const operations_subset &subset_i{synchronization_.at(i)};

        const size_t sz{subset_i.size()};

        for (size_t j{0}; j < sz; j++)
        {
            operations.push_back(subset_i.at(j));
        }
    }

    void sync_model_builder::get_routing_arcs(const size_t i, vector<triplet> &arcs) const
    {
        const operations_subset &subset_i{routing_.at(i)};

        init_arcs_(subset_i.get_arcs(), arcs);
    }

    void sync_model_builder::get_sync_arcs(const size_t i, vector<triplet> &arcs) const
    {
        const operations_subset &subset_i{synchronization_.at(i)};

        init_arcs_(subset_i.get_arcs(), arcs);
    }

    void sync_model_builder::get_routing_arc_resources(const size_t i, vector<vector<double>> &resources) const
    {
        const operations_subset &subset_i{routing_.at(i)};

        init_arc_resources_(subset_i.get_arcs(), resources);
    }

    void sync_model_builder::get_sync_arc_resources(const size_t i, vector<vector<double>> &resources) const
    {
        const operations_subset &subset_i{synchronization_.at(i)};

        init_arc_resources_(subset_i.get_arcs(), resources);
    }

    void sync_model_builder::get_routing_subsets_maps(vector<int> &ss_maps) const
    {
        ss_maps.resize(operations_.size(), -1);
        routing_.get_subsets_maps(ss_maps);
    }

    void sync_model_builder::get_sync_subsets_maps(vector<int> &ss_maps) const
    {
        ss_maps.resize(operations_.size(), -1);
        synchronization_.get_subsets_maps(ss_maps);
    }

    void sync_model_builder::get_routing_arcs(vector<triplet> &arcs) const
    {
        get_arcs_(routing_, arcs);
    }

    void sync_model_builder::get_sync_arcs(vector<triplet> &arcs) const
    {
        get_arcs_(synchronization_, arcs);
    }

    void sync_model_builder::get_routing_arc_names(vector<string> &names) const
    {
        get_arc_labels_(routing_, names);
    }

    void sync_model_builder::get_sync_arc_names(vector<string> &names) const
    {
        get_arc_labels_(synchronization_, names);
    }

    void sync_model_builder::get_routing_arc_resources(vector<vector<double>> &resources) const
    {
        get_arc_resources_(routing_, resources);
    }

    void sync_model_builder::get_sync_arc_resources(vector<vector<double>> &resources) const
    {
        get_arc_resources_(synchronization_, resources);
    }

    void sync_model_builder::get_arc_labels_(const operations_partition &P, vector<string> &labels) const
    {
        const int partition_subsets{(int)P.size()};

        init_arc_labels_(operations_, P.get_arcs(), labels);

        for (int k{0}; k < partition_subsets; k++)
        {
            const operations_subset &subset_k{P.at(k)};

            init_arc_labels_(operations_, subset_k.get_arcs(), labels);
        }
    }

    void sync_model_builder::get_arc_resources_(const operations_partition &P, vector<vector<double>> &resources) const
    {
        const int partition_subsets{(int)P.size()};

        init_arc_resources_(P.get_arcs(), resources);

        for (int k{0}; k < partition_subsets; k++)
        {
            const operations_subset &subset_k{P.at(k)};

            init_arc_resources_(subset_k.get_arcs(), resources);
        }
    }

    void sync_model_builder::get_routing_subset_names(vector<string> &names) const
    {
        const int sz{(int)routing_.size()};

        for (int i{0}; i < sz; i++)
        {
            names.push_back(routing_.at(i).get_name());
        }
    }

    void sync_model_builder::get_sync_subset_names(vector<string> &names) const
    {
        const int sz{(int)synchronization_.size()};

        for (int i{0}; i < sz; i++)
        {
            names.push_back(synchronization_.at(i).get_name());
        }
    }

    void sync_model_builder::get_routing_subset_resources(vector<vector<double>> &resources) const
    {
        const int sz{(int)routing_.size()};

        for (int i{0}; i < sz; i++)
        {
            const resource_vector &r{routing_.at(i).get_resources()};
            resources.push_back(r);
        }
    }

    void sync_model_builder::get_sync_subset_resources(vector<vector<double>> &resources) const
    {
        const int sz{(int)synchronization_.size()};

        for (int i{0}; i < sz; i++)
        {
            const resource_vector &r{synchronization_.at(i).get_resources()};
            resources.push_back(r);
        }
    }

    void sync_model_builder::get_operation_names(vector<string> &names) const
    {
        const int sz{(int)operations_.size()};

        for (int i{0}; i < sz; i++)
        {
            names.push_back(operations_[i].get_name());
        }
    }

    void sync_model_builder::get_operation_resources(vector<vector<double>> &resources) const
    {
        const int sz{(int)operations_.size()};

        for (int i{0}; i < sz; i++)
        {
            const resource_vector &r{operations_[i].get_resources()};
            resources.push_back(r);
        }
    }
}