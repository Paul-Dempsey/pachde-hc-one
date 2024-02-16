// Copyright (C) Paul Chase Dempsey
#pragma once
#ifndef EM_TUNING_HPP_INCLUDED
#define EM_TUNING_HPP_INCLUDED
#include <rack.hpp>
#include "../misc.hpp"
namespace eagan_matrix {
using namespace ::rack;

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
    assert(pachde::in_range(nth, 1, 50));
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

std::string describeTuning(Tuning grid);

}
#endif