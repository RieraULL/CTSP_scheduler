/**
 * @file sch_io.hpp
 * @brief Scheduler Input/Output utilities
 *
 * This module provides utilities for managing input files and output streams
 * for the CTSP scheduler application. It handles command-line argument parsing
 * and file stream management.
 */

#pragma once

#include <iostream>
#include <fstream>
#include <string>

using namespace std;

namespace SCH
{
    /**
     * @class input_files
     * @brief Container for input file paths
     *
     * Manages the paths to the instance file and solution file required
     * by the scheduler.
     */
    class input_files
    {
    public:
        string ins_file; ///< Path to instance file (.contsp format)
        string sol_file; ///< Path to solution file (.sol format)

        /**
         * @brief Constructor with file paths
         * @param _ins_file Path to instance file
         * @param _sol_file Path to solution file
         */
        input_files(const string &_ins_file, const string &_sol_file);

        /**
         * @brief Default constructor
         */
        input_files(void);

        /**
         * @brief Destructor
         */
        ~input_files();

        /**
         * @brief Set input file paths
         * @param _ins_file Path to instance file
         * @param _out_file Path to solution file
         */
        void set(const string &_ins_file, const string &_out_file);
    };

    class output_files
    {
    public:
        string output_path;
        string instance_name;

        output_files(const string &_output_path, const string &_ins_file);
        ~output_files(void);

        void set(const string &_output_path, const string &_ins_file);
    };

    /**
     * @class output_streams
     * @brief Container for output file streams
     *
     * Manages the output stream for writing scheduling results.
     * Automatically closes the stream on destruction.
     */
    class output_streams
    {
    public:
        ofstream sch_s; ///< Output stream for schedule file (.sched.json format)

        /**
         * @brief Constructor with output file
         * @param sch_file Path to schedule output file
         */
        output_streams(const string &sch_file);

        /**
         * @brief Default constructor
         */
        output_streams(void);

        /**
         * @brief Destructor - automatically closes output stream
         */
        ~output_streams(void);

        /**
         * @brief Open output stream with file path
         * @param sch_file Path to schedule output file
         */
        void set(const string &sch_file);
    };

    /**
     * @enum problem_type
     * @brief Supported problem variants
     *
     * Identifies which CTSP variant to solve.
     */
    enum class problem_type
    {
        CTSP2 ///< Multi-depot Consistent TSP
    };

    /**
     * @brief Parse command-line arguments and set up I/O
     * @param argc Argument count
     * @param argv Argument vector
     * @param sch_instance Output streams (will be initialized)
     * @param input_files_instance Input files (will be initialized)
     * @param prob_type Problem type (will be set)
     *
     * **Expected Command Line:**
     * ```
     * ctsp_scheduler <problem_type> <instance_file> <solution_file> <schedule_output>
     * ```
     *
     * **Example:**
     * ```bash
     * ./ctsp_scheduler ctsp2 input/bayg29.contsp input/bayg29.sol output/bayg29.sched.json
     * ```
     *
     * @note Exits program with error if problem_type is not recognized
     */
    void set_files(int argc, char **argv, output_streams &sch_instance, input_files &input_files_instance, problem_type &prob_type);

}