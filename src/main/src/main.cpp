/**
 * @file main.cpp
 * @brief Entry point for CTSP scheduler application
 * 
 * This is the main executable that converts CTSP optimization solutions
 * into temporal schedules with time windows. It integrates all library
 * components to produce actionable schedules from routing solutions.
 */

#include <iostream>
#include <string>
#include <exception>

#include "schedulers.hpp"
#include "sch_io.hpp"

using namespace std;

namespace {
    /**
     * @brief Display usage information
     * @param program_name Name of the executable
     */
    void print_usage(const char* program_name) {
        std::cerr << "\n"
                  << "CTSP Scheduler - Convert routing solutions to temporal schedules\n"
                  << "================================================================\n\n"
                  << "Usage:\n"
                  << "  " << program_name << " <problem_type> <instance_file> <solution_file> <output_file>\n\n"
                  << "Arguments:\n"
                  << "  problem_type    Problem variant: 'ctsp2' (multi-depot) or 'ctsp1' (single-depot)\n"
                  << "  instance_file   Path to CTSP instance file (.contsp format)\n"
                  << "  solution_file   Path to feasible solution file (.sol format)\n"
                  << "  output_file     Path for output schedule file (.sched.json format)\n\n"
                  << "Example:\n"
                  << "  " << program_name << " ctsp2 input/bayg29.contsp input/bayg29.sol output/schedule.json\n\n";
    }

    /**
     * @brief Validate command-line arguments
     * @param argc Argument count
     * @param argv Argument vector
     * @return true if arguments are valid, false otherwise
     */
    bool validate_arguments(int argc, char** argv) {
        const int REQUIRED_ARGS = 5;
        
        if (argc != REQUIRED_ARGS) {
            std::cerr << "Error: Invalid number of arguments.\n"
                      << "Expected " << (REQUIRED_ARGS - 1) << " arguments, got " << (argc - 1) << ".\n";
            print_usage(argv[0]);
            return false;
        }
        
        return true;
    }
}

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
    // Validate command-line arguments
    if (!validate_arguments(argc, argv)) {
        return 1;
    }

    try {
        // Initialize I/O structures
        SCH::output_streams output_streams;
        SCH::input_files input_files;
        SCH::problem_type prob_type;

        // Parse command-line arguments
        SCH::set_files(argc, argv, output_streams, input_files, prob_type);

        // Execute scheduling workflow
        SCH::run_method(input_files, output_streams, prob_type);

        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "\nError: " << e.what() << "\n\n";
        return 1;
    } catch (...) {
        std::cerr << "\nUnknown error occurred during execution.\n\n";
        return 1;
    }
}
