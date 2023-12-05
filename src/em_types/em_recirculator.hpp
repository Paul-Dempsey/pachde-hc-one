#pragma once
#ifndef EM_RECIRCULATOR_HPP_INCLUDED
#define EM_RECIRCULATOR_HPP_INCLUDED
#include <rack.hpp>
#include "../em_midi.hpp"

namespace eagan_matrix
{
using namespace em_midi;

struct Recirculator
{
    uint8_t r;
    bool disable;

    Recirculator() : r(0), disable(false) {}
    
    operator uint8_t() const { return r; }

    void setValue(uint8_t R) { r = R; }
    uint8_t getValue() { return r; }
    void clear() { r = 0; }

    bool extended() { return r & EM_Recirculator::Extend; }
    bool disabled() { return disable; }
    bool enabled() { return !disable; }

    void setExtended(bool extended) {
        r = extended ? (r | EM_Recirculator::Extend) : (r & ~EM_Recirculator::Extend);
    }
    void setDisabled(bool value) { disable = value; }

    void setKind(EM_Recirculator kind) {
        r = (r & EM_Recirculator::Extend) | (kind & EM_Recirculator::Mask);
    }

    EM_Recirculator kind() {
        return static_cast<EM_Recirculator>(r & EM_Recirculator::Mask);
    }

    const std::string name() {
        return RecirculatorName(kind());
    }

    const std::string parameter_name(int which)
    {
        return RecirculatorParameterName(kind(), which);
    }
};

}
#endif