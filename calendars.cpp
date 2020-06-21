#include "calendars.h"

int
yearToIndex(int year)
{
    const int index = limit(0, MAX_BINS - 1, BINS_SPLIT + year);
    assert(index >= 0 && index < MAX_BINS);
    return index;
}
int
indexToYear(int index)
{
    const int year = index - BINS_SPLIT;
    return year;
}

////////////////////////////////////////////////////////////////////////////////

Years* Years::instance = nullptr;

Years::Years() {}

Years::~Years() { std::cout << "DTOR " << __PRETTY_FUNCTION__ << "\n"; }

Years*
Years::getInstance()
{
    if (instance == nullptr)
        instance = new Years();

    return instance;
}


void
Years::insert(Entity* e)
{
    for (auto i = yearToIndex(e->startYear); i < yearToIndex(e->endYear); ++i)
        ++year_bins[i];
}

std::set<Entity*>
Years::getEntitiesInInterval(int start, int end)
{
    std::set<Entity*> v;
    for (auto yi = start; yi < end; ++yi)
    {
        auto year = bins[yearToIndex(yi)];
        for (auto* e : year)
            v.insert(e);
    }

    return v;
}
