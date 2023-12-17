#pragma once
#ifndef EM_COMPRESSOR_HPP_INCLUDED
#define EM_COMPRESSOR_HPP_INCLUDED
#include <rack.hpp>
#include "../misc.hpp"

using namespace ::rack;

namespace eagan_matrix {

struct Compressor
{
    uint8_t threshold;  // EMCC_CompressorThreshold  = 90;
    uint8_t attack;     // EMCC_CompressorAttack     = 91;
    uint8_t ratio;      // EMCC_CompressorRatio      = 92;
    uint8_t mix;        // EMCC_CompressorMix        = 93;

    Compressor()
    :   threshold(127),
        attack(64),
        ratio(64),
        mix(0)
    {}

    void clear()
    {
        threshold = 127;
        attack = 64;
        ratio = 64;
        mix = 0;
    }
};

}
#endif