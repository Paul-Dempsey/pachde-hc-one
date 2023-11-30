#pragma once
#ifndef Compress_HPP_INCLUDED
#define Compress_HPP_INCLUDED
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

using namespace em_midi;
namespace pachde {

struct CompressModuleWidget;

struct CompressModule : Module, ISendMidi, IHandleHcEvents
{
    enum Params
    {
        P_COMP_THRESHOLD,
        P_COMP_ATTACK,
        P_COMP_RATIO,
        P_COMP_MIX,
        P_COMP_THRESHOLD_REL,
        P_COMP_ATTACK_REL,
        P_COMP_RATIO_REL,
        P_COMP_MIX_REL,

        NUM_PARAMS,
    };
    enum Inputs
    {
        IN_COMP_THRESHOLD,
        IN_COMP_ATTACK,
        IN_COMP_RATIO,
        IN_COMP_MIX,

        NUM_INPUTS
    };
    enum Outputs
    {
        NUM_OUTPUTS
    };
    enum Lights
    {
        L_COMP_THRESHOLD_REL,
        L_COMP_ATTACK_REL,
        L_COMP_RATIO_REL,
        L_COMP_MIX_REL,

        L_COMPRESSOR,

        NUM_LIGHTS
    };

    Compressor compressor;
    TiltEq tilt_eq;

    IHandleHcEvents * ui_event_sink = nullptr;
    PartnerBinding partner_binding;
    Hc1Module* getPartner();
    bool partner_subscribed = false;

    // cv processing
    const int CV_INTERVAL = 64;
    RateTrigger control_rate;
    rack::dsp::SchmittTrigger round_initial_trigger;

    explicit CompressModule();
    virtual ~CompressModule();

    void pullCompressor(Hc1Module * partner = nullptr);
    void pushCompressor(Hc1Module * partner = nullptr);

    void absoluteCV();
    void relativeCV();

    void processCV(int paramId);
    void processCompressorControls();
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
    void onCompressorChanged(const CompressorChangedEvent& e) override;
    //void onTiltEqChanged(const TiltEqChangedEvent& e) override;
    void onDeviceChanged(const DeviceChangedEvent& e) override;
    void onDisconnect(const DisconnectEvent& e) override;
};

struct CompressModuleWidget : ModuleWidget, IHandleHcEvents
{
    CompressModule* my_module = nullptr;
    PartnerPicker* partner_picker = nullptr;

    explicit CompressModuleWidget(CompressModule * module);
    virtual ~CompressModuleWidget() {
        // if (my_module) {
        //     my_module->ui_event_sink = nullptr;
        // }
    }
    Hc1Module* getPartner();
    void createCompressorUI();

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
