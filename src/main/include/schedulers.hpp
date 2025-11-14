/**
 * @file schedulers.hpp
 * @brief CTSP scheduling algorithms
 *
 * This module provides the main scheduling algorithms that convert CTSP
 * solutions into temporal schedules with time windows. It bridges the
 * optimization output with practical implementation schedules.
 */

#pragma once

#include "sync_model_builder.hpp"

#include "CTSP_instance.hpp"
#include "sync_solution.hpp"

#include <iostream>
#include <fstream>
#include <string>

#include "sch_io.hpp"

using namespace std;

namespace SCH
{
    /**
     * @brief Generate schedule for CTSP2 (multi-depot) problem
     * @param I CTSP instance
     * @param initial_feasible_solution Feasible CTSP solution (routing)
     * @param output_streams_instance Output streams for schedule file
     *
     * This function:
     * 1. Builds synchronization model from CTSP instance
     * 2. Converts solution format to model_a representation
     * 3. Verifies synchronization constraints via LP
     * 4. Computes operation start times
     * 5. Generates temporal schedule for each depot
     * 6. Computes customer time windows
     * 7. Writes schedule to JSON format
     *
     * @note Asserts that solution is feasible (satisfies sync constraints)
     * @note Output format is JSON compatible with visualization tools
     */
    void CTSP2_scheduler(
        const SCH::output_files &output_files,
        const CTSP::instance &I,
        const SYNC_LIB::sync_solution &initial_feasible_solution);


    /**
     * @typedef scheduler_ptr
     * @brief Function pointer type for scheduler functions
     *
     * Points to functions that generate schedules from CTSP instances and solutions.
     */
    typedef void (*scheduler_ptr)(
        const SCH::output_files &output_files,
        const CTSP::instance &I,
        const SYNC_LIB::sync_solution &feasible_solution);

    /**
     * @typedef sch_method_ptr
     * @brief Function pointer type for scheduling methods
     *
     * Points to complete scheduling workflows (load, solve, output).
     */
    typedef int (*sch_method_ptr)(const SCH::input_files &input_files,
                                  const SCH::output_files &output_files,
                                  SCH::output_streams &os_instance);

    /**
     * @brief Complete CTSP2 scheduling workflow
     * @param input_files Input file paths (instance and solution)
     * @param os_instance Output streams for results
     * @return 0 on success
     *
     * This function implements the complete workflow:
     * 1. Load CTSP instance from file
     * 2. Load solution from file
     * 3. Generate temporal schedule
     * 4. Write schedule to output file
     *
     * @note This is the main entry point called from command line
     */
    int ctsp2_scheduler(const SCH::input_files &input_files,
                        SCH::output_streams &os_instance);

    /**
     * @brief Complete CTSP1 scheduling workflow
     * @param input_files Input file paths
     * @param os_instance Output streams
     * @return 0 on success
     */
    int ctsp1_scheduler(const SCH::input_files &input_files,
                        SCH::output_streams &os_instance);

    /**
     * @brief Run appropriate scheduler based on problem type
     * @param input_files Input file paths
     * @param sch_instance Output streams
     * @param prob_type Problem type identifier (CTSP1 or CTSP2)
     * @return 0 on success, non-zero on error
     *
     * This function dispatches to the appropriate scheduler based on
     * the problem type. It uses function pointer arrays for efficient
     * dispatching.
     *
     * @note Currently only CTSP2 is fully implemented
     */
    int run_method(const SCH::input_files &input_files,
                   const SCH::output_files &output_files,
                   SCH::output_streams &sch_instance,
                   SCH::problem_type prob_type);

}