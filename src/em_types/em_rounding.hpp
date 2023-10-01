#pragma once
#ifndef EM_ROUNDING_HPP_INCLUDED
#define EM_ROUNDING_HPP_INCLUDED
#include <rack.hpp>
#include "../misc.hpp"
#include "em_tuning.hpp"
using namespace ::rack;

namespace em_midi {

enum class RoundKind : uint8_t {
    Normal,
    Release,
    Y,
    InverseY
};

enum class RoundEqual : uint8_t {
    Disabled,
    Enabled,
    Equal   
};

std::string describeRoundKind(RoundKind kind);
std::string describeRoundKindShort(RoundKind kind);

struct Rounding
{
    uint8_t rate;
    bool initial;
    RoundKind kind;
    Tuning tuning;
    RoundEqual equal;

    Rounding()
    :   rate(0),
        initial(false),
        kind(RoundKind::Normal),
        tuning(Tuning::EqualTuning),
        equal(RoundEqual::Equal)
    {}

    void clear()
    {
        rate = 0;
        initial = false;
        kind = RoundKind::Normal;
        tuning = Tuning::EqualTuning;
        equal = RoundEqual::Equal;
    }
};

}
#endif