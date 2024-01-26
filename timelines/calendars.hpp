#pragma once

#include <array>
#include <cassert>

#include "details/utilities.h"
#include "entities.hpp"
#include "globals.hpp"

namespace tl
{
tl::idx_t year_to_index(tl::year_t year);
tl::year_t index_to_year(tl::idx_t index);
tl::year_t year_limits(tl::year_t year);

struct TimeInterface
{
};

struct JulianCalendar : public TimeInterface
{
};

struct Years
{
    Years() = default;
    ~Years();

    std::array<size_t, tl::MAX_BINS> year_bins{ 0 };

    void insert(Entity* e);
    void clear();
};
}