#pragma once

#include <stdint.h>

using int_pixels_t = uint16_t;
constexpr int_pixels_t screen_w = 1440;
constexpr int_pixels_t screen_h = 680;

// Used for indices of years
using int_index_t = uint32_t;
using int_year_t = int;

static const int_index_t MAX_BINS = 32768;
//constexpr int_index_t BINS_OFFSET = 32768 - 3000;
constexpr int_index_t BINS_SPLIT = 32768 - 3000;;
