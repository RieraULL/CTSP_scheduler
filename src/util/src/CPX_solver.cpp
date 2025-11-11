#include "CPX_solver.hpp"

namespace GOMA
{
    CPX_solver::CPX_solver(const model_description &model, const double tol) : LP_solver(model, tol),
                                                                               env_(NULL),
                                                                               problem_(NULL)
    {
        init_solver();
        build_model(model);
        //CPXwriteprob(env_, problem_, "debug_primal_dual.lp", NULL);
        
    }

    CPX_solver::~CPX_solver()
    {
        clear();
    }

    void CPX_solver::clear(void)
    {
        int status;

        if (problem_ != NULL)
        {

            status = CPXfreeprob(env_, &problem_);
            CPXcloseCPLEX(&env_);

            if (status)
            {
                fprintf(stderr, "CPXfreeprob failed, error code %d.\n", status);
            }

            problem_ = NULL;
        }

        if (env_ != NULL)
        {
            status = CPXcloseCPLEX(&env_);

            if (status)
            {

                char errmsg[1024];
                fprintf(stderr, "Could not close CPLEX environment.\n");
                CPXgeterrorstring(env_, status, errmsg);
                fprintf(stderr, "%s", errmsg);
            }

            env_ = NULL;
        }
    }

    int CPX_solver::get_nz(void) const
    {
        return CPXgetnumnz(env_, problem_);
    }

    double CPX_solver::get_obj(void) const
    {
        if (lpstat_ == CPX_STAT_OPTIMAL)
        {
            double obj;
            int status = CPXgetobjval(env_, problem_, &obj);
            if (status)
            {
                fprintf(stderr, "Failed to obtain objective value.\n");
                CPXwriteprob(env_, problem_, "errorObj.lp", NULL);
                exit(1);
            }

            return obj;
        }
        else if (lpstat_ == CPX_STAT_UNBOUNDED)
        {
            return -1e20;
        }
        else
        {
            return 1e20;
        }

        
    }

    void CPX_solver::disable_prep_linear(void)
    {
        int status = CPXsetintparam(env_, CPXPARAM_Preprocessing_Linear, CPX_OFF);

        if (status)
        {
            fprintf(stderr, "Failed to disable presolve.\n");
            exit(1);
        }
    }

    void CPX_solver::get_vars(double *alpha) const
    {
        int status = CPXgetx(env_, problem_, alpha, 0, n_col_ - 1);

        if (status)
        {
            fprintf(stderr, "Failed to obtain solution.\n");
            CPXwriteprob(env_, problem_, "error.lp", NULL);
            exit(1);
        }
    }

    void CPX_solver::get_dual_vars(double *alpha) const
    {
        int status = CPXgetpi(env_, problem_, alpha, 0, n_row_ - 1);

        if (status)
        {
            fprintf(stderr, "Failed to obtain dual solution.\n");
            exit(1);
        }
    }

    void CPX_solver::set_obj(double *obj_coef, int *obj_inx, int sz)
    {
        int status = CPXchgobj(env_, problem_, sz, obj_inx, obj_coef);

        if (status)
        {
            fprintf(stderr, "Failed to change objective.\n");
            exit(1);
        }
    }

    void CPX_solver::set_rhs(int cnt, const int *rhs_inx, const double *rhs_val)
    {
        int status = CPXchgrhs(env_, problem_, cnt, rhs_inx, rhs_val);

        if (status)
        {
            fprintf(stderr, "Failed to set rhs.\n");
            exit(1);
        }
    }

    void CPX_solver::set_rhs(const int row, const double val)
    {
        int status = CPXchgrhs(env_, problem_, 1, &row, &val);

        if (status)
        {
            fprintf(stderr, "Failed to set rhs.\n");
            exit(1);
        }
    }

    void CPX_solver::set_bdn(double *obj_coef, char *sense, int *obj_inx, int sz)
    {
        //CPXwriteprob(env_, problem_, "before_bounds.lp", NULL);
        int status = CPXchgbds(env_, problem_, sz, obj_inx, sense, obj_coef);
        //CPXwriteprob(env_, problem_, "after_bounds.lp", NULL);

        if (status)
        {
            fprintf(stderr, "Failed to change bounds.\n");
            exit(1);
        }

        //CPXwriteprob(env_, problem_, "bd_debug.lp", NULL);
    }

    void CPX_solver::del_rows(int begin, int end)
    {
        int status = CPXdelrows(env_, problem_, begin, end);

        if (status)
        {
            fprintf(stderr, "Failed to delete rows.\n");
            exit(1);
        }
    }

    void CPX_solver::add_cut(int nzcnt, double const *rhs, char const *sense, int const *rmatbeg, int const *rmatind, double const *rmatval, char **rowname)
    {
        int status = CPXaddrows(env_, problem_, 0, 1, nzcnt, rhs, sense, rmatbeg, rmatind, rmatval, NULL, rowname);

        if (status)
        {
            fprintf(stderr, "Failed to add user cuts.\n");
            exit(1);
        }
    }

