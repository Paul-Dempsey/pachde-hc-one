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
    j = json_object_get(root, "midi-rate");
    if (j) {
        midi_rate = json_integer_value(j);
        if (!in_range(static_cast<int>(midi_rate), 0, 2)) {
            midi_rate = 0;
        }
    }
}

json_t* InitPhaseInfo::toJson() const {
    auto root = json_object();
    json_object_set_new(root, "delay", json_real(post_delay));
    json_object_set_new(root, "midi_rate", json_integer(midi_rate));
    return root;
}

void RefreshPhases(std::vector<InitPhaseInfo>& phases) {
    for (InitPhaseInfo& phase: phases) {
        phase.state = InitState::Uninitialized;
    }
}

}