#pragma once
#ifndef PEDAL_HPP_INCLUDED
#define PEDAL_HPP_INCLUDED
#include <rack.hpp>
#include "../em_midi.hpp"
#include "../widgets/components.hpp"

namespace em_midi {

std::string ShortPedalAssignment(uint8_t pedalcc);
std::string LongPedalAssignment(uint8_t pedalcc);

// organized by menu/knob value
enum class PedalAssign : uint8_t
{
    OctaveShift, First = OctaveShift,
    MonoSwitch,
    PresetAdvance,
    FineTune,
    Expression,  // Sends cc on ch 16, but has no effect on engine. Has value for CV pedal output.
    Volume,      // Sends cc on ch 16, but has no effect on engine. Has value for CV pedal output.
    PreLevel,
    PostLevel,
    AudioInputLevel,
    //Attenuation, // can be set/sent, but does not behave
    // Jack1,
    // Jack2, // found in BeautifulPursuit, but not sent when configured
    Sustain,
    Sostenuto,
    Sostenuto2,

    Macro_i,
    Macro_ii,
    Macro_iii,
    Macro_iv,
    Macro_v,
    Macro_vi,

    R1,
    R2,
    R3,
    R4,
    RecirculatorMix,

    RoundRate,
    RoundInitial,
    RoundEqual, Last = RoundEqual
};

uint8_t PedalCC(PedalAssign assign);
PedalAssign PedalAssignFromCC(uint8_t cc);

enum PedalType : uint8_t
{
    NoPedal,
    SwitchPedal,
    ExpressionPedal,
    DamperPedal,
    TriValuePedal,
    CVPedal,
    PotPedal
};

struct PedalInfo
{
    uint8_t value;
    uint8_t jack;
    uint8_t cc;
    PedalType type;
    uint8_t min;
    uint8_t max;

    PedalInfo()
    :   value(0),
        jack(0),
        cc(EMCC_Sustain),
        type(PedalType::NoPedal),
        min(0),
        max(127)
    {}

    explicit PedalInfo(uint8_t jack)
    :   value(0),
        jack(jack),
        cc(0),
        type(PedalType::NoPedal),
        min(0),
        max(127)
    {
        switch (jack){
        case 0: cc = EMCC_Sustain; break;
        case 1: cc = EMCC_Sostenuto; break;
        default: assert(false); break;
        }
    }

    void clear()
    {
        value = 0;
        min = 0;
        max = 127;        
        switch (jack){
        case 0: cc = EMCC_Sustain; break;
        case 1: cc = EMCC_Sostenuto; break;
        default: assert(false); break;
        }
    }
};

}
#endif