    void CPX_solver::set_coef(int cnt, const int *row_inx, const int *col_inx, const double *coef_val)
    {
        //CPXwriteprob(env_, problem_, "before_coef.lp", NULL);
        int status = CPXchgcoeflist(env_, problem_, cnt, row_inx, col_inx, coef_val);  
        //CPXwriteprob(env_, problem_, "after_coef.lp", NULL);

        if (status)
        {
            fprintf(stderr, "Failed to change coefficients.\n");
            exit(1);
        }
    }

    int CPX_solver::get_n_rows(void) const
    {
        return CPXgetnumrows(env_, problem_);
    }

    void CPX_solver::init_solver(void)
    {
        int status;

        env_ = CPXopenCPLEX(&status);

        if (env_ == NULL)
        {
            fprintf(stderr, "Could not open CPLEX environment.\n");
            exit(1);
        }

        // status = CPXsetintparam(env_, CPX_PARAM_SCRIND, CPX_OFF);
        // status = CPXsetintparam(env_, CPXPARAM_Threads, 1);

        // status = CPXsetintparam(env_, CPX_PARAM_LPMETHOD, CPX_ALG_DUAL);

        if (status)
        {
            fprintf(stderr,
                    "Failure to turn off screen indicator, error %d.\n", status);
            exit(1);
        }

        problem_ = CPXcreateprob(env_, &status, "dual_checker");

        if (problem_ == NULL)
        {
            fprintf(stderr, "Failed to create LP.\n");
            exit(1);
        }
    }

    void CPX_solver::build_model(const CPX_model_structure &model)
    {

        // cout << "Creando modelo CBC" << endl;
        int status = CPXcopylp(env_, problem_, model.ncol(), model.nrow(), CPX_MAX, model.get_obj(), model.get_rhs(),
                               model.get_sense(), model.get_matbeg(), model.get_matcnt(), model.get_matind(), model.get_matval(),
                               model.get_lb(), model.get_ub(), NULL);

        int c_inx[model.ncol()];

        for (int i{0}; i < model.ncol(); ++i)
            c_inx[i] = i;

        if (prob_type_ == ProbType::MIP)
        {
            status = CPXchgctype(env_, problem_, model.ncol(), c_inx, model.get_ctype());

            if (status)
            {
                fprintf(stderr, "Failed to change column type data.\n");
                exit(1);
            }

            status = CPXchgprobtype(env_, problem_, CPXPROB_MILP);
        }

        status = CPXsetintparam(env_, CPX_PARAM_SCRIND, CPX_OFF);

        status = CPXchgobjsen(env_, problem_, model.get_obj_sense());

        // cout << "Modelo CBC creado" << endl;

        if (status)
        {
            fprintf(stderr, "Failed to copy problem data.\n");
            exit(1);
        }

        const int max_sz = model.nrow() > model.ncol() ? model.nrow() : model.ncol();
        int inx[max_sz];

        for (int i{0}; i < max_sz; ++i)
            inx[i] = i;

        {
            char **colname = model.get_colname();

            status = CPXchgcolname(env_, problem_, model.ncol(), inx, colname);

            if (status)
            {
                fprintf(stderr, "Failed to rename columns.\n");
                exit(1);
            }
        }

        {
            char **rowname = model.get_rowname();

            status = CPXchgrowname(env_, problem_, model.nrow(), inx, rowname);

            if (status)
            {
                fprintf(stderr, "Failed to rename rows.\n");
                exit(1);
            }
        }

        // CPXwriteprob(env_, problem_, "model_debug.lp", NULL);
    }

    void CPX_solver::solve_LP(void)
    {
        int status = CPXprimopt(env_, problem_);

        if (status)
        {
            fprintf(stderr, "Failed to optimize LP. Code %d\n", status);
            exit(1);
        }

        lpstat_ = CPXgetstat(env_, problem_);

        // double obj;

        // DEBUG
        // CPXgetobjval(env_, problem_, &obj);

        // cout << "OBJETIVO SEP: " << obj << endl;

        /// CPXwriteprob(env_, problem_, "dual1.lp", NULL);

        // cout << "CBC STATUS: " << lpstat_ << endl;
    }

    void CPX_solver::solve_MIP(void)
    {
        int status = CPXmipopt(env_, problem_);

        if (status)
        {
            fprintf(stderr, "Failed to optimize LP. Code %d\n", status);
            // exit(1);
        }

        lpstat_ = CPXgetstat(env_, problem_);

        // double obj;

        // DEBUG
        // CPXgetobjval(env_, problem_, &obj);

        // cout << "OBJETIVO SEP: " << obj << endl;

        // CPXwriteprob(env_, problem_, "dual1.lp", NULL);

        // cout << "CBC STATUS: " << lpstat_ << endl;
    }

    void CPX_solver::write_model(const char *filename) const
    {
        CPXwriteprob(env_, problem_, filename, NULL);
    }

    void CPX_solver::solve(void)
    {
        // DEBUG
        //CPXwriteprob(env_, problem_, "infeas_debug.lp", NULL);
        //exit(1);

        //if (prob_type_ == ProbType::MIP)
        //    solve_MIP();
        //else
            solve_LP();
    }

}