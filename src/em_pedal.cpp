#include "em_pedal.hpp"
#include "misc.hpp"

namespace em_midi {

std::string ShortPedalAssignment(uint8_t pedalcc)
{
    switch (pedalcc) {
    case 7: return "Vol";
    case 8: return "Oct";
    case 9: return "Mono";
    case 10: return "Fine"; // 4..124 = -60..+60 cents 
    case 11: return "Exp";
    case 12: return "i";
    case 13: return "ii";
    case 14: return "iii";
    case 15: return "iv";
    case 16: return "v";
    case 17: return "vi";
    case 18: return "Post";
    case 19: return "AudIn";
    case 20: return "R1";
    case 21: return "R2";
    case 22: return "R3";
    case 23: return "R4";
    case 24: return "RMix";
    case 25: return "RRt";
    case 28: return "RIn";
    case 31: return "Adv";
    case 64: return "Sus";
    case 65: return "Equal";
    case 66: return "Sos";
    case 69: return "Sos2";
    }
    return pachde::format_string("cc%d", pedalcc);
}

}