#pragma once

#include "sync_model_builder.hpp"
#include "sync_mapping.hpp"

/**
 * @file sync_model_a_builder.hpp
 * @brief Advanced model builder for arc-based CTSP formulations
 * 
 * This file defines the Model A builder, which extends the base model builder
 * to create a detailed arc-based mathematical programming formulation.
 * 
 * Model A is described in:
 * Jorge Riera-Ledesma, Inmaculada Rodríguez-Martín, Hipólito Hernández–Pérez,
 * "Dual-driven path elimination for vehicle routing with idle times and 
 * arrival-time consistency", Computers & Operations Research, 2025, 107326,
 * https://doi.org/10.1016/j.cor.2025.107326
 * 
 * Key features of Model A:
 * - Arc-based formulation with explicit routing and synchronization arcs
 * - Efficient mappings between operations, arcs, and decision variables
 * - Supports CPLEX-based optimization and custom constraint generation
 */

namespace SYNC_LIB
{
    /**
     * @class sync_model_a_builder
     * @brief Builds Model A arc-based formulation for CTSP problems
     * 
     * This class extends sync_model_builder to create a complete arc-based
     * mathematical programming model. It provides:
     * 
     * 1. Routing arcs: Vehicle-specific arcs for routing decisions
     * 2. Synchronization arcs: Cross-vehicle arcs for sync constraints  
     * 3. Efficient mappings: Bidirectional maps between arcs and variables
     * 4. Operations metadata: Customer-depot assignments, time windows, costs
     * 
     * The model is suitable for:
     * - MIP solvers (CPLEX, Gurobi, etc.)
     * - Branch-and-cut algorithms
     * - Constraint programming hybrid approaches
     */
    class sync_model_a_builder : public sync_model_builder
    {
    protected:
        const size_t n_operations_;         ///< Total number of operations
        const int problem_type_;            ///< 1=CTSP1, 2=CTSP2

        const size_t n_customers_;          ///< Number of customers
        const size_t n_vehicles_;           ///< Number of vehicles/depots
        const size_t n_depots_;             ///< Number of depot locations
        const double max_distance_;         ///< Maximum route distance/duration
        const double time_windows_max_size_;///< Maximum time window width

        // Routing arc structures
        pair_map routing_arcs_pair_map_;    ///< Maps (op_i, op_j) to routing arc index
        vector<triplet> routing_arcs_;      ///< List of all routing arcs
        vector<string> routing_arc_names_;  ///< Human-readable routing arc names
        vector<double> routing_arc_times_;  ///< Travel time for each routing arc

        vector<vector<int>> routing_outbound_arcs_;  ///< For each operation, outgoing routing arcs
        vector<vector<int>> routing_inbound_arcs_;   ///< For each operation, incoming routing arcs

        // Synchronization arc structures  
        pair_map sync_arcs_pair_map_;       ///< Maps (op_i, op_j) to sync arc index
        vector<triplet> sync_arcs_;         ///< List of all synchronization arcs
        vector<string> sync_arc_names_;     ///< Human-readable sync arc names
        vector<double> sync_arc_times_;     ///< Time offset for each sync arc

        // Global time matrix
        GOMA::matrix<double> arc_time_matrix_;  ///< Travel times between all operations

        // Operation metadata
        vector<string> operation_names_;        ///< Operation identifiers
        vector<vector<double>> operation_resources_;  ///< Resource consumption per operation
        vector<double> operation_costs_;        ///< Cost/objective coefficient per operation
        GOMA::matrix<int> operations_map_;      ///< Maps (customer, depot) to operation index
        vector<int> operation_2_customer_;      ///< Maps operation to customer ID
        vector<int> operation_2_depot_;         ///< Maps operation to depot ID

    public:
        /**
         * @brief Construct Model A builder
         * @param problem_type 1=CTSP1 (time window sync), 2=CTSP2 (exact sync)
         * @param instance_name Problem identifier
         * @param n_vehicles Number of vehicles
         * @param n_depots Number of depot locations
         * @param n_customers Number of customers
         * @param demands Customer demand matrix [customer][resource]
         * @param max_distance Maximum route distance/duration
         * @param w Time window widths for synchronization (per customer)
         * @param distances Distance/time matrix between locations
         * @param triangle_inequality Whether to enforce triangle inequality in preprocessing
         */
        sync_model_a_builder(const int problem_type, const string &instance_name, 
                            const size_t n_vehicles, const size_t n_depots, 
                            const size_t n_customers, const vector<vector<int>> &demands, 
                            const double max_distance, const vector<double> &w, 
                            const GOMA::matrix<double> &distances, 
                            const bool triangle_inequality);

