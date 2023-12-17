#pragma once
#ifndef EM_PRIORITY_HPP_INCLUDED
#define EM_PRIORITY_HPP_INCLUDED
#include <rack.hpp>
#include "../misc.hpp"
using namespace ::rack;
namespace eagan_matrix {

enum class NotePriorityType: uint8_t {
    LRU,
    LRR,
    LCN,
    HI1,
    HI2,
    HI3,
    HI4
};

const char * NotePriorityName(NotePriorityType pri);

struct NotePriority
{
    uint8_t raw;

    NotePriority() : raw(0) { }
    explicit NotePriority(uint8_t b) : raw(b) { }

    static NotePriority from_raw(uint8_t b) { return NotePriority(b); }
    uint8_t to_raw() const { return raw; }
    void set_raw(uint8_t b) { raw = b; }

    NotePriorityType priority() const { return static_cast<NotePriorityType>((raw & 0x1Cu) >> 2u); }
    void set_priority(NotePriorityType pri) { raw = (raw & ~0x1Cu) | ((U8(pri) << 2u) & 0x1Cu); }

    bool increased_computation() const { return raw & 1; }
    void set_increased_computation(bool comp) { raw = (raw & ~1) | comp; }

    bool octave_toggle() const { return raw & 0x60; }
    void set_octave_toggle(bool toggle) { raw =  (raw & ~0x60u) | (toggle * 0x60); }

};

}
#endif