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
    UserTuning = 80,
    UserTuningLast = 87
};
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
    ptUser,
    ptUserLast = ptUser + (Tuning::UserTuningLast - Tuning::UserTuning)
};

uint8_t packTuning(Tuning tuning);
Tuning unpackTuning(uint8_t packed);
std::string describeRoundKind(RoundKind kind);
std::string describeTuning(Tuning grid);

struct Rounding
{
    uint8_t rate;
    bool initial;
    RoundKind kind;
    RoundEqual equal;
    Tuning tuning;

    Rounding()
    :   rate(0),
        initial(false),
        kind(RoundKind::Normal),
        equal(RoundEqual::Equal),
        tuning(Tuning::EqualTuning)
    {}
};

}
#endif