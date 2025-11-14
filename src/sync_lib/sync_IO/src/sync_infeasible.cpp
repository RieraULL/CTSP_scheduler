#include "sync_infeasible.hpp"

#include <iomanip>
#include <cmath>

namespace SYNC_LIB
{
    /**
     * Default constructor
     */
    sync_infeasible::sync_infeasible(const vector<double> &x,const sync_model_a_builder &builder)
        : operation_names_(builder.get_operation_names()),
          routing_arc_names_(builder.get_routing_arc_names()),
          sync_arc_names_(builder.get_sync_arc_names()),
          routing_arcs_(builder.get_routing_arcs()),
          sync_arcs_(builder.get_sync_arcs()),
          routing_arc_times_(builder.get_routing_arc_times()),
          x_(x)
    {
    }

    /**
     * Destructor
     */
    sync_infeasible::~sync_infeasible(void)
    {
    }

    ostream &sync_infeasible::write_path_(ostream &os, const vector<int> &cycle) const
    {
        const size_t n_routing_arcs{routing_arcs_.size()};

        for (int inx : cycle)
        {
            if (inx < (int)n_routing_arcs)
            {
                os << routing_arc_names_[inx] << " ";
            }
            else
            {
                os << sync_arc_names_[inx - (int)n_routing_arcs] << " ";
            }
        }

        os << endl;

        return os;
    }

    ostream &sync_infeasible::write_infeasible_paths(ostream &os) const
    {
        os << "Infeasible paths detected in the solution:" << endl;

        for (const auto &cycle : violated_cycles_)
        {
            write_path_(os, cycle);
            os << endl;
        }

        return os;
    }

    ostream &sync_infeasible::write_primal_dual_graph(ostream &os) const
    {
        write_(os, x_, alpha_, beta_);

        return os;
    }

    ostream &sync_infeasible::write_(ostream &os, const vector<double> &x, const vector<double> &alpha_v, const vector<double> &beta_v) const
    {
        os << endl;
        os << "digraph G { \n\nrankdir=LR; \noverlap=false \n " << endl;

        // const vector<double> &routing_arc_costs{input_.get_routing_arc_costs()};
        // const vector<double> &sync_arc_costs{input_.get_sync_arc_costs()};
        // const pair_map &sync_arc_map{input_.get_sync_arcs_pair_map()};

        /////////////////////////
        {
            const size_t n_routing_arcs{routing_arc_times_.size()};

            for (size_t i{0}; i < n_routing_arcs; i++)
            {
                const double alpha_val{alpha_v[i]};
                const double x_val{x[i]};

                if ((fabs(alpha_val) > tol_) && (fabs(x_val - alpha_val) < tol_))
                {
                    const triplet &arc = routing_arcs_[i];

                    const int s{arc.i_};
                    const int t{arc.j_};

                    os << setw(5) << operation_names_[s] << " -> " << setw(5) << operation_names_[t];
                    os << " [ fontsize=\"10pt\" ";
                    os << ", label = \" ";
                    os << setw(4) << fixed << setprecision(2) << alpha_val;
                    // os << "<" << fixed << setprecision(2) << x_val << ">";
                    os << "\"";
                    os << ", color =\"blue\" ";

                    if (alpha_val > 0.9)
                    {
                        os << ", style =\"solid\" ";
                    }
                    else if (alpha_val > 0.4)
                    {
                        os << ", style =\"dashed\" ";
                    }
                    else
                    {
                        os << ", style =\"dotted\" ";
                    }

                    os << " ] ";
                    os << endl;
                }
                else
                {
                    if (x_val > tol_)
                    {
                        const triplet &arc = routing_arcs_[i];

                        const int s{arc.i_};
                        const int t{arc.j_};

                        os << setw(5) << operation_names_[s] << " -> " << setw(5) << operation_names_[t];
                        os << " [ fontsize=\"10pt\", label = \" ";
                        os << setw(4) << fixed << setprecision(2) << x_val;
                        if (alpha_val > tol_)
                        {
                            os << " / " << setw(4) << fixed << setprecision(2) << alpha_val;
                            os << "\", color =\"green\"";
                        }
                        else
                            os << "\", color =\"gray\"";

                        if (x_val > 0.9)
                        {
                            os << ", style =\"solid\" ";
                        }
                        else if (x_val > 0.4)
                        {
                            os << ", style =\"dashed\" ";
                        }
                        else
                        {
                            os << ", style =\"dotted\" ";
                        }

                        os << " ] ";
                        os << endl;
                    }
                    else
                    {
                        if (fabs(alpha_val) > tol_)
                        {
                            const triplet &arc = routing_arcs_[i];

                            const int s{arc.i_};
                            const int t{arc.j_};

                            os << setw(5) << operation_names_[s] << " -> " << setw(5) << operation_names_[t];
                            os << " [ fontsize=\"10pt\", label = \" ";

                            os << setw(4) << fixed << setprecision(2) << alpha_val;
                            // os << "<" << fixed << setprecision(0) << routing_arc_costs[i] << ">";
                            os << "\", color =\"blue\" ]";
                            os << endl;
                        }
                    }
                }
            }
        }

        /////////////////////////

        const size_t n_sync_arcs{sync_arc_names_.size()};

        for (size_t i{0}; i < n_sync_arcs; i++)
        {
            const double val{beta_v[i]};

            if (val > tol_)
            {
                const triplet &arc = sync_arcs_[i];

                const int s{arc.i_};
                const int t{arc.j_};

                const pair<int, int> reverse_arc{pair<int, int>(t, s)};
                // const int reverse_arc_inx{sync_arc_map.at(reverse_arc)};

                os << setw(5) << operation_names_[s] << " -> " << setw(5) << operation_names_[t];
                os << " [ fontsize=\"10pt\" ";
                // os << ", label = \" ";
                // os << setw(4) << fixed << setprecision(2) << val;
                // os << " <" << fixed << setprecision(0) << sync_arc_costs[i] << ">";
                // os << "<" << fixed << setprecision(0) << sync_arc_costs[reverse_arc_inx] << "> \"";

                if (val > 0.9)
                {
                    os << ", style =\"solid\" ";
                }
                else if (val > 0.4)
                {
                    os << ", style =\"dashed\" ";
                }
                else
                {
                    os << ", style =\"dotted\" ";
                }

                os << ", color =\"red\" ]";
                os << endl;
            }
        }

        os << "}" << endl;

        return os;
    }
}