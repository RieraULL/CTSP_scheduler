/**
 * @file sch_io.cpp
 * @brief Implementation of input/output utilities for scheduler
 * 
 * Manages file paths for instance, solution, and schedule output files.
 * Handles command-line argument parsing and stream initialization.
 */

#include "sch_io.hpp"

namespace SCH
{
    /**
     * @brief Default constructor - empty file paths
     */
    input_files::input_files(void) : ins_file(""), out_file("")
    {
    }
    /**
     * @brief Constructor with file paths
     * @param _ins_file Path to instance file (.contsp)
     * @param _out_file Path to solution file (.sol)
     */
    input_files::input_files(const string &_ins_file, const string &_out_file) : ins_file(_ins_file), out_file(_out_file)
    {
    }

    input_files::~input_files()
    {
    }

    /**
     * @brief Set input file paths
     * @param _ins_file Instance file path
     * @param _out_file Solution file path
     */
    void input_files::set(const string &_ins_file, const string &_out_file)
    {
        ins_file = _ins_file;
        out_file = _out_file;
    }

    /**
     * @brief Constructor with schedule output file
     * @param sch_file Path to schedule output file (.sched.json)
     */
    output_streams::output_streams(const string &sch_file) : sch_s(sch_file)
    {
    }

    /**
     * @brief Default constructor - stream not opened
     */
    output_streams::output_streams(void) : sch_s()
    {
    }

    /**
     * @brief Destructor - ensures proper file closure
     * 
     * Writes final newline and closes output stream.
     */
    output_streams::~output_streams()
    {
        sch_s << endl;
        sch_s.close();
    }

    /**
     * @brief Open output schedule file
     * @param sch_file Path to schedule output file
     */
    void output_streams::set(const string &sch_file)
    {
        sch_s.open(sch_file);
    }

    /**
     * @brief Parse command-line arguments and configure I/O
     * @param argc Argument count
     * @param argv Argument vector
     * @param sch_instance Output streams to configure
     * @param input_files_instance Input files to configure
     * @param prob_type Problem type to set (CTSP1 or CTSP2)
     * 
     * Expected arguments:
     * - argv[1]: Problem type ("ctsp2" or "ctsp1")
     * - argv[2]: Instance file (.contsp)
     * - argv[3]: Solution file (.sol)
     * - argv[4]: Schedule output file (.sched.json)
     * 
     * @note Exits with error if problem type is not recognized
     */
    void set_files(int argc, char **argv, output_streams &sch_instance, input_files &input_files_instance, problem_type &prob_type)
    {

        const string prob_type_s(argv[1]);

        const string ins_file(argv[2]);
        const string sol_file(argv[3]);
        const string sch_file(argv[4]);

        input_files_instance.set(ins_file, sol_file);

        sch_instance.set(sch_file);


        if (prob_type_s == "ctsp2")
            prob_type = problem_type::CTSP2;
        else
        {
            cerr << "ERROR: Incorrect problem type" << endl;
            exit(1);
        }
    }
}