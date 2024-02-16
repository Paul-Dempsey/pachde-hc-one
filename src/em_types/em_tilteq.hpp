// Copyright (C) Paul Chase Dempsey
#pragma once
#ifndef EM_TILTEQ_HPP_INCLUDED
#define EM_TILTEQ_HPP_INCLUDED
#include <rack.hpp>
#include "../misc.hpp"

using namespace ::rack;

namespace eagan_matrix {

struct TiltEq
{
    uint8_t tilt;       // EMCC_TiltEq          = 83;
    uint8_t frequency;  // EMCC_TiltEqFrequency = 84; // 0..127 = 120Hz..15kHz	
    uint8_t mix;        // EMCC_TiltEqMix       = 85; // 0=dry .. 127=wet

    TiltEq()
    :   tilt(64),
        frequency(64),
        mix(0)
    {}

    void clear()
    {
        tilt = 64;
        frequency = 64;
        mix = 0;
    }
};

}
#endif