#pragma once

#include <vector>
#include <string>
#include <iostream>

#include "sync_model_a_builder.hpp"

using namespace std;

namespace SYNC_LIB
{
    /**
     * @class sync_infeasible
     * @brief Exception for infeasible synchronization constraints
     *
     * This exception is thrown when a solution violates synchronization
     * constraints, indicating that no feasible schedule exists under the
     * current configuration.
     */
    class sync_infeasible
    {
    public:
        string instance_name_; ///< Name of the problem instance

    private:
        vector<double> alpha_; ///< Dual variables for arrival time constraints
        vector<double> beta_;  ///< Dual variables for route duration constraints
        vector<double> gamma_; ///< Dual variables for synchronization constraints

        vector<vector<int>> violated_cycles_; ///< Detected violated cycles in the solution

        const vector<string> &operation_names_;
        const vector<string> &routing_arc_names_; ///< Names for routing arcs (display)
        const vector<string> &sync_arc_names_;    ///< Names for sync arcs (display)
        const vector<triplet> &routing_arcs_;
        const vector<triplet> &sync_arcs_;
        const vector<double> &routing_arc_times_;

        const vector<double> &x_;

        const double tol_{1E-6}; ///< Tolerance for variable activity

    public:
        sync_infeasible(const vector<double> &x,const sync_model_a_builder &builder);
        virtual ~sync_infeasible(void);

        inline vector<double> &alpha(void) { return alpha_; }
        inline vector<double> &beta(void) { return beta_; }
        inline vector<double> &gamma(void) { return gamma_; }

        inline const vector<double> &alpha(void) const { return alpha_; }
        inline const vector<double> &beta(void) const { return beta_; }
        inline const vector<double> &gamma(void) const { return gamma_; }

        inline vector<vector<int>> &violated_cycles(void) { return violated_cycles_; }
        inline const vector<vector<int>> &violated_cycles(void) const { return violated_cycles_; }

        ostream &write_infeasible_paths(ostream &os) const;
        ostream &write_primal_dual_graph(ostream &os) const;

    private:
        ostream &write_path_(ostream &os, const vector<int> &cycle) const;
        ostream &write_(ostream &os, const vector<double> &x, const vector<double> &alpha_v, const vector<double> &beta_v) const;
    };
} // namespace SYNC_LIB