/**
 * @file schedulers.cpp
 * @brief Implementation of CTSP scheduling algorithms
 *
 * Converts CTSP routing solutions into temporal schedules by:
 * 1. Building synchronization constraint model
 * 2. Converting solution format
 * 3. Computing operation start times
 * 4. Generating customer time windows
 * 5. Writing schedule to JSON format
 */

#include "schedulers.hpp"

#include "sync_infeasible.hpp"

#include "sync_model_a_builder.hpp"
#include "CTSP_model_a_builder.hpp"
#include "model_a_solution_interface.hpp"

#include "sol_2_scheduling.hpp"

namespace SCH
{

    /**
     * @brief Generate temporal schedule for CTSP2 problem
     * @param instance CTSP instance with customers, depots, and constraints
     * @param feas_sol Feasible routing solution
     * @param output_streams Output file stream for schedule
     *
     * Implementation steps:
     * 1. Build CTSP model_a from instance
     * 2. Create scheduling solver with tolerance 1e-6
     * 3. Convert sync_solution to model_a format
     * 4. Solve LP to compute operation times and time windows
     * 5. Write JSON schedule to output
     *
     * @note Asserts that solution is feasible (LP has solution)
     * @note Output format includes schedules per depot and time windows per customer
     */
    void CTSP2_scheduler(const SCH::output_files &output_files, const CTSP::instance &instance, const SYNC_LIB::sync_solution &feas_sol)
    {
        // Build synchronization model from CTSP instance
        CTSP::CTSP_model_a_builder model_builder(CTSP::CTSP_problem_type::CTSP2, instance);

        // Create scheduler with numerical tolerance
        SYNC_LIB::conTSP2_scheduling scheduler(model_builder, 1e-6);

        // Convert solution to model_a format
        vector<double> x;
        {
            SYNC_LIB::model_a_solution_interface solution_interfaz;
            solution_interfaz.set(model_builder);
            solution_interfaz.sync_solution_2_model_a(feas_sol, x);
        }

        // Compute schedule and time windows via LP
        SYNC_LIB::sync_scheduling feasible_schedule;
        SYNC_LIB::sync_infeasible infeasible_paths(x, model_builder);

        const bool feasible{scheduler.solve(feas_sol.get_instance_name(), x, feasible_schedule, infeasible_paths)};

        if (feasible)
        {
            std::ofstream schedule_file(output_files.output_path + "/" + output_files.instance_name + ".sched.json");
            std::ostream& sch_s = schedule_file;

            // Write schedule to JSON output
            feas_sol.write_header(sch_s);
            sch_s << endl;
            feasible_schedule.write_json(sch_s);
            feas_sol.write_end(sch_s);

            schedule_file.close();
        }
        else
        {
            std::ofstream infeasible_paths_file(output_files.output_path + "/" + output_files.instance_name + "_infeasible_paths.txt");
            std::ofstream primal_dual_graph_file(output_files.output_path + "/" + output_files.instance_name + "_primal_dual_graph.dot");
            infeasible_paths.write_infeasible_paths(infeasible_paths_file);
            infeasible_paths.write_primal_dual_graph(primal_dual_graph_file);
            infeasible_paths_file.close();
            primal_dual_graph_file.close();
        }

        // Verify solution feasibility
    }

    /**
     * @brief Array of scheduler function pointers
     * @note Index 0: CTSP2_scheduler
     */
    scheduler_ptr scheduler_array[] = {CTSP2_scheduler};

    /**
     * @brief Complete CTSP2 scheduling workflow
     * @param input_files Input file paths (instance, solution)
     * @param os_instance Output streams for schedule
     * @return 0 on success
     *
     * Workflow:
     * 1. Load CTSP instance from .contsp file
     * 2. Load solution from .sol file
     * 3. Generate temporal schedule
     * 4. Write to .sched.json file
     */
    int ctsp2_scheduler(const SCH::input_files &input_files,
                        const SCH::output_files &output_files,
                        SCH::output_streams &os_instance)
    {
        // Load instance and solution from files
        CTSP::instance I(input_files.ins_file);
        SYNC_LIB::sync_solution feas_sol(input_files.sol_file);

        // Generate schedule
        (*scheduler_array[0])(output_files, I, feas_sol);

        return 0;
    }

    /**
     * @brief Array of scheduling method function pointers
     * @note Index 0: ctsp2_scheduler
     */
    sch_method_ptr sch_method_array[] = {ctsp2_scheduler};

    /**
     * @brief Dispatch to appropriate scheduler based on problem type
     * @param input_files Input file paths
     * @param sch_instance Output streams
     * @param prob_type Problem type (CTSP1=0 or CTSP2=0)
     * @return 0 on success
     *
     * Uses function pointer array for efficient dispatching.
     * Currently only CTSP2 is implemented.
     */
    int run_method(const SCH::input_files &input_files,
                   const SCH::output_files &output_files,
                   SCH::output_streams &sch_instance,
                   SCH::problem_type prob_type)
    {
        return (*sch_method_array[static_cast<int>(prob_type)])(input_files, output_files, sch_instance);
    }

}