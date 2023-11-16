#include "init_phase.hpp"

namespace pachde {

const char * PhaseLabel(InitPhase phase) {
    switch (phase) {
    case InitPhase::DeviceOutput:  return "phase-device-output";
    case InitPhase::DeviceInput:   return "phase-device-input";
    case InitPhase::DeviceHello:   return "phase-device-hello";
    case InitPhase::DeviceConfig:  return "phase-device-config";
    case InitPhase::CachedPresets: return "phase-cached-presets";
    case InitPhase::UserPresets:   return "phase-user-presets";
    case InitPhase::SystemPresets: return "phase-system-presets";
    case InitPhase::Favorites:     return "phase-favorites";
    case InitPhase::SavedPreset:   return "phase-saved_preset";
    case InitPhase::PresetConfig:  return "phase-preset-config";
    case InitPhase::RequestUpdates:return "phase-request-updates";
    case InitPhase::Heartbeat:     return "phase-heartbeat";
    case InitPhase::None:          return "phase-none";
    case InitPhase::Done:          return "phase-done";
    default: return "phase-whatever";
    }
}
const char * PhaseName(InitPhase phase) {
    switch (phase) {
    case InitPhase::None:          return "initialize module";
    case InitPhase::DeviceOutput:  return "get MIDI output device";
    case InitPhase::DeviceInput:   return "get MIDI input device";
    case InitPhase::DeviceHello:   return "request initial EM handshake";
    case InitPhase::DeviceConfig:  return "request device configuration";
    case InitPhase::CachedPresets: return "load cached presets";
    case InitPhase::UserPresets:   return "load User presets";
    case InitPhase::SystemPresets: return "load System presets";
    case InitPhase::Favorites:     return "load favorites";
    case InitPhase::SavedPreset:   return "restore saved preset";
    case InitPhase::PresetConfig:  return "request preset";
    case InitPhase::RequestUpdates:return "request updates";
    case InitPhase::Heartbeat:     return "hearbeat handshake";
    case InitPhase::Done:          return "ready";
    default: return "[Unknown]";
    }
}
void PhasesToJson(json_t* root, const std::vector<InitPhaseInfo>& phases)
{
    for (const InitPhaseInfo& phase: phases) {
        json_object_set_new(root, PhaseLabel(phase.id), phase.toJson());
    }
}

void PhasesFromJson(json_t* root, std::vector<InitPhaseInfo>& phases)
{
    for (InitPhaseInfo& phase: phases) {
        phase.fromJson(json_object_get(root, PhaseLabel(phase.id)));
    }
}

void InitPhaseInfo::fromJson(json_t* root) {
    if (!root) return;
    auto j = json_object_get(root, "delay");
    if (j) { post_delay = json_number_value(j); }
    j = json_object_get(root, "budget");
    if (j) { 
        budget = json_number_value(j);
        if (budget < 0.f) budget = 0.f;
    }
    j = json_object_get(root, "midi-rate");
    if (j) {
        midi_rate = static_cast<EMMidiRate>(json_integer_value(j));
        if (!in_range(static_cast<int>(midi_rate), 0, 2)) {
            midi_rate = EMMidiRate::Third;
        }
    }
}

json_t* InitPhaseInfo::toJson() const {
    auto root = json_object();
    json_object_set_new(root, "delay", json_real(post_delay));
    json_object_set_new(root, "midi_rate", json_integer(static_cast<int>(midi_rate)));
    json_object_set_new(root, "budget", json_integer(static_cast<int>(budget)));
    return root;
}

void RefreshPhases(std::vector<InitPhaseInfo>& phases) {
    for (InitPhaseInfo& phase: phases) {
        phase.state = InitState::Uninitialized;
    }
}

}