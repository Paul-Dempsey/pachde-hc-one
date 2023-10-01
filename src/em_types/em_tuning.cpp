#include "em_tuning.hpp"

namespace em_midi {
using namespace pachde;

Tuning unpackTuning(uint8_t packed)
{
    if (packed <= ptFiftyTone) {
        return static_cast<Tuning>(packed);
    }
    if (in_range<uint8_t>(packed, ptJustC, ptJustB)) {
        return static_cast<Tuning>(Tuning::JustC + (packed - ptJustC));
    }
    if (in_range<uint8_t>(packed, ptUser1, ptUserLast)) {
        return static_cast<Tuning>(Tuning::UserTuning1 +  (packed - ptUser1));
    }
    return Tuning::EqualTuning;
}

uint8_t packTuning(Tuning tuning)
{
    if (tuning <= Tuning::FiftyTone) {
        return static_cast<uint8_t>(tuning);
    }
    if (in_range(tuning, Tuning::JustC, Tuning::JustB)) {
        return ptJustC + (tuning - Tuning::JustC);
    }
    if (in_range(tuning, Tuning::UserTuning1, Tuning::UserTuningLast)) {
        return ptUser1 + (tuning - Tuning::UserTuning1);
    }
    return 0;
}

std::string describeTuning(Tuning grid)
{
    switch (grid) {
    case Tuning::EqualTuning:  return "Equal"; break;
    case Tuning::JustC:  return "Just C"; break;
    case Tuning::JustCs: return "Just C#"; break;
    case Tuning::JustD:  return "Just D"; break;
    case Tuning::JustEb: return "Just Eb"; break;
    case Tuning::JustF:  return "Just F"; break;
    case Tuning::JustFs: return "Just F#"; break;
    case Tuning::JustG:  return "Just G"; break;
    case Tuning::JustAb: return "Just Ab"; break;
    case Tuning::JustA:  return "Just A"; break;
    case Tuning::JustBb: return "Just Bb"; break;
    case Tuning::JustB:  return "Just B"; break;
    default:
        if (in_range(grid, Tuning::OneTone, Tuning::FiftyTone)) {
            return format_string("%d-tone equal", 1 + grid - Tuning::OneTone);
        }
        if (in_range(grid, Tuning::UserTuning1, Tuning::UserTuningLast)) {
            return format_string("User %d", 1 + grid - Tuning::UserTuning1);
        }
        return "Unknown";
    }
}
std::string describePackedTuning(PackedTuning grid)
{
    return describeTuning(unpackTuning(grid));
}

}