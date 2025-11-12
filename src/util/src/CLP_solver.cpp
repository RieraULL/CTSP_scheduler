#include "CLP/CLP_solver.hpp"
#include "CLP/CLP_model_structure.hpp"

#include <ClpSimplex.hpp>
#include <CoinPackedMatrix.hpp>
#include <CoinPackedVector.hpp>
#include <limits>
#include <stdexcept>
#include <cstring>

namespace GOMA
{
    CLP_solver::CLP_solver(const model_description &model, const double tol) : LP_solver(model, tol),
                                                                               model_(nullptr)
    {
        init_solver();
        CLP_model_structure clp_model(model, tol);
        build_model(clp_model);
    }

    CLP_solver::~CLP_solver()
    {
        clear();
    }

    void CLP_solver::clear(void)
    {
        if (model_ != nullptr)
        {
            delete model_;
            model_ = nullptr;
        }
    }

    int CLP_solver::get_nz(void) const
    {
        if (model_ == nullptr)
            return 0;
        
        const CoinPackedMatrix* matrix = model_->matrix();
        if (matrix == nullptr)
            return 0;
        
        return matrix->getNumElements();
    }

    double CLP_solver::get_obj(void) const
    {
        if (model_ == nullptr)
            return 1e20;

        // Check solver status
        // 0 = optimal, 1 = primal infeasible, 2 = dual infeasible (unbounded)
        int status = model_->status();
        
        if (status == 0)  // Optimal
        {
            return model_->objectiveValue();
        }
        else if (status == 2)  // Unbounded
        {
            return -1e20;
        }
        else  // Infeasible or error
        {
            return 1e20;
        }
    }

    void CLP_solver::disable_prep_linear(void)
    {
        if (model_ != nullptr)
        {
            // CLP no expone un control directo equivalente; dejamos no-op
            // (se podría usar pre-resolve externo o parámetros avanzados si fuera necesario)
        }
    }

    void CLP_solver::get_vars(double *alpha) const
    {
        if (model_ == nullptr || alpha == nullptr)
            return;

        const double *solution = model_->primalColumnSolution();
        if (solution == nullptr)
            return;

        memcpy(alpha, solution, n_col_ * sizeof(double));
    }

    void CLP_solver::get_dual_vars(double *alpha) const
    {
        if (model_ == nullptr || alpha == nullptr)
            return;

        const double *duals = model_->dualRowSolution();
        if (duals == nullptr)
            return;

        memcpy(alpha, duals, n_row_ * sizeof(double));
    }

    void CLP_solver::set_obj(double *obj_coef, int *obj_inx, int sz)
    {
        if (model_ == nullptr)
            return;

        double *obj = model_->objective();
        
        for (int i = 0; i < sz; ++i)
        {
            int idx = obj_inx[i];
            if (idx >= 0 && idx < n_col_)
            {
                obj[idx] = obj_coef[i];
            }
        }
    }

    void CLP_solver::set_rhs(int cnt, const int *rhs_inx, const double *rhs_val)
    {
        if (model_ == nullptr)
            return;

        double *row_lower = model_->rowLower();
        double *row_upper = model_->rowUpper();

        for (int i = 0; i < cnt; ++i)
        {
            int idx = rhs_inx[i];
            if (idx >= 0 && idx < n_row_)
            {
                // Need to know the sense to update correctly
                // For now, assume equality constraints (both bounds equal)
                // This is a simplification - in production you'd track constraint types
                double rhs = rhs_val[i];
                
                if (row_lower[idx] == row_upper[idx])  // Equality
                {
                    row_lower[idx] = rhs;
                    row_upper[idx] = rhs;
                }
                else if (row_lower[idx] > -1e20)  // Greater than
                {
                    row_lower[idx] = rhs;
                }
                else  // Less than
                {
                    row_upper[idx] = rhs;
                }
            }
        }
    }

    void CLP_solver::set_rhs(const int row, const double val)
    {
        set_rhs(1, &row, &val);
    }

