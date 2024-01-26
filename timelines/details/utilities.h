#pragma once

#include <algorithm>
#include <cmath>

namespace util
{
const double pi = std::acos(-1);

template<typename Type>
Type
limit(Type min, Type max, Type value)
{
    return std::min(max, std::max(min, value));
}

size_t length(const char* cstr);
}
