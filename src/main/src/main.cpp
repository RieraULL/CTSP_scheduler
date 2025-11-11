/**
 * @file main.cpp
 * @brief Entry point for CTSP scheduler application
 * 
 * This is the main executable that converts CTSP optimization solutions
 * into temporal schedules with time windows. It integrates all library
 * components to produce actionable schedules from routing solutions.
 */

#include <cstdio>
#include <cstdlib>

#include "schedulers.hpp"
#include "sch_io.hpp"

using namespace std;

/**
 * @brief Main entry point
 * @param argc Argument count (must be 5)
 * @param argv Argument vector:
 *   - argv[0]: Program name
 *   - argv[1]: Problem type ("ctsp2" or "ctsp1")
 *   - argv[2]: Instance file path (.contsp format)
 *   - argv[3]: Solution file path (.sol format)
 *   - argv[4]: Output file path (.sched.json)
 * @return 0 on success, 1 on error
 * 
 * @note Requires exactly 4 arguments plus program name
 * @note Output is written to JSON file with schedules for each depot
 * 
 * Example usage:
 * @code
 * ./ctsp_scheduler ctsp2 instance.contsp solution.sol output.sched.json
 * @endcode
 */
int main(int argc, char **argv)
{

    if (argc == 5)
    {
        SCH::output_streams output_streams;
        SCH::input_files input_files;
        SCH::problem_type prob_type;

        SCH::set_files(argc, argv, output_streams, input_files, prob_type);

        SCH::run_method(input_files, output_streams, prob_type);

        return 0;
    }

    return 1;
}
