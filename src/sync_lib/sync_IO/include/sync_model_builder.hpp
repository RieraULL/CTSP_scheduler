#pragma once

#include "sync_operations.hpp"

#include "matrix.hpp"

#include <vector>
#include <map>
#include <utility>

/**
 * @file sync_model_builder.hpp
 * @brief Intermediate model builder for CTSP-like problems
 * 
 * This file defines the base model builder that constructs an intermediate
 * representation of CTSP problems using operations and partitions. This
 * representation bridges the gap between high-level problem instances and
 * low-level mathematical programming formulations.
 */

namespace SYNC_LIB
{
    /**
     * @class sync_model_builder
     * @brief Builds intermediate operation-based representation for CTSP problems
     * 
     * This class transforms a CTSP problem instance into an operation-based model:
     * 
     * 1. Operations: Creates tasks (customer, vehicle) pairs
     * 2. Routing Partition: Groups operations by vehicle for routing decisions
     * 3. Synchronization Partition: Groups operations by customer for sync constraints
     * 
     * The model supports two problem variants:
     * - CTSP1: Synchronization within time windows
     * - CTSP2: Exact synchronization (simultaneous arrivals)
     * 
     * This intermediate representation can be used to build various mathematical
     * programming formulations (e.g., Model A, Model B, etc.)
     */
    class sync_model_builder
    {
        typedef pair<int, int> operation_pair;

    protected:
        string instance_name_;              ///< Problem instance identifier

        int problem_type_;                  ///< 1 for CTSP1, 2 for CTSP2

        vector<sync_operation> operations_; ///< All operations (tasks) in the problem
        operations_partition routing_;      ///< Partition by vehicle (routing structure)
        operations_partition synchronization_; ///< Partition by customer (sync structure)

        const size_t n_vehicles_;           ///< Number of vehicles/depots
        const size_t n_customers_;          ///< Number of customers

    protected:
        map<operation_pair, int> operations_map_;     ///< Map (customer, vehicle) to operation index
        map<int, operation_pair> operations_map_inv_; ///< Map operation index to (customer, vehicle)

    public:
        /**
         * @brief Construct model builder for CTSP problem
         * @param problem_type 1 for CTSP1 (time window sync), 2 for CTSP2 (exact sync)
         * @param instance_name Problem instance identifier
         * @param n_vehicles Number of vehicles/depots
         * @param n_depots Number of depot locations
         * @param n_customers Number of customers
         * @param demands Demand matrix [customer][resource_type]
         * @param max_distance Maximum route length/duration
         * @param w Synchronization time window widths per customer
         * @param distances Distance/time matrix between locations
         */
        sync_model_builder(const int problem_type, const string &instance_name, 
                          const size_t n_vehicles, const size_t n_depots, 
                          const size_t n_customers, const vector<vector<int>> &demands, 
                          const double max_distance, const vector<double> &w, 
                          const GOMA::matrix<double> &distances);

        virtual ~sync_model_builder(void);

        // Accessors
        inline const string &get_instance_name(void) const { return instance_name_; }
        inline const vector<sync_operation> &get_operations(void) const { return operations_; }
        inline const operations_partition &get_routing(void) const { return routing_; }
        inline const operations_partition &get_synchronization(void) const { return synchronization_; }
        inline size_t get_n_operations(void) const { return operations_.size(); }

        /**
         * @brief Build complete intermediate representation
         * @param n_depots Number of depots
         * @param n_customers Number of customers  
         * @param demands Demand matrix
         * @param max_distance Maximum route distance
         * @param w Synchronization time windows
         * @param distances Distance matrix
         */
        void build_instance(const size_t n_depots, const size_t n_customers, 
                           const vector<vector<int>> &demands, const double max_distance, 
                           const vector<double> &w, const GOMA::matrix<double> &distances);

        // Build components of the model
        void build_operations(const size_t n_depots, const size_t n_customers, 
                             const vector<double> &w, const double max_distance, 
                             const vector<vector<int>> &demands);
        void build_routing_partition(const size_t n_depots, const double max_distance, 
                                     const GOMA::matrix<double> &distances);
        void build_synchronization_partition(const size_t n_depots, const size_t n_customers, 
                                             const vector<double> &w, const double max_distance);

        // Alternative build methods with explicit output
        void build_operations(const size_t n_depots, const size_t n_customers, 
                             const vector<double> &w, const double max_distance, 
                             const vector<vector<int>> &demands, 
                             vector<sync_operation> &operations);
        void build_routing_partition(const size_t n_depots, const double max_distance, 
                                     const GOMA::matrix<double> &distances, 
                                     const vector<sync_operation> &operations, 
                                     operations_partition &routing);
        void build_synchronization_partition(const size_t n_depots, const size_t n_customers, 
                                             const vector<double> &w, const double max_distance, 
                                             const vector<sync_operation> &operations, 
                                             operations_partition &synchronization);

        // Query methods
        size_t get_n_vehicles(void) const;
        size_t get_n_depots(void) const;
        double get_max_distance(void) const;

        void get_routing_operations(const size_t i, vector<int> &operations) const;
        void get_sync_operations(const size_t i, vector<int> &operations) const;

        void get_routing_arcs(const size_t i, vector<triplet> &arcs) const;
        void get_sync_arcs(const size_t i, vector<triplet> &arcs) const;

        void get_routing_operations_subset(vector<vector<int>> &vertices) const;

        void get_routing_arc_resources(const size_t i, vector<vector<double>> &resources) const;
        void get_sync_arc_resources(const size_t i, vector<vector<double>> &resources) const;

        void get_routing_arcs(vector<triplet> &arcs) const;
        void get_sync_arcs(vector<triplet> &arcs) const;

        void get_routing_arc_names(vector<string> &names) const;
        void get_sync_arc_names(vector<string> &names) const;

        void get_routing_arc_resources(vector<vector<double>> &resources) const;
        void get_sync_arc_resources(vector<vector<double>> &resources) const;

        void get_routing_subset_names(vector<string> &names) const;
        void get_sync_subset_names(vector<string> &names) const;

        void get_routing_subset_resources(vector<vector<double>> &resources) const;
        void get_sync_subset_resources(vector<vector<double>> &resources) const;

        void get_operation_names(vector<string> &names) const;
        void get_operation_resources(vector<vector<double>> &resources) const;

        void get_routing_subsets_maps(vector<int> &ss_maps) const;
        void get_sync_subsets_maps(vector<int> &ss_maps) const;

    private:
        // Internal methods for extracting information from partitions
        void get_arcs_(const operations_partition &P, vector<triplet> &arcs) const;
        void get_arc_labels_(const operations_partition &P, vector<string> &labels) const;
        void get_arc_resources_(const operations_partition &P, vector<vector<double>> &resources) const;

        void init_arcs_(const operation_arc_list &A, vector<triplet> &arcs) const;
        void init_arc_labels_(const vector<sync_operation> &operations, const operation_arc_list &A, vector<string> &labels) const;
        void init_arc_resources_(const operation_arc_list &A, vector<vector<double>> &resources) const;
    };
}