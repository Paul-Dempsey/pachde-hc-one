#pragma once
#ifndef HC_EVENTS_HPP_INCLUDED
#define HC_EVENTS_HPP_INCLUDED
#include <rack.hpp>
#include "presets.hpp"
#include "em_device.hpp"
#include "em.hpp"

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
        const eagan_matrix::PedalInfo & pedal;
    };
    virtual void onPedalChanged(const PedalChangedEvent& e) {}

    struct RoundingChangedEvent {
        const eagan_matrix::Rounding& rounding;
    };
    virtual void onRoundingChanged(const RoundingChangedEvent& e) {}

    struct CompressorChangedEvent {
        const eagan_matrix::Compressor& compressor;
    };
    virtual void onCompressorChanged(const CompressorChangedEvent& e) {}

    struct TiltEqChangedEvent {
        const eagan_matrix::TiltEq& tilt_eq;
    };
    virtual void onTiltEqChanged(const TiltEqChangedEvent& e) {}

    struct DeviceChangedEvent {
        const std::shared_ptr<MidiDeviceConnection> device;
    };
    virtual void onDeviceChanged(const DeviceChangedEvent& e) {}

    struct RoutingChangedEvent {
        const uint8_t routing;
    };
    virtual void onRoutingChanged(const RoutingChangedEvent& e) {}

    struct PolyphonyChangedEvent {
        const eagan_matrix::Polyphony polyphony;
    };
    virtual void onPolyphonyChanged(const PolyphonyChangedEvent& e) {}

    struct NotePriorityChangedEvent {
        const eagan_matrix::NotePriority piority;
    };
    virtual void onNotePriorityChanged(const NotePriorityChangedEvent& e) {}

    struct MpeChangedEvent {
        const eagan_matrix::Mpe& mpe;
    };
    virtual void onMpeChanged(const MpeChangedEvent& e) {}

    struct VelocityChangedEvent {
        const eagan_matrix::VelocitySplit & velocity;
    };
    virtual void onVelocityChanged(const VelocityChangedEvent& e) {}

    struct DisconnectEvent { };
    virtual void onDisconnect(const DisconnectEvent& e) {}

};

}
#endif