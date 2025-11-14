#pragma once

#include <vector>
#include <string>
#include <iostream>

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

    public:
        sync_infeasible(void);
        virtual ~sync_infeasible(void);

        inline vector<double> &alpha(void) { return alpha_; }
        inline vector<double> &beta(void) { return beta_; }
        inline vector<double> &gamma(void) { return gamma_; }

        inline const vector<double> &alpha(void) const { return alpha_; }
        inline const vector<double> &beta(void) const { return beta_; }
        inline const vector<double> &gamma(void) const { return gamma_; }

        inline vector<vector<int>> &violated_cycles(void) { return violated_cycles_; }
        inline const vector<vector<int>> &violated_cycles(void) const { return violated_cycles_; }
    };
} // namespace SYNC_LIB