    void CLP_solver::set_bdn(double *obj_coef, char *sense, int *obj_inx, int sz)
    {
        if (model_ == nullptr)
            return;

        double *col_lower = model_->columnLower();
        double *col_upper = model_->columnUpper();
        const double CLP_INFINITY = std::numeric_limits<double>::infinity();

        for (int i = 0; i < sz; ++i)
        {
            int idx = obj_inx[i];
            if (idx >= 0 && idx < n_col_)
            {
                if (sense[i] == 'L')  // Lower bound
                {
                    col_lower[idx] = obj_coef[i];
                }
                else if (sense[i] == 'U')  // Upper bound
                {
                    col_upper[idx] = obj_coef[i];
                }
                else if (sense[i] == 'B')  // Both bounds (binary)
                {
                    col_lower[idx] = 0.0;
                    col_upper[idx] = 1.0;
                }
            }
        }
    }

    void CLP_solver::del_rows(int begin, int end)
    {
        if (model_ == nullptr || begin < 0 || end >= n_row_)
            return;

        int num_delete = end - begin + 1;
        int *which = new int[num_delete];
        
        for (int i = 0; i < num_delete; ++i)
        {
            which[i] = begin + i;
        }

        model_->deleteRows(num_delete, which);
        
        delete[] which;
        
        n_row_ = model_->numberRows();
    }

    void CLP_solver::add_cut(int nzcnt, double const *rhs, char const *sense, 
                             int const *rmatbeg, int const *rmatind, 
                             double const *rmatval, char **rowname)
    {
        if (model_ == nullptr)
            return;

        // Add one row at a time (nzcnt is number of rows, not non-zeros)
        for (int r = 0; r < nzcnt; ++r)
        {
            int start = rmatbeg[r];
            int end = (r < nzcnt - 1) ? rmatbeg[r + 1] : get_nz();
            int num_elements = end - start;

            CoinPackedVector row;
            for (int i = start; i < end; ++i)
            {
                row.insert(rmatind[i], rmatval[i]);
            }

            double row_lower, row_upper;
            convert_row_bounds(sense[r], rhs[r], row_lower, row_upper);

            const int num = row.getNumElements();
            const int* idx = row.getIndices();
            const double* elts = row.getElements();
            model_->addRow(num, idx, elts, row_lower, row_upper);
        }

        n_row_ = model_->numberRows();
    }

    void CLP_solver::set_coef(int cnt, const int *row_inx, const int *col_inx, const double *coef_val)
    {
        if (model_ == nullptr)
            return;

        CoinPackedMatrix *matrix = const_cast<CoinPackedMatrix*>(model_->matrix());
        if (matrix == nullptr)
            return;

        for (int i = 0; i < cnt; ++i)
        {
            int row = row_inx[i];
            int col = col_inx[i];
            
            if (row >= 0 && row < n_row_ && col >= 0 && col < n_col_)
            {
                matrix->modifyCoefficient(row, col, coef_val[i]);
            }
        }
    }

    int CLP_solver::get_n_rows(void) const
    {
        if (model_ == nullptr)
            return 0;
        
        return model_->numberRows();
    }

    void CLP_solver::init_solver(void)
    {
        if (model_ != nullptr)
        {
            delete model_;
        }

        model_ = new ClpSimplex();
        
        // Set up some default options
        model_->setLogLevel(0);  // Quiet mode (0 = no output)
        
        // Use dual simplex (generally faster for most problems)
        model_->setOptimizationDirection(1);  // 1 = minimize, -1 = maximize
    }

