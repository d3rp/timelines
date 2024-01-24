#pragma once

#include <array>
#include <cassert>

#include "details/utilities.h"
#include "entities.hpp"
#include "globals.hpp"

int_index_t year_to_index(int_year_t year);
int_year_t index_to_year(int_index_t index);
int_year_t year_limits(int_year_t year);

struct TimeInterface
{
};

struct JulianCalendar : public TimeInterface
{
};

class Years
{
    Years() = default;

  public:
    using EntityPtr = std::unique_ptr<Entity>;

    ~Years();

    static Years& instance();

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
