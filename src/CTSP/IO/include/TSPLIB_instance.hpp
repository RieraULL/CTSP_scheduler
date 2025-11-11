/**
 * @file TSPLIB_instance.hpp
 * @brief TSPLIB format instance reader
 * 
 * This module provides a comprehensive parser for TSPLIB-format instance files,
 * commonly used in Traveling Salesman Problem benchmarks and variants.
 * 
 * TSPLIB is a standard format for TSP instances developed by Gerhard Reinelt.
 * This implementation supports various distance calculation methods and data formats.
 * 
 * @see http://comopt.ifi.uni-heidelberg.de/software/TSPLIB95/
 */

#pragma once

#include <vector>
#include <iostream>
#include <string>
#include <utility>
#include <cmath>
#include <algorithm>
#include "matrix.hpp"

using namespace std;

#define NCTYPE_NUM 3
#define WTYPE_NUM 10
#define WFORMAT_NUM 9
#define DTYPE_NUM 3
#define KEY_NUM 25

namespace TSP
{
    /// Coordinate data type (typically double precision)
    typedef double coordItemType;
    
    /// 2D coordinate pair (x, y)
    typedef pair<coordItemType, coordItemType> coordType;

    /**
     * @class TSPLIB_instance
     * @brief Parser and container for TSPLIB format instances
     * 
     * This class reads and parses TSPLIB format files, which include:
     * - **Classic TSP**: Single-visit routing problems
     * - **VRP variants**: Vehicle routing with capacity constraints
     * - **PTSP/CTSP**: Periodic/Consistent TSP with time windows
     * 
     * **Supported Features:**
     * - Multiple distance calculation methods (Euclidean, Manhattan, Geographic, etc.)
     * - Various edge weight formats (matrices, coordinate-based)
     * - Time windows and demand information
     * - Multi-day/multi-vehicle extensions
     * 
     * **Distance Types:**
     * - `EUC_2D`: Euclidean distance in 2D
     * - `GEO`: Geographic distance (latitude/longitude)
     * - `MAN_2D`: Manhattan distance
     * - `CEIL_2D`: Ceiling of Euclidean distance
     * - `ATT`: Pseudo-Euclidean distance
     * - `EXPLICIT`: Direct distance matrix
     * 
     * **File Format Example:**
     * ```
     * NAME : bayg29
     * TYPE : TSP
     * DIMENSION : 29
     * EDGE_WEIGHT_TYPE : EUC_2D
     * NODE_COORD_SECTION
     * 1 1150.0 1760.0
     * 2 630.0 1660.0
     * ...
     * EOF
     * ```
     * 
     * @note This class is the foundation for PTSP and CTSP instance readers
     * @note Supports extended keywords for multi-day and time window problems
     */
    class TSPLIB_instance
    class TSPLIB_instance
    {
        /// Function pointer type for distance calculation methods
        typedef double (TSPLIB_instance::*distanceType)(const coordType &, const coordType &) const;
        
        /// Function pointer type for section reading methods
        typedef void (TSPLIB_instance::*readType)(istream &is, ostream &os);

    private:
        /// TSPLIB keywords (standard and extensions)
        const vector<string> keywords = {
            /* 0*/ "NAME",
            /* 1*/ "TYPE",
            /* 2*/ "COMMENT",
            /* 3*/ "DIMENSION",
            /* 4*/ "CAPACITY",
            /* 5*/ "EDGE_WEIGHT_TYPE",
            /* 6*/ "EDGE_WEIGHT_FORMAT",
            /* 7*/ "DISPLAY_DATA_TYPE",
            /* 8*/ "EDGE_WEIGHT_SECTION",
            /* 9*/ "DISPLAY_DATA_SECTION",
            /*10*/ "NODE_COORD_SECTION",
            /*11*/ "NODE_COORD_TYPE",
            /*12*/ "DEPOT_SECTION",
            /*13*/ "CAPACITY_VOL",
            /*14*/ "DEMAND_SECTION",
            /*15*/ "TIME_WINDOW_SECTION",
            /*16*/ "STANDTIME_SECTION",
            /*17*/ "PICKUP_SECTION",
            /*18*/ "EOF",
            /*19*/ "NUMBER_OF_TRUCKS",
            /*20*/ "NUM_DAYS",              // Extension for multi-day problems
            /*21*/ "DISTANCE",              // Extension for max distance constraint
            /*22*/ "MAXIMUM_ALLOWABLE_DIFFERENTIAL"}; // Extension for CTSP time windows

