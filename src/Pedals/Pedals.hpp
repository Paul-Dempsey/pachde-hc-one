#pragma once
#ifndef PEDALS_HPP_INCLUDED
#define PEDALS_HPP_INCLUDED
#include <stdint.h>
#include "../em_midi.hpp"
#include "../em_types.hpp"
#include "../hc_events.hpp"
#include "../module_broker.hpp"
#include "../plugin.hpp"
#include "../presets.hpp"
#include "../widgets/symbol_widget.hpp"
#include "../widgets/vert_slider.hpp"

using namespace ::rack;
namespace pachde {

struct PedalCore : Module, ISendMidi, IHandleHcEvents
{
    enum Params
    {
        P_PEDAL_ASSIGN,
        P_PEDAL_VALUE,
        NUM_PARAMS,
    };
    enum Inputs
    {
        I_PEDAL_VALUE,
        NUM_INPUTS
    };
    enum Outputs
    {
        O_PEDAL_VALUE,
        NUM_OUTPUTS
    };
    enum Lights
    {
        NUM_LIGHTS
    }; 

    uint8_t pedal_id;
    IHandleHcEvents * ui_event_sink = nullptr;
    PartnerBinding partner_binding;
    Hc1Module* getPartner();
    bool partner_subscribed = false;
    RateTrigger control_rate;
    const int CV_INTERVAL = 32;
    int check_cv = 0;
    uint8_t last_pedal_value = 0;
    
    explicit PedalCore(uint8_t id);
    virtual ~PedalCore();

    // IHandleHcEvents
    //void onPresetChanged(const PresetChangedEvent& e) override;
    void onPedalChanged(const PedalChangedEvent& e) override;
    //void onRoundingChanged(const RoundingChangedEvent& e) override;
    void onDeviceChanged(const DeviceChangedEvent& e) override;
    void onDisconnect(const DisconnectEvent& e) override;

    // ISendMidi
    void sendControlChange(uint8_t channel, uint8_t cc, uint8_t value) override;
    bool readyToSend() override;

    void onSampleRateChange() override {
        control_rate.onSampleRateChanged();
    }
    void syncAssign(Hc1Module * partner);
    void syncValue(Hc1Module * partner);
    void process(const ProcessArgs& args) override;
};

struct PedalUICore : ModuleWidget, IHandleHcEvents
{
    PedalCore* core_module;
    SymbolTipWidget* pedal_type = nullptr;
    StaticTextLabel* pedal_assign = nullptr;
    StaticTextLabel* device_label = nullptr;
    StaticTextLabel* pedal_amount = nullptr;
    VerticalSlider* slider = nullptr;
    uint8_t pedal_id = 0;

    explicit PedalUICore(PedalCore* module);

    Hc1Module * getPartner() {
        if (!core_module) return nullptr;
        return core_module->getPartner();
    }

    void createUI();

    // IHandleHcEvents
    void onPresetChanged(const PresetChangedEvent& e) override;
    void onPedalChanged(const PedalChangedEvent& e) override;
    // void onRoundingChanged(const RoundingChangedEvent& e) override;
    void onDeviceChanged(const DeviceChangedEvent& e) override;
    void onDisconnect(const DisconnectEvent& e) override;

};

//
// ========================================================================
//

struct Pedal1Module : PedalCore
{
    explicit Pedal1Module() : PedalCore(0) {}
};

struct Pedal2Module : PedalCore
{
    explicit Pedal2Module() : PedalCore(1) {}
};

struct Pedal1UI: PedalUICore
{
    explicit Pedal1UI(Pedal1Module* module);
};

struct Pedal2UI: PedalUICore
{
    explicit Pedal2UI(Pedal2Module* module);
};

}

#endif