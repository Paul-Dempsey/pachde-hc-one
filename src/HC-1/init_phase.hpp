#pragma once
#ifndef INIT_PHASE_HPP_INCLUDED
#define INIT_PHASE_HPP_INCLUDED
#include <rack.hpp>
#include <stdint.h>
#include "../misc.hpp"

namespace pachde {

enum class InitPhase : uint8_t {
    None = 0xff,
    DeviceOutput = 0,
    DeviceInput,
    DeviceHello,
    DeviceConfig,
    CachedPresets,
    UserPresets,
    SystemPresets,
    Favorites,
    SavedPreset,
    PresetConfig,
    RequestUpdates,
    Heartbeat,
    Done
};

inline uint8_t PhaseIndex(InitPhase phase) { return static_cast<uint8_t>(phase); }
inline InitPhase NextPhase(InitPhase phase) {
    if (InitPhase::None == phase) return InitPhase::DeviceOutput;
    if (InitPhase::Done == phase) return phase;
    return static_cast<InitPhase>(PhaseIndex(phase) + 1);
}

enum class EMMidiRate : uint8_t {
    Full,
    Third,
    Twentieth
};

const char * PhaseLabel(InitPhase phase);
const char * PhaseName(InitPhase phase);

struct InitPhaseInfo
{
    InitPhase id;
    InitState state;
    EMMidiRate midi_rate;
    float post_delay;
    float budget;

    bool fresh() { return InitState::Uninitialized == state; }
    bool pending() { return InitState::Pending == state; }
    bool finished() { return InitState::Complete == state; }
    bool broken() { return InitState::Broken == state; }

    void refresh() { state = InitState::Uninitialized; }
    void pend()  { state = InitState::Pending; }
    void finish() { state = InitState::Complete; }
    void fail() { state = InitState::Broken; }

    void fromJson(json_t* root);
    json_t* toJson() const;
};

void PhasesToJson(json_t* root, const std::vector<InitPhaseInfo>& phases);
void PhasesFromJson(json_t* root, std::vector<InitPhaseInfo>& phases);
void RefreshPhases(std::vector<InitPhaseInfo>& phases);

}
#endif