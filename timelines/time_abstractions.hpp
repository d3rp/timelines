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

//#include <emscripten.h>

//#define SDL_MAIN_HANDLED

#include "calendars.hpp"
#include "details/utilities.h"
#include "entities.hpp"
#include "globals.hpp"
#include "time_abstractions.hpp"

struct YearRange
{
  int start = 0;
  int end = 0;

  static YearRange new_relative_year_range(Sint32 xrel, YearRange* yrRange, int_pixels_t relativeMax = screen_w)
  {
    return newRelativeYearRange(xrel, yrRange->start, yrRange->end, relativeMax);
  }

  static YearRange
  newRelativeYearRange(Sint32 xrel, int yearStart, int yearEnd, int_pixels_t relativeMax = screen_w)
  {
    float xScale = (float)(yearEnd - yearStart) / relativeMax;
    int scaledRelativeX = xrel * xScale;
    return { yearStart + scaledRelativeX, yearEnd + scaledRelativeX };
  }

  static YearRange new_scaled_year_range(Sint32 value,
                     YearRange* yrRange,
                     int_pixels_t midX = (screen_w / 2),
                     int_pixels_t relativeMax = screen_w)
  {
    return newScaledYearRange(value, yrRange->start, yrRange->end, midX, relativeMax);
  }

  static YearRange
  newScaledYearRange(Sint32 value,
                     int yearStart,
                     int yearEnd,
                     int midX = (screen_w / 2),
                     int_pixels_t relativeMax = screen_w)
  {
    float midPoint = 1.0f - ((float)midX / relativeMax);
    constexpr float scaleCoeff = 1e-2f;
    const float timescale = 1.0 - limit(-0.9f, 0.9f, scaleCoeff * (float)value);
    const float start = yearStart;
    const float mid = (yearStart + yearEnd) * midPoint;
    const float end = yearEnd;

    const int newStart = year_limits((int)(((start - mid) * timescale) + mid));
    const int newEnd = year_limits((int)(((end - mid) * timescale) + mid));
    return { newStart, newEnd };
  }
};
