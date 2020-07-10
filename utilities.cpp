#include "utilities.h"

size_t
length(const char* cstr)
{
    size_t _length = 0;
    char c = 'a';
    for (; c != 0; ++_length)
        c = cstr[_length];

    return _length;
}
