#pragma once

#include <array>
#include <cassert>

#include "globals.hpp"
#include "details/utilities.h"
#include "entities.hpp"

int_index_t yearToIndex(int_year_t year);
int_year_t indexToYear(int_index_t index);
int_year_t yearLimits(int_year_t year);


struct TimeInterface
{

};
struct JulianCalendar : public TimeInterface
{};
class Years
{
    Years();

    static Years* instance;

public:
    ~Years();
    static Years* getInstance();

    typedef std::unique_ptr<Entity> EntityPtr;

//    std::array<std::vector<Entity*>, MAX_BINS> bins{};
    std::array<size_t, MAX_BINS> year_bins{ 0 };

    void insert(Entity* e);
    void clear();
    struct IdEquals
    {
        const int id;
        IdEquals(Entity* e)
            : id(e->id)
        {
        }
        bool operator()(Entity* x) const { return x->id == id; }
    };

    std::set<Entity*> getEntitiesInInterval(int start, int end);
};
