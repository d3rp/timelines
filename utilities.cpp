#include "utilities.h"

//template <typename ValueType>
//ValueType
//limit(const ValueType min, const ValueType max, const ValueType value)
//{
//    return std::min(max, std::max(min, value));
//}

size_t
length(const char* cstr)
{
    size_t _length = 0;
    char c = 'a';
    for (; c != 0; ++_length)
        c = cstr[_length];

    return _length;
}
