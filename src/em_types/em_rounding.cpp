#include "em_rounding.hpp"

namespace eagan_matrix {
using namespace pachde;

std::string describeRoundKind(RoundKind kind)
{
    switch (kind) {
    case RoundKind::Normal: return "Normal";
    case RoundKind::Release: return "Release";
    case RoundKind::Y: return "Y";
    case RoundKind::InverseY: return "Inverse Y";
    }
    return "?";
}
std::string describeRoundKindShort(RoundKind kind)
{
    switch (kind) {
    case RoundKind::Normal: return "N";
    case RoundKind::Release: return "R";
    case RoundKind::Y: return "Y";
    case RoundKind::InverseY: return "iY";
    }
    return "?";
}

}