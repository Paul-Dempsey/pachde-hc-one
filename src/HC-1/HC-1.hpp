#pragma once
#ifndef HC_ONE_HPP_INCLUDED
#define HC_ONE_HPP_INCLUDED
#include <stdint.h>
#include "../plugin.hpp"
#include "../em_midi.hpp"
#include "../presets.hpp"
#include "../midi_input_proxy.hpp"
#include "../text.hpp"

using namespace em_midi;
namespace pachde {

#define VERBOSE_LOG
#ifdef VERBOSE_LOG
#define DebugLog(format, ...) DEBUG(format, ##__VA_ARGS__)
#else
#define DebugLog(format, ...) {}
#endif


extern const NVGcolor blue_light;
extern const NVGcolor green_light;
extern const NVGcolor bright_green_light;
extern const NVGcolor orange_light;
extern const NVGcolor yellow_light;
extern const NVGcolor red_light;
extern const NVGcolor white_light;
extern const NVGcolor purple_light;
extern const NVGcolor preset_name_color;

struct Hc1Module : Module, IProcessMidi
{
    enum Params
    {
        M1_PARAM, M2_PARAM, M3_PARAM, M4_PARAM, M5_PARAM, M6_PARAM,
        R1_PARAM, R2_PARAM, R3_PARAM, R4_PARAM, RMIX_PARAM,
        NUM_PARAMS
    };
    enum Inputs
    {
        M1_INPUT, M2_INPUT, M3_INPUT, M4_INPUT, M5_INPUT, M6_INPUT,
        R1_INPUT, R2_INPUT, R3_INPUT, R4_INPUT, RMIX_INPUT,
        NUM_INPUTS
    };
    enum Outputs
    {
        NUM_OUTPUTS
    };
    enum Lights
    {
        HEART_LIGHT,
        NUM_LIGHTS
    };

    Preset preset0;
    bool requested_config = false;
    bool have_config = false;
    bool requested_updates = false;
    bool waiting_for_handshake = false;
    bool in_preset = false;
    uint16_t firmware_version = 0;
    uint8_t dsp[3] {0};
    int data_stream = -1;

    // heartbeat
    float heart_phase = 0.f;
    float heart_time = 1.0;
    bool tick_tock = true;
    NVGcolor ledColor = green_light;

    // device management
    bool is_eagan_matrix = false;
    int inputDeviceId = -1;
    std::string device_name;

    // cc handling
    uint8_t pedal_fraction = 0;
    int64_t notesOn = 0;
    uint8_t recirculator = 0;
    int download_message_id = -1; // CC109

    MidiInputProxy midiInput;
    midi::Output midiOutput;

    // cv processing
    const int CV_INTERVAL = 64;
    int check_cv = 0;
    const float MIDI_RATE = 0.01f;
    rack::dsp::Timer midi_timer;

    const std::string deviceName() { return device_name; }
    bool isEaganMatrix() { return is_eagan_matrix; }
    
    Hc1Module();

    // void onSampleRateChange() override {
    //     float rate = APP->engine->getSampleRate();
    // }

    void paramToDefault(int id) {
        auto pq = getParamQuantity(id);
        getParam(id).setValue(pq->getDefaultValue());
    }
    void centerKnobs();

    json_t *dataToJson() override;
    void dataFromJson(json_t *root) override;

    void onReset() override;
    void findEMOut();
    void findEM();

    EM_Recirculator recirculatorType() {
        return static_cast<EM_Recirculator>(recirculator & EM_Recirculator::Mask);
    }
    bool isExtendRecirculator() { return recirculator & EM_Recirculator::Extend; }
    const std::string recirculatorName() {
        return RecirculatorName(recirculatorType());
    }
    const std::string recirculatorParameterName(int r) {
        return RecirculatorParameterName(recirculatorType(), r);
    }
    bool isRecirculatorExtend() { return recirculator & EM_Recirculator::Extend; }
    void setRecirculatorCCValue(int id, uint8_t value);

    void sendCC(uint8_t channel, uint8_t cc, uint8_t value);
    void transmitRequestUpdates();
    void transmitRequestConfiguration();
    void sendEditorPresent();
    void sendNote(uint8_t channel, uint8_t note, uint8_t velocity);
    void sendNoteOff(uint8_t channel, uint8_t note);
    void handle_ch16_cc(uint8_t cc, uint8_t value);
    void handle_ch16_message(const midi::Message& msg);
    void onNoteOn(uint8_t channel, uint8_t note, uint8_t velocity);
    void onNoteOff(uint8_t channel, uint8_t note, uint8_t velocity);
    void setMacroCCValue(int id, uint8_t value);
    void onSoundOff();
    void onChannel0CC(uint8_t cc, uint8_t value);
    void handle_ch0_message(const midi::Message& msg);
    void processMidi(const midi::Message& msg) override;
    void processCV(int inputId);
    void processAllCV();
    void process(const ProcessArgs& args) override;
};

struct Hc1ModuleWidget : ModuleWidget
{
    Hc1Module *my_module = nullptr;
    GrayModuleLightWidget * status_light;

    Hc1ModuleWidget(Hc1Module *module);
    void step() override;
    void drawLayer(const DrawArgs& args, int layer) override;
    void draw(const DrawArgs& args) override;
};

}
#endif