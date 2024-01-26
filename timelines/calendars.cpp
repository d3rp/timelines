#include <iostream>

#include "calendars.hpp"

namespace tl
{
tl::idx_t
year_to_index(tl::year_t year)
{
    const auto index = util::limit<tl::idx_t>(0, tl::MAX_BINS - 1, tl::BINS_SPLIT + year);
    assert(index >= 0 && index < tl::MAX_BINS);
    return index;
}

tl::year_t
index_to_year(tl::idx_t index)
{
    const tl::year_t year = index - tl::BINS_SPLIT;
    return year;
}

tl::year_t
year_limits(tl::year_t year)
{
    return util::limit<tl::year_t>(-tl::BINS_SPLIT, tl::MAX_BINS - tl::BINS_SPLIT, year);
}
////////////////////////////////////////////////////////////////////////////////

Years::~Years() { std::cout << "DTOR " << __PRETTY_FUNCTION__ << "\n"; }

void
Years::insert(Entity* e)
{
    for (auto i = year_to_index(e->start_year); i < year_to_index(e->end_year); ++i)
        ++year_bins[i];
}

void
Years::clear()
{
    std::fill(year_bins.begin(), year_bins.end(), 0);
}
}