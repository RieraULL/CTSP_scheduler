#pragma once

#include <vector>
#include <iostream>
#include <string>
#include <utility>

using namespace std;

/**
 * @file sync_solution.hpp
 * @brief Solution representation for CTSP-like problems
 * 
 * This file defines the sync_solution class which encapsulates a complete
 * solution for vehicle routing problems with synchronization constraints.
 * A solution consists of a set of routes, one per vehicle, visiting customers.
 */

namespace SYNC_LIB
{
    /**
     * @class sync_solution
     * @brief Represents a complete routing solution for CTSP-like problems
     * 
     * A solution is defined by:
     * - An instance name identifying the problem
     * - A collection of routes, where each route is a sequence of customer visits
     * 
     * The class provides methods for reading/writing solutions in both
     * standard and JSON formats, as well as extracting arc representations.
     */
    class sync_solution 
    {
    private:
        string instance_name_;          ///< Name of the problem instance
        vector<vector<int>> routes_;    ///< Routes for each vehicle (depot)
    public:
        /**
         * @brief Construct a solution with instance name and routes
         * @param instance_name Name of the problem instance
         * @param routes Vector of routes, one per vehicle
         */
        sync_solution(const string &instance_name, const vector<vector<int>> &routes);
        
        /**
         * @brief Construct a solution by loading from file
         * @param instance_name Path to solution file
         */
        sync_solution(const string &instance_name);
        
        /**
         * @brief Default constructor
         */
        sync_solution(void);
        
        virtual ~sync_solution(void);

        /**
         * @brief Initialize/clear the solution
         */
        void init(void);

        /**
         * @brief Set the solution data
         * @param instance_name Name of the problem instance
         * @param routes Vector of routes
         */
        void set(const string &instance_name, const vector<vector<int>> &routes);

        /**
         * @brief Read solution from input stream (standard format)
         * @param is Input stream
         */
        void read(istream &is);
        
        /**
         * @brief Write solution to output stream (standard format)
         * @param os Output stream
         */
        void write(ostream &os) const;

        /**
         * @brief Read solution from JSON format
         * @param is Input stream
         */
        void read_json(istream &is);
        
        /**
         * @brief Write solution in JSON format
         * @param os Output stream
         */
        void write_json(ostream &os) const;

        /**
         * @brief Check if solution is empty
         * @return true if no routes defined
         */
        bool empty(void) const;

        /**
         * @brief Extract arcs from a specific depot's route
         * @param depot Depot/vehicle index
         * @param arcs Output vector of (from, to) arcs
         */
        void get_arcs(const size_t depot, vector<pair<int,int>> &arcs) const;

        /**
         * @brief Get the routes (const version)
         * @return Constant reference to routes
         */
        inline const vector<vector<int>> &get_routes(void) const
        {
            return routes_;
        }

        /**
         * @brief Get the routes (mutable version)
         * @return Reference to routes
         */
        inline vector<vector<int>> &get_routes(void)
        {
            return routes_;
        }

        /**
         * @brief Get the instance name
         * @return Instance name string
         */
        inline const string &get_instance_name(void) const
        {
            return instance_name_;
        }

        /**
         * @brief Write JSON header section
         * @param os Output stream
         */
        void write_header(ostream &os) const;
        
        /**
         * @brief Write routes section
         * @param os Output stream
         */
        void write_routes(ostream &os) const;
        
        /**
         * @brief Write JSON closing section
         * @param os Output stream
         */
        void write_end(ostream &os) const;

    };
}