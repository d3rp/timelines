#pragma once

#include <cmath>
#include <algorithm>

const double pi = std::acos(-1);

template <typename Type>
Type
limit(Type min, Type max, Type value)
{
    return std::min(max, std::max(min, value));
}

size_t
length(const char* cstr);

