#include "em_types.hpp"
#include "misc.hpp"

namespace pachde {

Tuning unpackTuning(uint8_t packed)
{
    if (packed <= ptFiftyTone) {
        return static_cast<Tuning>(packed);
    }
    if (in_range<uint8_t>(packed, ptJustC, ptJustB)) {
        return static_cast<Tuning>(Tuning::JustC + (packed - ptJustC));
    }
    if (in_range<uint8_t>(packed, ptUser, ptUserLast)) {
        return static_cast<Tuning>(Tuning::UserTuning +  (packed - ptUser));
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
    if (in_range(tuning, Tuning::UserTuning, Tuning::UserTuningLast)) {
        return ptUser + (tuning - Tuning::UserTuning);
    }
    return 0;
}

std::string describeRoundKind(RoundKind kind)
{
    switch (kind) {
    case RoundKind::Normal: return "Normal";
    case RoundKind::Release: return "Release";
    case RoundKind::Y: return "Y (Full to none)";
    case RoundKind::InverseY: return "Inverse Y (None to full)";
    }
    return "?";
}

std::string describeTuning(Tuning grid)
{
    switch (grid) {
    case Tuning::EqualTuning:  return "Equal (Standard)"; break;
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
        if (in_range(grid, Tuning::UserTuning, Tuning::UserTuningLast)) {
            return format_string("User %d", 1 + grid - Tuning::UserTuning);
        }
        return "Unknown";
    }
}

}