// Copyright (C) Paul Chase Dempsey
#pragma once
#ifndef HC2_HPP_INCLUDED
#define HC2_HPP_INCLUDED
#include <stdint.h>
#include "../em_midi.hpp"
#include "../em.hpp"
#include "../hc_events.hpp"
#include "../HC-1/HC-1.hpp"
#include "../module_broker.hpp"
#include "../plugin.hpp"
#include "../presets.hpp"
#include "../widgets/label_widget.hpp"
#include "../widgets/partner_picker.hpp"
#include "../widgets/symbol_widget.hpp"
#include "cc_map_widget.hpp"

// #define VERBOSE_LOG
// #include "../debug_log.hpp"

using namespace em_midi;
namespace pachde {

using Symbol = SymbolWidget::Symbol;
struct Hc2ModuleWidget;

struct Hc2Module : Module, ISendMidi, IHandleHcEvents
{
    enum Params
    {
        NUM_PARAMS,
    };
    enum Inputs
    {
        NUM_INPUTS
    };
    enum Outputs
    {
        NUM_OUTPUTS
    };
    enum Lights
    {
        NUM_LIGHTS
    };

    IHandleHcEvents * ui_event_sink = nullptr;
    PartnerBinding partner_binding;
    Hc1Module* getPartner();

    // cv processing
    const int CV_INTERVAL = 64;
    RateTrigger control_rate;

    explicit Hc2Module();
    virtual ~Hc2Module();

    void processCV(int paramId);
    void processControls();

    // Module
    void onSampleRateChange(const SampleRateChangeEvent& e) override {
        control_rate.onSampleRateChanged(e);
    }
    json_t * dataToJson() override;
    void dataFromJson(json_t *root) override;
    void process(const ProcessArgs& args) override;

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
    //void onPresetChanged(const PresetChangedEvent& e) override;
    //void onPedalChanged(const PedalChangedEvent& e) override;
    //void onRoundingChanged(const RoundingChangedEvent& e) override;
    //void onCompressorChanged(const CompressorChangedEvent& e) override;
    //void onTiltEqChanged(const TiltEqChangedEvent& e) override;
    void onDeviceChanged(const DeviceChangedEvent& e) override;
    void onDisconnect(const DisconnectEvent& e) override;
};

struct Hc2ModuleWidget : ModuleWidget, IHandleHcEvents
{
    Hc2Module* my_module = nullptr;
    PartnerPicker* partner_picker = nullptr;

    explicit Hc2ModuleWidget(Hc2Module * module);
    virtual ~Hc2ModuleWidget() {
        // if (my_module) {
        //     my_module->ui_event_sink = nullptr;
        // }
    }
    Hc1Module* getPartner();

    // IHandleHcEvents
    //void onPresetChanged(const PresetChangedEvent& e) override;
    //void onPedalChanged(const PedalChangedEvent& e) override;
    //void onRoundingChanged(const RoundingChangedEvent& e) override;
    void onDeviceChanged(const DeviceChangedEvent& e) override;
    void onDisconnect(const DisconnectEvent& e) override;

    //void draw(const DrawArgs& args) override;
    void appendContextMenu(Menu *menu) override;
};

}
#endif
