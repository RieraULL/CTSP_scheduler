#include "sync_solution.hpp"

#include "json_format_io.hpp"

#include <iostream>
#include <fstream>
#include <iomanip>

namespace SYNC_LIB
{
    sync_solution::sync_solution(const string &instance_name, const vector<vector<int>> &routes) : instance_name_(instance_name), routes_(routes)
    {
    }

    sync_solution::sync_solution(const string &instance_file)
    {
        if (instance_file != "")
        {
            ifstream is(instance_file);
            if (!is.is_open())
            {
                cerr << "Error opening file: " << instance_file << endl;
            }
            else
            {
                read(is);
                is.close();
            }
        }
    }

    sync_solution::sync_solution(void)
    {
    }

    sync_solution::~sync_solution(void)
    {
    }

    void sync_solution::init(void)
    {
        routes_.clear();
    }

    bool sync_solution::empty(void) const
    {
        return routes_.empty();
    }

    void sync_solution::set(const string &instance_name, const vector<vector<int>> &routes)
    {
        instance_name_ = instance_name;
        routes_ = routes;
    }

    void sync_solution::read(istream &is)
    {
        is >> instance_name_;

        if (instance_name_.empty())
        {
            cerr << "Error: Feasible solution file is empty." << endl;
            return;
        }

        size_t n_routes;
        is >> n_routes;

        routes_.resize(n_routes);

        for (size_t i = 0; i < n_routes; ++i)
        {
            size_t n_nodes;
            is >> n_nodes;

            routes_[i].resize(n_nodes);

            size_t node;

            for (size_t j = 0; j < n_nodes; ++j)
            {
                is >> node;
                routes_[i][j] = node - 1;
            }
        }
    }

    void sync_solution::write(ostream &os) const
    {
        os << setw(15) << left << instance_name_ << endl;
        os << endl;
        os << " " << setw(4) << right << routes_.size() << endl;
        os << endl;

        for (size_t i = 0; i < routes_.size(); ++i)
        {
            os << " " << setw(4) << right << routes_[i].size() << endl;
            os << endl;

            if (routes_[i].size() > 0)
            {

                bool depot_inserted{routes_[i][0] == 1};

                if (!depot_inserted)
                {
                    os << " " << setw(4) << right << 1 << " ";
                }

                for (size_t j = 0; j < routes_[i].size(); ++j)
                {
                    os << setw(4) << right << routes_[i][j] << " ";
                }

                if (!depot_inserted)
                {
                    os << " " << setw(4) << right << 1 << " ";
                }
            }
            os << endl;
            os << endl;
        }

        os << endl;
    }

    void sync_solution::read_json(istream &is)
    {
        json_format_io json_io;

        json_io.read_sol(is, instance_name_, routes_);
    }

    void sync_solution::write_header(ostream &os) const
    {
        os << "{" << endl;
        os << "  \"instance_name\": \"" << instance_name_ << "\"," << endl;
        // os << "  \"obj_value\": " << obj_value << "," << endl;
        //os << "  \"routes\": " << endl;
    }

    void sync_solution::write_routes(ostream &os) const
    {
        json_format_io json_io;

        json_io.write_vector_of_integer_vectors(os, routes_);
    }

    void sync_solution::write_end(ostream &os) const
    {
        os << "}" << endl;
    }

    void sync_solution::write_json(ostream &os) const
    {
        json_format_io json_io;

        json_io.write_sol(os, instance_name_, routes_);
    }

    void sync_solution::get_arcs(const size_t depot, vector<pair<int, int>> &arcs) const
    {
        arcs.clear();

        const vector<int> &route{routes_[depot]};

        for (size_t i = 0; i < route.size() - 1; ++i)
        {
            arcs.push_back(make_pair(route[i], route[i + 1]));
        }
    }
}