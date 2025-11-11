#pragma once

#include <vector>
#include <iostream>
#include <string>
#include <utility>

#include "sync_types.hpp"

using namespace std;

/**
 * @file sync_operations.hpp
 * @brief Operation and arc structures for CTSP-like problems
 * 
 * This file defines the fundamental building blocks for representing CTSP problems:
 * - Operations (tasks to be performed at customer locations)
 * - Arcs (transitions between operations)
 * - Operation subsets (groups of related operations)
 * - Operation partitions (complete decomposition of operations)
 * 
 * These structures support the intermediate model representation described in:
 * Jorge Riera-Ledesma, Inmaculada Rodríguez-Martín, Hipólito Hernández–Pérez,
 * "Dual-driven path elimination for vehicle routing with idle times and 
 * arrival-time consistency", Computers & Operations Research, 2025, 107326,
 * https://doi.org/10.1016/j.cor.2025.107326
 */

namespace SYNC_LIB
{
    /**
     * @class sync_operation
     * @brief Represents a single operation (task) in a CTSP problem
     * 
     * An operation corresponds to a specific vehicle visiting a specific customer.
     * In the literature, this is often called a "task". Each operation has:
     * - A unique name
     * - Resource consumption (e.g., time, capacity)
     * - Customer and depot/vehicle assignment
     * 
     * For n customers and m vehicles, there are typically n*m operations
     * (each vehicle can potentially visit each customer).
     */
    class sync_operation
    {
    private:
        string name_;                    ///< Unique operation identifier (e.g., "c5_v2")
        resource_vector r_;              ///< Resource consumption vector
        pair<int, int> custom_vehicle_;  ///< (customer_id, depot/vehicle_id)

    public:
        /// Default constructor
        sync_operation(void) : name_(), r_() {}
        
        /**
         * @brief Construct operation with full details
         * @param name Operation name
         * @param r Resource consumption vector
         * @param custom_vehicle (customer_id, depot_id) pair
         */
        sync_operation(const string &name, const resource_vector &r, const pair<int,int> &custom_vehicle) 
            : name_(name), r_(r), custom_vehicle_(custom_vehicle) {}
        
        virtual ~sync_operation(void) {}

        /// Get operation name
        inline const string &get_name(void) const
        {
            return name_;
        }

        /// Get resource consumption vector
        inline const resource_vector &get_resources(void) const
        {
            return r_;
        }

        /// Set operation name
        void set_name(const string &name)
        {
            name_ = name;
        }

        /// Set resource consumption
        void set_r(const resource_vector &r)
        {
            r_ = r;
        }

        /// Get (customer, depot) pair
        inline const pair<int, int> &get_custom_vehicle(void) const
        {
            return custom_vehicle_;
        }

        /// Get customer ID
        inline int get_customer(void) const
        {
            return custom_vehicle_.first;
        }

        /// Get depot ID (0-indexed)
        inline int get_depot(void) const
        {
            return custom_vehicle_.second - 1;
        }

        /**
         * @brief Write full operation information to stream
         * @param os Output stream
         * @return Reference to output stream
         */
        ostream &write(ostream &os) const;

        /**
         * @brief Write abbreviated operation name to stream
         * @param os Output stream
         * @return Reference to output stream
         */
        ostream &short_write(ostream &os) const;
    };

    /**
     * @class operation_arc
     * @brief Represents an arc (transition) between two operations
     * 
     * An arc defines a feasible transition from one operation to another,
     * including:
     * - Source and destination operations
     * - Source and destination subsets (for partitioned models)
     * - Resource consumption for the transition (e.g., travel time)
     */
    class operation_arc : public pair<arc_pair, resource_vector>
    {
    public:
        /**
         * @brief Construct an operation arc
         * @param p Operation pair (from_op, to_op)
         * @param s Subset pair (from_subset, to_subset)
         * @param r Resource consumption for this arc
         */
        operation_arc(const operation_pair &p, const subset_pair& s, const resource_vector &r) 
            : pair<arc_pair, resource_vector>(make_pair(arc_pair(p,s), r)) {}
        
        virtual ~operation_arc(void) {}

        /**
         * @brief Get human-readable name for this arc
         * @param operations List of all operations
         * @return Arc name in format "(from_op_to_op)"
         */
        const string get_name(const vector<sync_operation> &operations) const;
    };

    /**
     * @class operation_arc_list
     * @brief Collection of operation arcs
     */
    class operation_arc_list : public vector<operation_arc>
    {
    public:
        operation_arc_list(void) : vector<operation_arc>() {}
        virtual ~operation_arc_list(void) {}

