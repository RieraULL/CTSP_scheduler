#include "sync_scheduling.hpp"

#include "json_format_io.hpp"

namespace SYNC_LIB
{
    sync_scheduling::sync_scheduling(void)
    {
    }

    sync_scheduling::sync_scheduling(const string &instance_name) : instance_name_(instance_name)
    {
    }

    sync_scheduling::~sync_scheduling(void)
    {
    }

    ostream &sync_scheduling::write_json(ostream &os) const
    {
        json_format_io json_io;
        json_io.write(os, instance_name_, *this);
        return os;
    }

    istream &sync_scheduling::read_json(istream &is)
    {
        json_format_io json_io;
        //json_io.read_sch(is, instance_name_, this->starting_times_); // read_sch
        return is;
    }
}