        /// Edge weight types (distance calculation methods)
        const vector<string> wtypes = {
            "EXPLICIT",    // Distance matrix provided explicitly
            "EUC_2D",      // 2D Euclidean distance
            "EUC_3D",      // 3D Euclidean distance
            "MAX_2D",      // 2D maximum metric
            "MAX_3D",      // 3D maximum metric
            "MAN_2D",      // 2D Manhattan distance
            "MAN_3D",      // 3D Manhattan distance
            "CEIL_2D",     // Ceiling of 2D Euclidean
            "GEO",         // Geographic distance (lat/lon)
            "ATT"};        // Special distance for att48 instance

        /// Edge weight matrix formats
        const vector<string> wformats = {
            "UPPER_ROW",      // Upper triangular by rows
            "LOWER_ROW",      // Lower triangular by rows
            "UPPER_DIAG_ROW", // Upper triangular with diagonal by rows
            "LOWER_DIAG_ROW", // Lower triangular with diagonal by rows
            "UPPER_COL",      // Upper triangular by columns
            "LOWER_COL",      // Lower triangular by columns
            "UPPER_DIAG_COL", // Upper triangular with diagonal by columns
            "LOWER_DIAG_COL", // Lower triangular with diagonal by columns
            "FULL_MATRIX"};   // Full n×n matrix

        /// Display data types
        const vector<string> dtypes = {
            "COORD_DISPLAY",  // Display using coordinates
            "TWOD_DISPLAY",   // 2D display
            "NO_DISPLAY"};    // No display data

        const vector<string> messages = {
            "--  Reading input file  --: \n"};

    private:
        string name_;                     ///< Instance name
        string type_;                     ///< Problem type (TSP, CVRP, etc.)
        string comment_;                  ///< Instance description/comment
        int dimension_;                   ///< Number of nodes (including depot)
        int edge_weight_type_;            ///< Distance calculation method index
        int edge_weight_format_;          ///< Matrix format index
        int display_data_type_;           ///< Display format index
        int num_days_;                    ///< Number of days (PTSP/CTSP extension)
        int max_distance_;                ///< Maximum route distance (CTSP extension)
        int maximum_allowable_differencial_; ///< Max time window width (CTSP extension)
        int depot_;                       ///< Depot node index

        double *distances_; ///< Distance matrix (1D array)

        vector<int> coord_id_;            ///< Node IDs for coordinates
        vector<coordType> coord_;         ///< Node coordinates (x, y)

        vector<int> display_id_;          ///< Node IDs for display
        vector<coordType> display_;       ///< Display coordinates

        vector<vector<int>> demand_;      ///< Demands per node per day

        vector<distanceType> distance_function_;           ///< Distance calculation functions
        vector<readType> read_function_;                   ///< Section reading functions
        vector<readType> edge_weight_reading_function_;    ///< Weight matrix reading functions

        vector<double> optimal_values_; ///< Known optimal/best solutions

    public:
        /**
         * @brief Default constructor
         */
        TSPLIB_instance(void);
        
        /**
         * @brief Destructor
         */
        virtual ~TSPLIB_instance(void);

        /**
         * @brief Read TSPLIB instance from file
         * @param input_file Path to TSPLIB format file
         * @note Automatically detects format and distance type
         * @note Computes distance matrix if coordinates are provided
         */
        void read(const string &input_file);

        void read(const string &input_file);

        // Getters
        inline const string &get_instance_name(void) const
        {
            return name_;
        }

        inline const string &get_instance_type(void) const
        {
            return type_;
        }

        inline const string &get_instance_comment(void) const
        {
            return comment_;
        }

        inline const int &get_dimension(void) const
        {
            return dimension_;
        }