    void CLP_solver::build_model(const CLP_model_structure &model)
    {
        if (model_ == nullptr)
            return;

        const int ncol = model.ncol();
        const int nrow = model.nrow();

        // Build constraint matrix in column-major format
        const vector<int>& starts = model.get_matbeg();
        const vector<int>& indices = model.get_matind();
        const vector<double>& values = model.get_matval();

        CoinPackedMatrix matrix(true,  // column-ordered
                               nrow,
                               ncol,
                               values.size(),
                               values.data(),
                               indices.data(),
                               starts.data(),
                               nullptr);  // lengths (nullptr means use starts[i+1]-starts[i])

        // Convert constraint sense to row bounds
        const vector<char>& sense = model.get_sense();
        const vector<double>& rhs = model.get_rhs();
        
        vector<double> row_lower(nrow);
        vector<double> row_upper(nrow);
        
        for (int i = 0; i < nrow; ++i)
        {
            convert_row_bounds(sense[i], rhs[i], row_lower[i], row_upper[i]);
        }

        // Load problem into CLP
        model_->loadProblem(matrix,
                           model.get_lb().data(),
                           model.get_ub().data(),
                           model.get_obj().data(),
                           row_lower.data(),
                           row_upper.data());

        // Set objective sense
        model_->setOptimizationDirection(model.get_obj_sense());

        // Handle integer variables if any (though CLP is LP-only)
        const vector<char>& ctype = model.get_ctype();
        for (int i = 0; i < ncol; ++i)
        {
            if (ctype[i] == 'B' || ctype[i] == 'I')
            {
                // CLP doesn't handle integer variables directly
                // Would need CBC (COIN-OR Branch and Cut) for MIP
                fprintf(stderr, "Warning: Integer variable detected in column %d. CLP is LP-only. Use CBC for MIP.\n", i);
            }
        }

        n_col_ = ncol;
        n_row_ = nrow;
    }

    void CLP_solver::solve_LP(void)
    {
        if (model_ == nullptr)
            return;

        // Use dual simplex (generally robust and fast)
        model_->dual();

        // Update solver status
        lpstat_ = model_->status();
        
        // 0 = optimal
        // 1 = primal infeasible
        // 2 = dual infeasible (unbounded)
        // 3 = stopped on iterations or time
        // 4 = stopped due to errors
        
        // Map to our status convention (1 = optimal)
        if (lpstat_ == 0)
            lpstat_ = 1;  // optimal
        else if (lpstat_ == 2)
            lpstat_ = 2;  // unbounded
        else
            lpstat_ = 0;  // infeasible or error
    }

    void CLP_solver::solve_MIP(void)
    {
        fprintf(stderr, "Error: CLP is an LP solver only. For MIP, use CBC (COIN-OR Branch and Cut).\n");
        throw std::runtime_error("MIP solving not supported in CLP. Use CBC instead.");
    }

    void CLP_solver::write_model(const char *filename) const
    {
        if (model_ == nullptr || filename == nullptr)
            return;

        string fname(filename);
        
        if (fname.find(".lp") != string::npos)
        {
            model_->writeLp(filename);
        }
        else if (fname.find(".mps") != string::npos)
        {
            model_->writeMps(filename);
        }
        else
        {
            // Default to LP format
            model_->writeLp(filename);
        }
    }

    void CLP_solver::solve(void)
    {
        if (prob_type_ == ProbType::MIP)
        {
            fprintf(stderr, "Warning: MIP detected but CLP is LP-only. Solving LP relaxation.\n");
        }
        
        solve_LP();
    }

    void CLP_solver::convert_row_bounds(char sense, double rhs, double &row_lower, double &row_upper) const
    {
        const double CLP_INFINITY = std::numeric_limits<double>::infinity();
        
        if (sense == 'L')  // Less than or equal
        {
            row_lower = -CLP_INFINITY;
            row_upper = rhs;
        }
        else if (sense == 'G')  // Greater than or equal
        {
            row_lower = rhs;
            row_upper = CLP_INFINITY;
        }
        else if (sense == 'E')  // Equal
        {
            row_lower = rhs;
            row_upper = rhs;
        }
        else
        {
            // Unknown sense, treat as equality
            row_lower = rhs;
            row_upper = rhs;
        }
    }
}
