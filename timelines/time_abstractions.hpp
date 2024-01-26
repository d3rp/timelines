#pragma once

#include <cassert>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <limits>
#include <memory>
#include <random>
#include <set>
#include <string>

#include <SDL.h>

// #include <emscripten.h>

// #define SDL_MAIN_HANDLED

#include "calendars.hpp"
#include "details/utilities.h"
#include "entities.hpp"
#include "globals.hpp"
#include "time_abstractions.hpp"


namespace tl
{

struct YearRange
{
    int start = 0;
    int end = 0;
};

static YearRange new_relative_year_range(const Sint32 xrel,
                        const YearRange* year_range,
                        const int_pixels_t relative_max = screen_w)
{
    float scale = (float)(year_range->end - year_range->start) / relative_max;
    int scaled_x = xrel * scale;
    return { year_range->start+ scaled_x, year_range->end + scaled_x };
}

static YearRange new_scaled_year_range(const Sint32 value,
                      const YearRange* year_range,
                      const int_pixels_t mid_x = (screen_w / 2),
                      const int_pixels_t relative_max = screen_w)
{
    const float mid_point = 1.0f - ((float)mid_x / relative_max);
    constexpr float scale = 1e-2f;
    const float timescale = 1.0 - util::limit(-0.9f, 0.9f, scale * (float)value);
    const float start = year_range->start;
    const float mid = (year_range->start + year_range->end) * mid_point;
    const float end = year_range->end;

    const int newStart = year_limits((int)(((start - mid) * timescale) + mid));
    const int newEnd = year_limits((int)(((end - mid) * timescale) + mid));
    return { newStart, newEnd };
}
}