#include "calendars.hpp"

int_index_t
year_to_index(int_year_t year)
{
    const int_index_t index = limit<int_index_t>(0, MAX_BINS - 1, BINS_SPLIT + year);
    assert(index >= 0 && index < MAX_BINS);
    return index;
}

int_year_t
index_to_year(int_index_t index)
{
    const int_year_t year = index - BINS_SPLIT;
    return year;
}

int_year_t
year_limits(int_year_t year)
{
    return limit<int_year_t>(-BINS_SPLIT, MAX_BINS - BINS_SPLIT, year);
}
////////////////////////////////////////////////////////////////////////////////


Years::~Years() { std::cout << "DTOR " << __PRETTY_FUNCTION__ << "\n"; }

Years&
Years::instance()
{
    static Years instance;
    return instance;
}


void
Years::insert(Entity* e)
{
    for (auto i = year_to_index(e->start_year); i < year_to_index(e->end_year); ++i)
        ++year_bins[i];
}

//std::set<Entity*>
//Years::getEntitiesInInterval(int start, int end)
//{
//    std::set<Entity*> v;
//    for (auto yi = start; yi < end; ++yi)
//    {
//        auto year = bins[year_to_index(yi)];
//        for (auto* e : year)
//            v.insert(e);
//    }
//
//    return v;
//}

void
Years::clear()
{
    std::fill(year_bins.begin(), year_bins.end(), 0);
//    for (auto& y : year_bins)
//        y = 0;
}
