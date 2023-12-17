#pragma once
#ifndef EM_POLYPHONY_HPP_INCLUDED
#define EM_POLYPHONY_HPP_INCLUDED
#include <rack.hpp>
#include "../misc.hpp"
using namespace ::rack;
namespace eagan_matrix {

struct Polyphony
{
    uint8_t raw;

    Polyphony() : raw(1) { }
    explicit Polyphony(uint8_t b) : raw(b) { }

    static Polyphony from_raw(uint8_t b) { return Polyphony(b); }
    uint8_t to_raw() const { return raw; }
    void set_raw(uint8_t b) { raw = b; }

    uint8_t polyphony() const { return raw & 0x1f; }
    void set_polyphony(uint8_t voices) { raw = voices | (raw & 0x40); }

    bool expanded_polyphony() const { return raw & 0x40; }
    void set_expanded_polyphony(bool expanded) { raw = (raw & ~0x40) | (expanded * 0x40); }
};

}
#endif