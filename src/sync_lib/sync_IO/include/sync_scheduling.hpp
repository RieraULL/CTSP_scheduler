#pragma once

#include <vector>
#include <iostream>
#include <string>
#include <utility>

using namespace std;

/**
 * @file sync_scheduling.hpp
 * @brief Scheduling of operations with timing information for CTSP-like problems
 * 
 * This file defines structures for representing complete schedules that include
 * not only routing decisions but also precise timing information for each operation.
 */

namespace SYNC_LIB
{
    /// Time information for an operation: (arrival_time, service_start_time)
    typedef pair<double, double> operation_times;
    
    /// Complete operation information: (customer_id, timing_info)
    typedef pair<int, operation_times> operation_info;

    /**
     * @class sync_scheduling
     * @brief Represents a complete schedule with timing for CTSP-like problems
     * 
     * A scheduling solution extends a routing solution by including precise
     * timing information for each customer visit. Each route contains a sequence
     * of operation_info elements that specify when each customer is visited
     * and serviced.
     * 
     * Structure: vector of routes, where each route is a vector of:
     * - customer ID
     * - (arrival time, service start time)
     */
    class sync_scheduling: public vector<vector<operation_info>>
    {
        public:
            string instance_name_;  ///< Name of the problem instance

        public:
            /// Default constructor
            sync_scheduling(void);
            
            /**
             * @brief Construct scheduling with instance name
             * @param instance_name Name of the problem instance
             */
            sync_scheduling(const string &instance_name);
            
            virtual ~sync_scheduling(void);

            /**
             * @brief Write scheduling to output stream in JSON format
             * @param os Output stream
             * @return Reference to output stream
             */
            ostream &write_json(ostream &os) const;
            
            /**
             * @brief Read scheduling from input stream in JSON format
             * @param is Input stream
             * @return Reference to input stream
             */
            istream &read_json(istream &is);
    };
}