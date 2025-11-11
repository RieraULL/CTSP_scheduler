#include "sync_mapping.hpp"

namespace SYNC_LIB
{
    pair_map::pair_map(const size_t n_items) : map_(n_items, n_items)
    {
        map_.init(EMPTY_VAR);
    }    

    pair_map::pair_map(void) {}

    pair_map::~pair_map(void) {}

    void pair_map::set(const vector<triplet> &arcs)
    {
        int index{0};

        for(const triplet &t : arcs)
        {
            const int i = t.i_;
            const int j = t.j_;

            map_(i + 1, j + 1) = index;

            index++;
        }

        //map_.write_raw(cout);
        //cout << endl;
    }
}