        /**
         * @brief Get names of all arcs in the list
         * @param operations List of all operations
         * @param names Output: vector of arc names
         */
        void get_names(const vector<sync_operation> &operations, vector<string> &names) const;
    };

    /**
     * @class operations_subset
     * @brief A subset of operations with associated arcs
     * 
     * In the partitioned model, operations are grouped into subsets.
     * Each subset contains:
     * - A list of operation indices
     * - A name and resource vector
     * - Arcs connecting operations within the subset
     * 
     * For example, in routing partitions, each subset might represent
     * all operations (tasks) for a single vehicle.
     */
    class operations_subset : public vector<int>
    {
    private:
        string name_;               ///< Subset identifier
        resource_vector r_;         ///< Aggregate resource consumption
        operation_arc_list A_;      ///< Arcs within this subset

    public:
        operations_subset(void) : vector<int>(), name_(), r_() {}
        
        /**
         * @brief Construct subset with operations
         * @param name Subset name
         * @param r Resource vector
         * @param v List of operation indices in this subset
         */
        operations_subset(const string &name, const resource_vector &r, const vector<int> &v) 
            : vector<int>(v), name_(name), r_(r) {}
        
        virtual ~operations_subset(void) {}

        /// Get subset name
        inline const string &get_name(void) const
        {
            return name_;
        }

        /// Get resource vector
        inline const resource_vector &get_resources(void) const
        {
            return r_;
        }

        /// Get list of operation indices in this subset
        const vector<int> &get_operations_id(void) const
        {
            return *this;
        }

        /// Get number of operations in this subset
        int get_n_operations(void) const
        {
            return vector<int>::size();
        }

        /// Set subset name
        void set_name(const string &name)
        {
            name_ = name;
        }

        /// Set resource vector
        void set_r(const resource_vector &r)
        {
            r_ = r;
        }

        /**
         * @brief Add an arc to this subset
         * @param arc Arc to add
         */
        void add_arc(operation_arc arc)
        {
            A_.push_back(arc);
        }

        /// Get all arcs in this subset
        inline const operation_arc_list &get_arcs(void) const
        {
            return A_;
        }

        /**
         * @brief Write subset information to stream
         * @param operations List of all operations
         * @param os Output stream
         * @return Reference to output stream
         */
        ostream &write(const vector<sync_operation> &operations, ostream &os) const;
    };

    /**
     * @class operations_partition
     * @brief Complete partition of operations into disjoint subsets
     * 
     * A partition divides all operations into non-overlapping subsets.
     * Two key partitions in CTSP models:
     * 
     * 1. Routing partition: Groups operations by vehicle
     *    - Each subset contains all tasks for one vehicle
     *    - Arcs within subsets represent routing decisions
     *    - Arcs between subsets typically not allowed
     * 
     * 2. Synchronization partition: Groups operations by customer
     *    - Each subset contains all possible visits to one customer
     *    - Arcs represent different vehicles serving the same customer
     *    - Used to enforce synchronization constraints
     */
    class operations_partition : public vector<operations_subset>
    {
    private:
        string name_;               ///< Partition identifier (e.g., "Routing", "Synchronization")
        operation_arc_list A_;      ///< Arcs between different subsets

    public:
        /**
         * @brief Construct partition with name
         * @param name Partition identifier
         */
        operations_partition(const string &name) : vector<operations_subset>(), name_(name) {}
        
        virtual ~operations_partition(void) {}

        /// Get partition name
        inline const string &get_name(void) const
        {
            return name_;
        }

        /// Get all subsets in this partition
        const vector<operations_subset> &get_subsets(void) const
        {
            return *this;
        }

        /// Get number of subsets
        int get_n_subsets(void) const
        {
            return vector<operations_subset>::size();
        }

        /**
         * @brief Add arc between subsets
         * @param arc Inter-subset arc
         */
        void add_arc(operation_arc arc)
        {
            A_.push_back(arc);
        }

        /// Get all inter-subset arcs
        inline const operation_arc_list &get_arcs(void) const
        {
            return A_;
        }

        /**
         * @brief Create mapping from operations to their subset indices
         * @param ss_maps Output: for each operation, its subset index
         */
        void get_subsets_maps(vector<int> &ss_maps) const;

        /**
         * @brief Write partition information to stream
         * @param operations List of all operations
         * @param os Output stream
         * @return Reference to output stream
         */
        ostream &write(const vector<sync_operation> &operations, ostream &os) const;
    };
}