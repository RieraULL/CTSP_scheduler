#include "sync_tw.hpp"

#include "json_format_io.hpp"

namespace SYNC_LIB
{
    tw_info::tw_info(void) : pair<double, double>(0.0, 0.0)
    {
    }

    tw_info::tw_info(double lower_bound, double upper_bound) : pair<double, double>(lower_bound, upper_bound)
    {
    }

    tw_info::~tw_info(void)
    {
    }

    double &tw_info::lower_bound(void)
    {
        return pair<double, double>::first;
    }

    double &tw_info::upper_bound(void)
    {
        return pair<double, double>::second;
    }

    const double &tw_info::lower_bound(void) const
    {
        return pair<double, double>::first;
    }

    const double &tw_info::upper_bound(void) const
    {
        return pair<double, double>::second;
    }

    sync_time_windows::sync_time_windows(void) : vector<tw_info>()
    {
    }

    sync_time_windows::sync_time_windows(const string &instance_name, const vector<tw_info> &time_windows)
        : vector<tw_info>(time_windows), instance_name_(instance_name)
    {
    }

    sync_time_windows::~sync_time_windows(void)
    {
    }

    ostream &sync_time_windows::write_json(ostream &os) const
    {
        //os << "{ \"instance_name\": \"" << instance_name_ << "\", \"time_windows\": [";

        os  << "  \"time_windows\": " << endl;
        
        vector<pair<double, double>> tw_vector;
        for (const auto &tw : *this)
        {
            tw_vector.emplace_back(tw.lower_bound(), tw.upper_bound());
        }

        json_format_io json_io;
        json_io.write_vector_of_pairs(os, tw_vector);

        return os;
    }

    istream &sync_time_windows::read_json(istream &is)
    {
        string line;
        getline(is, line);
        // Simple JSON parsing (not robust)
        sscanf(line.c_str(), "{ \"instance_name\": \"%[^\"]\", \"time_windows\": [", &instance_name_[0]);
        tw_info tw;
        vector<pair<double, double>> tw_vector;
        json_format_io json_io;
        json_io.read_vector_of_pairs(is, tw_vector);
        this->clear();

        for (const auto &p : tw_vector)
        {
            tw_info tw_a(p.first, p.second);
            this->push_back(tw_a);
        }

        return is;
    }
}