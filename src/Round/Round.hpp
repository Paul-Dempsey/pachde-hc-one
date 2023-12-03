#pragma once
#ifndef ROUND_HPP_INCLUDED
#define ROUND_HPP_INCLUDED
#include <stdint.h>
#include "../hc_events.hpp"
#include "../module_broker.hpp"
#include "../plugin.hpp"
#include "../widgets/label_widget.hpp"
#include "../widgets/partner_picker.hpp"
#include "../widgets/symbol_widget.hpp"

namespace pachde {

using Symbol = SymbolWidget::Symbol;


struct RoundModule : Module, ISendMidi, IHandleHcEvents
{
    enum Params
    {
        P_ROUND_RATE,    // 0..127 cc25
        P_ROUND_INITIAL, // 0..1 cc 28
        P_ROUND_KIND,    // 0..3 ch16 cc61, but includes reverse surface bit
        P_ROUND_TUNING,  // 0..69
        P_ROUND_RATE_REL,
        NUM_PARAMS,
    };
    enum Inputs
    {
        IN_ROUND_RATE,
        IN_ROUND_INITIAL,
        NUM_INPUTS
    };
    enum Outputs
    {
        NUM_OUTPUTS
    };
    enum Lights
    {
        L_ROUND_RATE_REL,
        L_ROUND_INITIAL,
        ROUND_Y_LIGHT, ROUND_INITIAL_LIGHT, ROUND_LIGHT, ROUND_RELEASE_LIGHT,
        NUM_LIGHTS
    };

    Rounding rounding;

    PartnerBinding partner_binding;
    bool partner_subscribed = false;

    IHandleHcEvents * ui_event_sink = nullptr;
    const int CV_INTERVAL = 128;
    RateTrigger control_rate;
    rack::dsp::SchmittTrigger round_initial_trigger;

    void pullRounding(Hc1Module * partner = nullptr);
    void pushRounding(Hc1Module * partner = nullptr);
    void processCV(int paramId);
    void processRoundingControls();
    void processControls();

    RoundModule();
    virtual ~RoundModule();
    Hc1Module * getPartner();

    // ISendMidi
    //virtual void sendNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {}
    //virtual void sendNoteOff(uint8_t channel, uint8_t note, uint8_t velocity) {}
    void sendControlChange(uint8_t channel, uint8_t cc, uint8_t value) override;
    //void sendProgramChange(uint8_t channel, uint8_t program) {}
    //void sendKeyPressure(uint8_t channel, uint8_t note, uint8_t pressure) {}
    //void sendChannelPressure(uint8_t channel, uint8_t pressure) {}
    //void sendPitchBend(uint8_t channel, uint8_t bend_lo, uint8_t bend_hi) {}
    bool readyToSend() override;

    // IHandleHcEvents
    void onPresetChanged(const PresetChangedEvent& e) override;
    void onRoundingChanged(const RoundingChangedEvent& e) override;
    // void onPedalChanged(const PedalChangedEvent& e) override;
    void onDeviceChanged(const DeviceChangedEvent& e) override;
    void onDisconnect(const DisconnectEvent& e) override;
    // void onFavoritesFileChanged(const FavoritesFileChangedEvent& e) override;

    // Module
    void onSampleRateChange(const SampleRateChangeEvent& e) override {
        control_rate.onSampleRateChanged(e);
    }
    json_t *dataToJson() override;
    void dataFromJson(json_t *root) override;
    void process(const ProcessArgs& args) override;
};


struct RoundModuleWidget : ModuleWidget, IHandleHcEvents
{
    RoundModule * my_module;
    PartnerPicker* partner_picker = nullptr;

    DynamicTextLabel* tuning_text = nullptr;
    DynamicTextLabel* type_text = nullptr;

    explicit RoundModuleWidget(RoundModule * module);
    virtual ~RoundModuleWidget() {
        if (my_module) {
            my_module->ui_event_sink = nullptr;
        }
    }

   Hc1Module * getPartner();

    // IHandleHcEvents
    void onPresetChanged(const PresetChangedEvent& e) override;
    void onRoundingChanged(const RoundingChangedEvent& e) override;
    void onDeviceChanged(const DeviceChangedEvent& e) override;
    void onDisconnect(const DisconnectEvent& e) override;
    // void onFavoritesFileChanged(const FavoritesFileChangedEvent& e) override;

};

}
#endif