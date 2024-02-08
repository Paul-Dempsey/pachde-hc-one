#pragma once
#ifndef TILT_HPP_INCLUDED
#define TILT_HPP_INCLUDED
#include "../hc_events.hpp"
#include "../module_broker.hpp"
#include "../plugin.hpp"
#include "../widgets/label_widget.hpp"
#include "../widgets/partner_picker.hpp"
//#include "../widgets/symbol_widget.hpp"

namespace pachde {

using Symbol = SymbolWidget::Symbol;


struct TiltModule : Module, ISendMidi, IHandleHcEvents
{
    enum Params
    {
        P_TEQ_TILT,
        P_TEQ_FREQ,
        P_TEQ_MIX,
        P_TEQ_TILT_REL,
        P_TEQ_FREQ_REL,
        P_TEQ_MIX_REL,

        NUM_PARAMS,
    };
    enum Inputs
    {
        IN_TEQ_TILT,
        IN_TEQ_FREQ,
        IN_TEQ_MIX,

        NUM_INPUTS
    };
    enum Outputs
    {
        NUM_OUTPUTS
    };
    enum Lights
    {
        L_TEQ_TILT_REL,
        L_TEQ_FREQ_REL,
        L_TEQ_MIX_REL,

        L_TEQ,

        NUM_LIGHTS
    };

    TiltEq tilt_eq;


    IHandleHcEvents * ui_event_sink = nullptr;
    PartnerBinding partner_binding;
    const int CV_INTERVAL = 128;
    RateTrigger control_rate;

    explicit TiltModule();
    virtual ~TiltModule();
    Hc1Module * getPartner();

    void pullTiltEq(Hc1Module * partner = nullptr);
    void pushTiltEq(Hc1Module * partner = nullptr);
    void processTiltEqControls();
    void processControls();
    void processCV(int paramId);

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
    //void onRoundingChanged(const RoundingChangedEvent& e) override;
    //void onPedalChanged(const PedalChangedEvent& e) override;
    void onTiltEqChanged(const TiltEqChangedEvent& e) override;
    void onDeviceChanged(const DeviceChangedEvent& e) override;
    void onDisconnect(const DisconnectEvent& e) override;
    //void onFavoritesFileChanged(const FavoritesFileChangedEvent& e) override;

    // Module
    json_t *dataToJson() override;
    void dataFromJson(json_t *root) override;
    void process(const ProcessArgs& args) override;
};

using TiltO = TiltModule::Outputs;

struct TiltModuleWidget : ModuleWidget, IHandleHcEvents
{
    TiltModule * my_module;
    PartnerPicker* partner_picker = nullptr;

    explicit TiltModuleWidget(TiltModule * module);

    virtual ~TiltModuleWidget() {
        if (my_module) {
            my_module->ui_event_sink = nullptr;
        }
    }

   Hc1Module * getPartner();

    // IHandleHcEvents
    // void onPresetChanged(const PresetChangedEvent& e) override;
    // void onRoundingChanged(const RoundingChangedEvent& e) override;
    void onDeviceChanged(const DeviceChangedEvent& e) override;
    void onDisconnect(const DisconnectEvent& e) override;
    // void onFavoritesFileChanged(const FavoritesFileChangedEvent& e) override;

    void appendContextMenu(Menu *menu) override;
};



}
#endif