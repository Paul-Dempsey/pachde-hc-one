#pragma once
#ifndef STRING_POOL_HPP_INCLUDED
#define STRING_POOL_HPP_INCLUDED

constexpr const size_t STRING_POOL_SIZE = 16*1024;

template <const int max_string_length, const size_t pool_size>
class StringPool
{
    char data[pool_size];
    char * next = data;
    char * end = data;
    char * const lim = data + pool_size;

public:
    size_t used() { return next - data; }
    size_t remaining() { return pool_size - used; }

    void reset() {
        end = next = data;
    }

    // truncates to max_string or limit of the pool capacity
    void build(char ch) {
        if ((end - next < max_string_length) && (end < lim-1)) {
            *end++ = ch;
        }
    }

    const char * finish() {
        if (end >= lim) return nullptr;

        const char * result = next;
        *end++ = 0;
        next = end;
        return result;
    }
};

#endif