        virtual ~sync_model_a_builder(void);

        // Accessors for problem parameters
        inline const vector<int> &get_operation_2_customer(void) const { return operation_2_customer_; }
        inline const vector<int> &get_operation_2_depot(void) const { return operation_2_depot_; }
        inline double get_time_windows_max_size(void) const { return time_windows_max_size_; }
        inline size_t get_n_vehicles(void) const { return n_vehicles_; }
        inline size_t get_n_depots(void) const { return n_depots_; }
        inline size_t get_n_customers(void) const { return n_customers_; }
        inline double get_max_distance(void) const { return max_distance_; }

        // Accessors for time/cost data
        inline const GOMA::matrix<double> &get_arc_time_matrix(void) const { return arc_time_matrix_; }

        // Accessors for operations metadata
        inline const vector<string> &get_operation_names(void) const { return operation_names_; }
        inline const vector<vector<double>> &get_operation_resources(void) const { return operation_resources_; }
        inline const vector<double> &get_operation_costs(void) const { return operation_costs_; }
        inline const GOMA::matrix<int> &get_operations_map(void) const { return operations_map_; }
        const vector<sync_operation> &get_operations(void) const { return sync_model_builder::operations_; }
        inline size_t get_n_operations(void) const { return sync_model_builder::operations_.size(); }

        // Accessors for routing arc data
        size_t get_n_routing_arcs(void) const { return routing_arcs_.size(); }
        inline const pair_map &get_routing_arcs_pair_map(void) const { return routing_arcs_pair_map_; }
        inline const vector<triplet> &get_routing_arcs(void) const { return routing_arcs_; }
        inline const vector<string> &get_routing_arc_names(void) const { return routing_arc_names_; }
        inline const vector<double> &get_routing_arc_times(void) const { return routing_arc_times_; }
        inline const vector<vector<int>> &get_routing_outbound_arcs(void) const { return routing_outbound_arcs_; }
        inline const vector<vector<int>> &get_routing_inbound_arcs(void) const { return routing_inbound_arcs_; }

        // Accessors for synchronization arc data
        size_t get_n_sync_arcs(void) const { return sync_arcs_.size(); }
        inline const pair_map &get_sync_arcs_pair_map(void) const { return sync_arcs_pair_map_; }
        inline const vector<triplet> &get_sync_arcs(void) const { return sync_arcs_; }
        inline const vector<string> &get_sync_arc_names(void) const { return sync_arc_names_; }
        inline const vector<double> &get_sync_arc_times(void) const { return sync_arc_times_; }

    private:
        void init_routing_arcs_map_(vector<triplet> &arcs);
        void init_sync_arcs_map_(vector<triplet> &arcs);

        void init_routing_operations_subset(vector<vector<int>> &vertices);

        void init_routing_subsets_names_(vector<string> &names);
        void init_sync_subsets_names_(vector<string> &names);

        void init_routing_subsets_resources_(vector<vector<double>> &resources);
        void init_sync_subsets_resources_(vector<vector<double>> &resources);

        void init_routing_arc_resources_(vector<double> &costs);
        void init_sync_arc_resources_(vector<double> &costs);

        void init_operation_names_(vector<string> &names);
        void init_operations_map_(GOMA::matrix<int> &operations_map);
        void init_operation_resources_(vector<vector<double>> &resources);
        void init_operation_costs_(vector<double> &costs) const;

        void init_routing_subset_maps_(vector<int> &ss_maps);
        void init_sync_subset_maps_(vector<int> &ss_maps);

        void get_routing_outbound_arcs_(vector<vector<int>> &arc_inxs) const;
        void get_routing_inbound_arcs_(vector<vector<int>> &arc_inxs) const;

        void set_routing_cost_matrix_(GOMA::matrix<double> &cost_matrix) const;

        void get_operation_2_customer_(vector<int> &operation_2_customer) const;
        void get_operation_2_depot_(vector<int> &operation_2_depot) const;

        void build_cluster_arcs_(vector<int> &clusters) const;
    };

}
