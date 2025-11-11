#pragma once

#include <vector>
#include <iostream>
#include <string>
#include <utility>

using namespace std;

/**
 * @file sync_tw.hpp
 * @brief Time windows representation for CTSP-like problems
 * 
 * This file defines structures for representing and managing time windows
 * in synchronization-constrained vehicle routing problems.
 */

namespace SYNC_LIB
{
   /**
    * @class tw_info
    * @brief Represents a time window with lower and upper bounds
    * 
    * A time window defines a valid time interval [lower_bound, upper_bound]
    * during which an operation can be executed.
    */
   class tw_info: public pair<double, double>
   {
       public:
           /// Default constructor
           tw_info(void);
           
           /**
            * @brief Construct time window with bounds
            * @param lower_bound Earliest start time
            * @param upper_bound Latest start time
            */
           tw_info(double lower_bound, double upper_bound);
           
           virtual ~tw_info(void);

           /**
            * @brief Get mutable reference to lower bound
            * @return Reference to lower bound
            */
           double &lower_bound(void);
           
           /**
            * @brief Get mutable reference to upper bound
            * @return Reference to upper bound
            */
           double &upper_bound(void);

           /**
            * @brief Get const reference to lower bound
            * @return Const reference to lower bound
            */
           const double &lower_bound(void) const;
           
           /**
            * @brief Get const reference to upper bound
            * @return Const reference to upper bound
            */
           const double &upper_bound(void) const;
   };

   /**
    * @class sync_time_windows
    * @brief Collection of time windows for all operations in a CTSP instance
    * 
    * This class stores time window information for each customer/operation
    * in the problem and provides JSON I/O capabilities.
    */
   class sync_time_windows : public vector<tw_info>
   {
       public:
           string instance_name_;  ///< Name of the problem instance

       public:
           /// Default constructor
           sync_time_windows(void);
           
           /**
            * @brief Construct with instance name and time windows
            * @param instance_name Name of the problem instance
            * @param time_windows Vector of time window information
            */
           sync_time_windows(const string &instance_name, const vector<tw_info> &time_windows);
           
           virtual ~sync_time_windows(void);

           /**
            * @brief Write time windows to output stream in JSON format
            * @param os Output stream
            * @return Reference to output stream
            */
           ostream &write_json(ostream &os) const;
           
           /**
            * @brief Read time windows from input stream in JSON format
            * @param is Input stream
            * @return Reference to input stream
            */
           istream &read_json(istream &is);
   };
}