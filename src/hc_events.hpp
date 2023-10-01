#pragma once
#ifndef HC_EVENTS_HPP_INCLUDED
#define HC_EVENTS_HPP_INCLUDED
#include <rack.hpp>
#include "presets.hpp"
#include "em_types.hpp"

namespace pachde {

struct IHandleHcEvents
{
    struct PresetChangedEvent {
        const std::shared_ptr<Preset> preset;
    };
    virtual void onPresetChanged(const PresetChangedEvent& e) {}

    struct FavoritesFileChangedEvent {
        const std::string &path;
    };
    virtual void onFavoritesFileChanged(const FavoritesFileChangedEvent& e) {}

    struct PedalChangedEvent {
        const em_midi::PedalInfo & pedal;
    };
    virtual void onPedalChanged(const PedalChangedEvent& e) {}

    struct RoundingChangedEvent {
        const em_midi::Rounding rounding;
    };
    virtual void onRoundingChanged(const RoundingChangedEvent& e) {}

    struct DeviceChangedEvent {
        const std::string &name;
    };
    virtual void onDeviceChanged(const DeviceChangedEvent& e) {}

    struct DisconnectEvent { };
    virtual void onDisconnect(const DisconnectEvent& e) {}

};

}
#endif