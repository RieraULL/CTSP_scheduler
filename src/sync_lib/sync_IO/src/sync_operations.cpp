#include "sync_operations.hpp"

#include <iomanip>

namespace SYNC_LIB
{
    ostream &sync_operation::write(ostream &os) const
    {
        os << setw(8) << left << name_ << " " << right;

        const int r_sz{(int)r_.size()};

        for (int i{0}; i < r_sz; i++)
        {
            os << setw(3) << r_[i] << " ";
        }
        return os;
    }

    ostream &sync_operation::short_write(ostream &os) const
    {
        os << name_;
        return os;
    }

    const string operation_arc::get_name(const vector<sync_operation> &operations) const
    {
        const string &from{operations[first.first.first].get_name()};
        const string &to{operations[first.first.second].get_name()};
        return "(" + from + "_" + to + ")";
    }

    void operation_arc_list::get_names(const vector<sync_operation> &operations, vector<string> &names) const
    {
        const int sz{(int)size()};

        for (int i{0}; i < sz; i++)
        {
            names.push_back(at(i).get_name(operations));
        }
    }

    ostream &operations_subset::write(const vector<sync_operation> &operations, ostream &os) const
    {
        os << setw(8) << left << name_ << ": " << right;

        const int sz{(int)size()};

        for (int i{0}; i < sz; i++)
        {
            operations[at(i)].write(os);
            os << " ";
        }

        cout << endl;

        // Writing arcs

        const int a_sz{(int)A_.size()};

        for (int i{0}; i < a_sz; i++)
        {
            const int from{A_[i].first.first.first};
            const int to{A_[i].first.first.second};

            os << "(";
            operations[from].short_write(os);
            os << ", ";
            operations[to].short_write(os);
            os << ") ";
        }

        os << endl;

        return os;
    }

    void operations_partition::get_subsets_maps(vector<int> &ss_maps) const
    {
        const int sz{(int)size()};

        for (int i{0}; i < sz; i++)
        {
            const vector<int> &ss{at(i)};

            const size_t subset_sz{ss.size()};

            for (size_t j{0}; j < subset_sz; j++)
            {
                const int op{ss.at(j)};
                ss_maps[op] = i;
            }
        }
    }

    ostream &operations_partition::write(const vector<sync_operation> &operations, ostream &os) const
    {
        os << " -- " << name_ << " -- " << endl;

        const int sz{(int)size()};

        for (int i{0}; i < sz; i++)
        {
            at(i).write(operations, os);
            os << endl;
        }

        // Writing arcs

        const int a_sz{(int)A_.size()};

        for (int i{0}; i < a_sz; i++)
        {
            const int from{A_[i].first.first.first};
            const int to{A_[i].first.first.second};

            os << "(";
            operations[from].short_write(os);
            os << ", ";
            operations[to].short_write(os);
            os << ") ";
        }

        os << endl;

        return os;
    }
}