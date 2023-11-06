#include "em_pedal.hpp"
#include "../misc.hpp"

namespace eagan_matrix {

std::string ShortPedalAssignment(uint8_t pedalcc)
{
    switch (pedalcc) {
    case 7: return "Vol"; // not in HE menu
    case 8: return "Oct";
    case 9: return "Mono";
    case 10: return "Fine"; // 4..124 = -60..+60 cents 
    case 11: return "Exp"; // not in HE menu
    case 12: return "i";
    case 13: return "ii";
    case 14: return "iii";
    case 15: return "iv";
    case 16: return "v";
    case 17: return "vi";
    case 18: return "Post";
    case 19: return "AIn";
    case 20: return "R1";
    case 21: return "R2";
    case 22: return "R3";
    case 23: return "R4";
    case 24: return "RMix";
    case 25: return "RRt";
    case 26: return "Pre";
    case 27: return "Att"; // not in HE menu
    case 28: return "RIn";
    case 29: return "PJ1"; // not in HE menu
    case 30: return "PJ2"; // not in HE menu
    case 31: return "Adv";

    case 64: return "Sus";
    case 65: return "Equal";
    case 66: return "Sos";
    case 69: return "Sos2";
    }
    return pachde::format_string("cc%d", pedalcc);
}

std::string LongPedalAssignment(uint8_t pedalcc)
{
    switch (pedalcc) {
    case 7: return "Volume"; // not in HE menu
    case 8: return "Octave shift";
    case 9: return "Mono switch";
    case 10: return "Fine tune"; // 4..124 = -60..+60 cents 
    case 11: return "Expression"; // not in HE menu
    case 12: return "i";
    case 13: return "ii";
    case 14: return "iii";
    case 15: return "iv";
    case 16: return "v";
    case 17: return "vi";
    case 18: return "Post level";
    case 19: return "Audio input level";
    case 20: return "R-1";
    case 21: return "R-2";
    case 22: return "R-3";
    case 23: return "R-4";
    case 24: return "Recirculator Mix";
    case 25: return "Round Rate";
    case 26: return "Pre level";
    case 27: return "Attenuation"; // not in HE menu
    case 28: return "Round Initial";
    case 29: return "Pedal Jack 1"; // not in HE menu
    case 30: return "Pedal Jack 2"; // not in HE menu
    case 31: return "Preset advance";
    case 64: return "Sustain";
    case 65: return "Round Equal";
    case 66: return "Sostenuto";
    case 69: return "Sostenuto 2";
    }
    return pachde::format_string("cc %d", pedalcc);
}

PedalAssign PedalAssignFromCC(uint8_t cc)
{
    switch (cc) {
    case EMCC_OctaveShift:  return PedalAssign::OctaveShift;
    case EMCC_MonoSwitch:   return PedalAssign::MonoSwitch;
    case EMCC_Advance:      return PedalAssign::PresetAdvance;
    case EMCC_FineTune:     return PedalAssign::FineTune;
    case MidiCC_Expression: return PedalAssign::Expression;
    case MidiCC_Volume:     return PedalAssign::Volume;
    case EMCC_PreLevel:     return PedalAssign::PreLevel;
    case EMCC_PostLevel:    return PedalAssign::PostLevel;
    case EMCC_AudioInLevel: return PedalAssign::AudioInputLevel;
    //case EMCC_Jack1:        return PedalAssign::Jack1;
    //case EMCC_Jack2:        return PedalAssign::Jack2;
    //case EMCC_Attenuation:  return PedalAssign::Attenuation;
    case EMCC_Sustain:      return PedalAssign::Sustain;
    case EMCC_Sostenuto:    return PedalAssign::Sostenuto;
    case EMCC_Sostenuto2:   return PedalAssign::Sostenuto2;
    case EMCC_i:            return PedalAssign::Macro_i;
    case EMCC_ii:           return PedalAssign::Macro_ii;
    case EMCC_iii:          return PedalAssign::Macro_iii;
    case EMCC_iv:           return PedalAssign::Macro_iv;
    case EMCC_v:            return PedalAssign::Macro_v;
    case EMCC_vi:           return PedalAssign::Macro_vi;
    case EMCC_R1:           return PedalAssign::R1;
    case EMCC_R2:           return PedalAssign::R2;
    case EMCC_R3:           return PedalAssign::R3;
    case EMCC_R4:           return PedalAssign::R4;
    case EMCC_RMIX:         return PedalAssign::RecirculatorMix;
    case EMCC_RoundRate:    return PedalAssign::RoundRate;
    case EMCC_RoundInitial: return PedalAssign::RoundInitial;
    case EMCC_RoundEqual:   return PedalAssign::RoundEqual;
    default: return PedalAssign::Sustain;
    }
}

uint8_t PedalCC(PedalAssign assign)
{
    switch (assign) {
    case PedalAssign::OctaveShift:     return EMCC_OctaveShift;
    case PedalAssign::MonoSwitch:      return EMCC_MonoSwitch;
    case PedalAssign::PresetAdvance:   return EMCC_Advance;
    case PedalAssign::FineTune:        return EMCC_FineTune;
    case PedalAssign::Expression:      return MidiCC_Expression;
    case PedalAssign::Volume:          return MidiCC_Volume;
    case PedalAssign::PreLevel:        return EMCC_PreLevel;
    case PedalAssign::PostLevel:       return EMCC_PostLevel;
    case PedalAssign::AudioInputLevel: return EMCC_AudioInLevel;
    //case PedalAssign::Jack1:           return EMCC_Jack1;
    //case PedalAssign::Jack2:           return EMCC_Jack2;
    //case PedalAssign::Attenuation:     return EMCC_Attenuation;
    case PedalAssign::Sustain:         return EMCC_Sustain;
    case PedalAssign::Sostenuto:       return EMCC_Sostenuto;
    case PedalAssign::Sostenuto2:      return EMCC_Sostenuto2;
    case PedalAssign::Macro_i:         return EMCC_i;
    case PedalAssign::Macro_ii:        return EMCC_ii;
    case PedalAssign::Macro_iii:       return EMCC_iii;
    case PedalAssign::Macro_iv:        return EMCC_iv;
    case PedalAssign::Macro_v:         return EMCC_v;
    case PedalAssign::Macro_vi:        return EMCC_vi;
    case PedalAssign::R1:              return EMCC_R1;
    case PedalAssign::R2:              return EMCC_R2;
    case PedalAssign::R3:              return EMCC_R3;
    case PedalAssign::R4:              return EMCC_R4;
    case PedalAssign::RecirculatorMix: return EMCC_RMIX;
    case PedalAssign::RoundRate:       return EMCC_RoundRate;
    case PedalAssign::RoundInitial:    return EMCC_RoundInitial;
    case PedalAssign::RoundEqual:      return EMCC_RoundEqual;
    default: return EMCC_Sostenuto;
    }
}

}