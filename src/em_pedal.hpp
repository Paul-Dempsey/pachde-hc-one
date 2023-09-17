#pragma once
#ifndef PEDAL_HPP_INCLUDED
#define PEDAL_HPP_INCLUDED
#include <rack.hpp>

namespace em_midi {

std::string ShortPedalAssignment(uint8_t pedalcc);

enum PedalType : uint8_t {
    NoPedal,
    SwitchPedal,
    ExpressionPedal,
    DamperPedal,
    TriValuePedal,
    CVPedal,
    PedalPot
};

}
#endif