        inline const vector<double> &get_optimal_values(void) const
        {
            return optimal_values_;
        }

        inline const vector<coordType> &coord(void) const
        {
            return coord_;
        }

        /**
         * @brief Get distance matrix
         * @param distances_ [output] Matrix to store distances (will be resized)
         * @note Matrix uses 1-based indexing
         */
        void get_distances(GOMA::matrix<double> &distances_) const;

        inline const vector<vector<int>> &get_demands(void) const
        {
            return demand_;
        }

        inline const int &get_depot(void) const
        {
            return depot_;
        }

        inline const int &get_num_days(void) const
        {
            return num_days_;
        }

        inline const int &get_max_distance(void) const
        {
            return max_distance_;
        }

        inline const int &get_maximum_allowable_differencial(void) const
        {
            return maximum_allowable_differencial_;
        }

    private:
        /**
         * @brief Compute distance matrix from coordinates
         * @note Uses the appropriate distance function based on edge_weight_type_
         */
        void compute_implicit_distance_matrix_(void);

        // Section reading methods (one per TSPLIB keyword)
        void read_name_section_(istream &is, ostream &os);
        void read_type_section_(istream &is, ostream &os);
        void read_comment_section_(istream &is, ostream &os);
        void read_dimension_section_(istream &is, ostream &os);
        void read_capacity_section_(istream &is, ostream &os);
        void read_edge_weight_type_section_(istream &is, ostream &os);
        void read_edge_weigh_format_section_(istream &is, ostream &os);
        void read_edge_weight_section_(istream &is, ostream &os);
        void read_display_data_type_section_(istream &is, ostream &os);
        void read_node_coord_section_(istream &is, ostream &os);
        void read_node_coord_type_section_(istream &is, ostream &os);
        void read_depot_section_(istream &is, ostream &os);
        void read_capacity_vol_section_(istream &is, ostream &os);
        void read_demand_section_(istream &is, ostream &os);
        void read_time_window_section_(istream &is, ostream &os);
        void read_standtime_section_(istream &is, ostream &os);
        void read_pickup_section_(istream &is, ostream &os);
        void read_EOF_section_(istream &is, ostream &os);
        void read_number_of_trucks_section_(istream &is, ostream &os);
        void read_num_days_section_(istream &is, ostream &os);
        void read_distance_section_(istream &is, ostream &os);
        void read_display_data_section_(istream &is, ostream &os);
        void read_maximum_allowable_differencial_section_(istream &is, ostream &os);

        // Edge weight matrix reading methods (one per format)
        void read_edge_weight_section_upper_row_(istream &is, ostream &os);
        void read_edge_weight_section_lower_row_(istream &is, ostream &os);
        void read_edge_weight_section_upper_diag_row_(istream &is, ostream &os);
        void read_edge_weight_section_lower_diag_row_(istream &is, ostream &os);
        void read_edge_weight_section_upper_col_(istream &is, ostream &os);
        void read_edge_weight_section_lower_col_(istream &is, ostream &os);
        void read_edge_weight_section_upper_diag_col_(istream &is, ostream &os);
        void read_edge_weight_section_lower_diag_col_(istream &is, ostream &os);
        void read_edge_weight_section_full_matrix_(istream &is, ostream &os);

        // Distance calculation methods (one per distance type)
        double compute_euc_2d_distance_(const coordType &a, const coordType &b) const;
        double compute_max_2d_distance_(const coordType &a, const coordType &b) const;
        double compute_man_2d_distance_(const coordType &a, const coordType &b) const;
        double compute_ceil_2d_distance_(const coordType &a, const coordType &b) const;
        double compute_geo_distance_(const coordType &a, const coordType &b) const;
        double compute_att_distance_(const coordType &a, const coordType &b) const;


        void establish_dimension_(const int dimension);

        void clean_(const string &i_file, const string &o_file);
        void clean_(istream &is, ostream &os);

        void read_(istream &is, ostream &os);

        int find_key_(const string &token, const vector<string> keywords) const;

        int nint_(double x) const;
        double dtrunc_(double x) const;
        void radian_coords_(const coordType &a, double &longitude, double &latitude) const;
    };

}