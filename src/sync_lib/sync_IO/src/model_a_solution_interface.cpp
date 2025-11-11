#include "model_a_solution_interface.hpp"

namespace SYNC_LIB
{
    model_a_solution_interface::model_a_solution_interface(void) {}

    model_a_solution_interface::~model_a_solution_interface() {}

    void model_a_solution_interface::set(const sync_model_a_builder &model_builder)
    {
        instance_name_ = model_builder.get_instance_name();
        routing_arcs_pair_map_ = model_builder.get_routing_arcs_pair_map();
        routing_arcs_ = model_builder.get_routing_arcs();
        routing_arc_times_ = model_builder.get_routing_arc_times();
        operations_ = model_builder.get_operations();
        operations_map_ = model_builder.get_operations_map();
        n_depots_ = model_builder.get_n_depots();
        routing_arc_names_ = model_builder.get_routing_arc_names();
    }

    void model_a_solution_interface::sync_solution_2_model_a(const SYNC_LIB::sync_solution &sol, vector<double> &x) const
    {
        if (sol.empty())
        {
            x.clear();
            return;
        }

        x.resize(routing_arcs_.size(), 0.0);

        const vector<vector<int>> &routes{sol.get_routes()};

        for (size_t k{0}; k < n_depots_; ++k)
        {
            //cout << "Route for depot " << k + 1 << ":" << endl;

            const vector<int> &route{routes[k]};

            const size_t route_sz{route.size()};

            {
                const int customer_t{route[1] + 1};

                const int operation_s{(int)k};
                const int operation_t{operations_map_.at(customer_t, k + 1)};

                const int inx{routing_arcs_pair_map_.at(operation_s, operation_t)};

                assert(inx >= 0);
                assert(inx < (int)x.size());

                x[inx] = 1.0;
            }

            for (size_t j{1}; j < route_sz - 2; ++j)
            {
                const int customer_s{route[j] + 1};
                const int customer_t{route[j + 1] + 1};

                const int operation_s{operations_map_.at(customer_s, k + 1)};
                const int operation_t{operations_map_.at(customer_t, k + 1)};

                const int inx{routing_arcs_pair_map_.at(operation_s, operation_t)};

                assert(inx >= 0);
                assert(inx < (int)x.size());

                x[inx] = 1.0;
            }

            {
                const int customer_s{route[route_sz - 2] + 1};

                const int operation_s{operations_map_.at(customer_s, k + 1)};
                const int operation_t{int(n_depots_ + k)};

                const int inx{routing_arcs_pair_map_.at(operation_s, operation_t)};

                assert(inx >= 0);
                assert(inx < (int)x.size());

                x[inx] = 1.0;
            }

            //cout << endl;
        }
    }

    void model_a_solution_interface::model_a_2_sync_solution(const vector<double> &x, SYNC_LIB::sync_solution &sol) const
    {
        const size_t n_operations{operations_.size()};

        GOMA::matrix<int> routing_arcs_matrix(n_operations, n_operations, 0);

        for (size_t i = 0; i < x.size(); ++i)
        {
            if (x[i] > 0.5)
            {
                const SYNC_LIB::triplet &arc{routing_arcs_[i]};

                const int operation_s{arc.i_};
                const int operation_t{arc.j_};

                routing_arcs_matrix.at(operation_s + 1, operation_t + 1) = 1;
            }
        }

        sol.init();

        vector<vector<int>> routes(n_depots_);

        for (size_t k{0}; k < n_depots_; ++k)
        {
            for (size_t i{0}; i < n_operations; ++i)
            {
                if (routing_arcs_matrix.at(k + 1, i + 1) == 1)
                {
                    const SYNC_LIB::sync_operation &operation_i{operations_[i]};

                    const int customer_i{operation_i.get_customer()};
                    const int depot_i{operation_i.get_depot()};

                    assert(depot_i == (int)k);

                    vector<int> &route{routes[depot_i]};
                    route.push_back(1);
                    route.push_back(customer_i + 1);

                    // Get the next customer

                    bool end_of_route{false};

                    size_t prev{i};

                    while (!end_of_route)
                    {

                        SYNC_LIB::sync_operation operation_j;

                        bool found{false};

                        for (size_t j{1}; (j < n_operations) && !found; ++j)
                        {
                            if (routing_arcs_matrix.at(prev + 1, j + 1) == 1)
                            {
                                operation_j = operations_[j];
                                prev = j;
                                found = true;
                            }
                        }

                        if (prev == k + n_depots_)
                        {
                            end_of_route = true;
                            route.push_back(1);
                        }
                        else
                        {
                            const int customer_j{operation_j.get_customer()};
                            route.push_back(customer_j + 1);
                        }
                    }
                }
            }
        }

        sol.set(instance_name_, routes);
    }

    void model_a_solution_interface::operation_2_sync_solution_pair(const int operation, pair<int, int> &sync_pair) const
    {
        const SYNC_LIB::sync_operation &operation_i{operations_[operation]};

        const int customer_i{operation_i.get_customer()};
        const int depot_i{operation_i.get_depot()};

        sync_pair = pair<int, int>(customer_i + 1, depot_i + 1);
    }
}
