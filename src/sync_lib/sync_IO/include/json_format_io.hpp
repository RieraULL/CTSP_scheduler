#pragma once

#include <vector>
#include <iostream>
#include <string>
#include <utility>

#include "sync_scheduling.hpp"

using namespace std;

/**
 * @file json_format_io.hpp
 * @brief JSON format input/output utilities for CTSP-like problems
 * 
 * This file provides a simple, ad-hoc JSON parser and writer for CTSP solutions
 * and schedules. While functional, this is a basic implementation - consider using
 * established JSON libraries (e.g., nlohmann/json, RapidJSON) for production use.
 * 
 * Note: This implementation does not use a full JSON parser library and has
 * limitations compared to standard JSON libraries.
 */

namespace SYNC_LIB
{
    /**
     * @class json_format_io
     * @brief Utility class for reading and writing CTSP data in JSON format
     * 
     * This class provides methods for serializing and deserializing various
     * data structures used in CTSP problems:
     * - Solutions (routes)
     * - Schedules (routes with timing information)
     * - Time windows
     * - Generic vectors and matrices
     * 
     * The JSON format is human-readable and suitable for storing intermediate
     * results and final solutions.
     */
    class json_format_io
    {
    public:
        json_format_io(void);
        virtual ~json_format_io(void);

        /**
         * @brief Write a solution (routes only) in JSON format
         * @param os Output stream
         * @param instance_name Name of the problem instance
         * @param routes Vector of routes (each route is a sequence of customers)
         */
        void write_sol(ostream &os, const string &instance_name, const vector<vector<int>> &routes) const;
        
        /**
         * @brief Read a solution from JSON format
         * @param is Input stream
         * @param instance_name Output: name of the instance
         * @param routes Output: vector of routes
         */
        void read_sol(istream &is, string &instance_name, vector<vector<int>> &routes) const;

        /**
         * @brief Write a complete schedule with timing information
         * @param os Output stream
         * @param instance_name Name of the problem instance
         * @param schedules Schedule data with timing for each operation
         */
        void write(ostream &os, const string &instance_name, const sync_scheduling &schedules) const;

        /**
         * @brief Read a vector of integers from JSON array format
         * @param is Input stream
         * @param vec Output vector
         */
        void read_integer_vector(istream &is, vector<int> &vec) const;
        
        /**
         * @brief Write a vector of integers in JSON array format
         * @param os Output stream
         * @param vec Input vector
         */
        void write_integer_vector(ostream &os, const vector<int> &vec) const;

        /**
         * @brief Read a vector of doubles from JSON array format
         * @param is Input stream
         * @param vec Output vector
         */
        void read_double_vector(istream &is, vector<double> &vec) const;
        
        /**
         * @brief Write a vector of doubles in JSON array format
         * @param os Output stream
         * @param vec Input vector
         */
        void write_double_vector(ostream &os, const vector<double> &vec) const;

        /**
         * @brief Read a 2D vector of integers from JSON
         * @param is Input stream
         * @param vec Output 2D vector
         * @return true if successful
         */
        bool read_vector_of_integer_vectors(istream &is, vector<vector<int>> &vec) const;
        
        /**
         * @brief Read a 2D vector of doubles from JSON
         * @param is Input stream
         * @param vec Output 2D vector
         * @return true if successful
         */
        bool read_vector_of_double_vectors(istream &is, vector<vector<double>> &vec) const;
        
        /**
         * @brief Write a 2D vector of integers in JSON format
         * @param os Output stream
         * @param vec Input 2D vector
         */
        void write_vector_of_integer_vectors(ostream &os, const vector<vector<int>> &vec) const;
        
        /**
         * @brief Write a 2D vector of doubles in JSON format
         * @param os Output stream
         * @param vec Input 2D vector
         */
        void write_vector_of_double_vectors(ostream &os, const vector<vector<double>> &vec) const;
        
        /**
         * @brief Write scheduling information (routes with timing) in JSON format
         * @param os Output stream
         * @param vec Scheduling data: for each route, list of (customer, timing) info
         */
        void write_vector_of_scheduling(ostream &os, const vector<vector<operation_info>> &vec) const;

        /**
         * @brief Write a pair of doubles in JSON format
         * @param os Output stream
         * @param p Pair to write
         */
        void write_pair_(ostream &os, const pair<double, double> &p) const;
        
        /**
         * @brief Read a pair of doubles from JSON format
         * @param is Input stream
         * @param p Output pair
         */
        void read_pair_(istream &is, pair<double, double> &p) const;

        /**
         * @brief Write a vector of pairs in JSON format
         * @param os Output stream
         * @param vec Vector of pairs
         */
        void write_vector_of_pairs(ostream &os, const vector<pair<double, double>> &vec) const;
        
        /**
         * @brief Read a vector of pairs from JSON format
         * @param is Input stream
         * @param vec Output vector of pairs
         */
        void read_vector_of_pairs(istream &is, vector<pair<double, double>> &vec) const;

        /**
         * @brief Read next non-whitespace character from stream
         * @param is Input stream
         * @return Next non-space character
         * 
         * Utility method for parsing JSON by skipping whitespace
         */
        char read_next_non_space_char(istream &is) const;
    };
}