#pragma once
#ifndef HC_EVENTS_HPP_INCLUDED
#define HC_EVENTS_HPP_INCLUDED
#include <rack.hpp>
#include "presets.hpp"
//#include "em_types.hpp"

namespace pachde {

struct IHandleHcEvents
{
    struct PresetChangedEvent {
        std::shared_ptr<Preset> preset;
    };
    virtual void onPresetChanged(const PresetChangedEvent& e) = 0;

    struct RoundingChangedEvent {
        Rounding rounding;
    };
    virtual void onRoundingChanged(const RoundingChangedEvent& e) = 0;

};

}
#endif