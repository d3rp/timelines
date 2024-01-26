#pragma once

#include <cstdint>

namespace tl
{

using int_pixels_t = uint16_t;
constexpr int_pixels_t screen_w = 1440;
constexpr int_pixels_t screen_h = 680;

// Used for indices of years
using idx_t = uint32_t;
using year_t = int;

static const idx_t MAX_BINS = 32768;
// constexpr idx_t BINS_OFFSET = 32768 - 3000;
constexpr idx_t BINS_SPLIT = 32768 - 3000;

}