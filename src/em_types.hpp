#pragma once
#ifndef EM_TYPES_HPP_INCLUDED
#define EM_TYPES_HPP_INCLUDED
#include <rack.hpp>
#include "misc.hpp"
using namespace ::rack;

namespace pachde {


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
enum Tuning : uint8_t {
    EqualTuning = 0,
    OneTone = 1,
    FiftyTone = 50,
    JustC = 60,
    JustCs,
    JustD,
    JustEb,
    JustF,
    JustFs,
    JustG,
    JustAb,
    JustA,
    JustBb,
    JustB,
    UserTuning1 = 80,
    UserTuning2,
    UserTuning3,
    UserTuning4,
    UserTuning5,
    UserTuning6,
    UserTuning7,
    UserTuning8,
    UserTuningLast = UserTuning8
};

inline Tuning nTone(int nth) {
    assert(in_range(nth, 1, 50));
    return static_cast<Tuning>((static_cast<int>(OneTone) + nth - 1));
}

// Packed tuning is for when you need a linear sequence of values,
// such as putting tuning on a knob.
enum PackedTuning: uint8_t {
    ptEqual,
    ptOneTone,
    ptFiftyTone = 50,
    ptJustC,
    ptJustCs,
    ptJustD,
    ptJustEb,
    ptJustF,
    ptJustFs,
    ptJustG,
    ptJustAb,
    ptJustA,
    ptJustBb,
    ptJustB,
    ptUser1,
    ptUser2,
    ptUser3,
    ptUser4,
    ptUser5,
    ptUser6,
    ptUser7,
    ptUser8,
    ptUserLast = ptUser8
};

uint8_t packTuning(Tuning tuning);
Tuning unpackTuning(uint8_t packed);

std::string describeRoundKind(RoundKind kind);
std::string describeRoundKindShort(RoundKind kind);
std::string describeTuning(Tuning grid);

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