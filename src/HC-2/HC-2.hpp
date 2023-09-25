#pragma once
#ifndef HC2_HPP_INCLUDED
#define HC2_HPP_INCLUDED
#include <stdint.h>
#include "../plugin.hpp"
#include "../HC-1/HC-1.hpp"
#include "../em_midi.hpp"
#include "../presets.hpp"
#include "../em_types.hpp"
#include "../hc_events.hpp"

// #define VERBOSE_LOG
// #include "../debug_log.hpp"

using namespace em_midi;
namespace pachde {

struct Hc2ModuleWidget;

struct Hc2Module : Module, ISendMidi, IHandleHcEvents
{
    enum Params
    {
        P_ROUND_RATE,    // 0..127 cc25
        P_ROUND_INITIAL, // 0..1 cc 28
        P_ROUND_KIND,    // 0..3 ch16 cc61, but includes reverse surface bit
        P_ROUND_TUNING,  // 0..69
        //P_ROUND_EQUAL,
        P_ROUND_RATE_REL,
        //P_TEST,
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
        NUM_LIGHTS
    };
    Rounding rounding;

    IHandleHcEvents * ui_event_sink = nullptr;
    Hc1Module* getPartner();
    bool partner_subscribed = false;

    // cv processing
    const int CV_INTERVAL = 64;
    int check_cv = 0;
    RateTrigger control_rate;
    rack::dsp::SchmittTrigger round_initial_trigger;

    Hc2Module();
    virtual ~Hc2Module();

    void onSampleRateChange() override {
        control_rate.onSampleRateChanged();
    }

    // json_t * dataToJson() override;
    // void dataFromJson(json_t *root) override;

    void pullRounding(Hc1Module * partner = nullptr);
    void pushRounding(Hc1Module * partner = nullptr);
    void syncParam(int paramId);
    void processCV(int paramId);
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
    void onPresetChanged(const PresetChangedEvent& e) override;
    void onRoundingChanged(const RoundingChangedEvent& e) override;
    void onDeviceChanged(const DeviceChangedEvent& e) override;
    void onDisconnect(const DisconnectEvent& e) override;
};

struct Hc2ModuleWidget : ModuleWidget, IHandleHcEvents
{
    Hc2Module * my_module = nullptr;
    StaticTextLabel* device_label = nullptr;
    DynamicTextLabel* rounding_summary = nullptr;

    explicit Hc2ModuleWidget(Hc2Module * module);
    virtual ~Hc2ModuleWidget() {
        if (my_module) {
            my_module->ui_event_sink = nullptr;
        }
    }
    Hc1Module* getPartner();
    void createRoundingUI(float x, float y);

    // IHandleHcEvents
    void onPresetChanged(const PresetChangedEvent& e) override;
    void onRoundingChanged(const RoundingChangedEvent& e) override;
    void onDeviceChanged(const DeviceChangedEvent& e) override;
    void onDisconnect(const DisconnectEvent& e) override;

    void drawCCMap(const DrawArgs& args, Hc1Module * partner);
    void draw(const DrawArgs& args) override;
    void appendContextMenu(Menu *menu) override;
};

}
#endif
