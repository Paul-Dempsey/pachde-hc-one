#pragma once
#ifndef HC_ONE_HPP_INCLUDED
#define HC_ONE_HPP_INCLUDED
#include <stdint.h>
#include "../plugin.hpp"
#include "../em_midi.hpp"
#include "../presets.hpp"
#include "../text.hpp"
#include "../colors.hpp"

using namespace em_midi;
namespace pachde {

#define VERBOSE_LOG
#include "../debug_log.hpp"

const NVGcolor& StatusColor(StatusItem status);

struct Hc1Module : IPresetHolder, ISendMidi, midi::Input, Module
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
    std::vector< std::shared_ptr<MinPreset> > presets;
    std::vector< std::shared_ptr<MinPreset> > user_presets;
    
    bool is_eagan_matrix = false;

    InitState preset_state = InitState::Uninitialized;
    bool hasPresets() { return InitState::Complete == preset_state; }

    InitState config_state = InitState::Uninitialized;
    bool configPending() { return InitState::Pending == config_state; }
    bool hasConfig() { return InitState::Complete == config_state; }

    InitState device_state = InitState::Uninitialized;
    bool hasDevice() { return InitState::Complete == device_state; }

    InitState handshake = InitState::Uninitialized;
    bool handshakePending() { return InitState::Pending == handshake; }

    bool stateError() {
        return InitState::Broken == preset_state
            || InitState::Broken == config_state
            || InitState::Broken == device_state
            || InitState::Broken == handshake;
    }
    bool anyPending() {
        return InitState::Pending == preset_state
            || InitState::Pending == config_state
            || InitState::Pending == device_state
            || InitState::Pending == handshake;
    }

    bool requested_updates = false;
    bool in_preset = false;
    bool in_user_names = false;
    bool in_sys_names = false;
    bool broken = false;
    uint16_t firmware_version = 0;
    uint8_t dsp[3] {0};
    int data_stream = -1;
    uint64_t midi_count = 0;
    float broken_idle = 0.f;
    
    // heartbeat
    float heart_phase = 0.f;
    float heart_time = 1.0;
    bool tick_tock = true;
    NVGcolor ledColor = green_light;
	//float blinkPhase = 0.f;

    // device management
    int inputDeviceId = -1;
    std::string device_name;

    // cc handling
    uint8_t pedal_fraction = 0;
    int64_t notesOn = 0;
    uint8_t recirculator = 0;
    int download_message_id = -1; // CC109

    midi::Output midiOutput;

    // cv processing
    const int CV_INTERVAL = 64;
    int check_cv = 0;
    const float MIDI_RATE = 0.01f;
    rack::dsp::Timer midi_timer;

    const std::string deviceName() { return device_name; }
    bool isEaganMatrix() { return is_eagan_matrix; }
    bool is_gathering_presets() { return preset_state == InitState::Pending; }

    Hc1Module();

    //midi::Input
    void onMessage(const midi::Message& msg) override;

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

    // ISendMidi
    void sendNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) override;
    void sendNoteOff(uint8_t channel, uint8_t note, uint8_t velocity) override;
    void sendControlChange(uint8_t channel, uint8_t cc, uint8_t value) override;
    void sendProgramChange(uint8_t channel, uint8_t program) override;

    // IPresetHolder
    void setPreset(std::shared_ptr<MinPreset> preset) override;
    bool isCurrentPreset(std::shared_ptr<MinPreset> preset) override
    {
        return preset && (preset->number == preset0.number
            && preset->bank_hi == preset0.bank_hi
            && preset->bank_lo == preset0.bank_lo
            && 0 == preset->name.compare(preset0.name()));
    }

    void sendResetAllreceivers();
    void transmitInitDevice();
    void transmitRequestUpdates();
    void transmitRequestConfiguration();
    void transmitRequestPresets();
    void sendEditorPresent();
    void silence(bool reset);
    //void chooseUserPreset(uint8_t index);
    void beginPreset();
    void handle_ch16_cc(uint8_t cc, uint8_t value);
    void handle_ch16_message(const midi::Message& msg);
    void onNoteOn(uint8_t channel, uint8_t note, uint8_t velocity);
    void onNoteOff(uint8_t channel, uint8_t note, uint8_t velocity);
    void setMacroCCValue(int id, uint8_t value);
    void onSoundOff();
    void onChannel0CC(uint8_t cc, uint8_t value);
    void handle_ch0_message(const midi::Message& msg);
    void processCV(int inputId);
    void processAllCV();
    void process(const ProcessArgs& args) override;
};

struct Hc1ModuleWidget : ModuleWidget
{
    Hc1Module *my_module = nullptr;
    GrayModuleLightWidget * status_light;
    bool have_preset_widgets = false;
    std::vector<Widget*> presets;

    Hc1ModuleWidget(Hc1Module *module);

    void clearPresetWidgets();
    void populatePresetWidgets();

    void step() override;
    void drawLayer(const DrawArgs& args, int layer) override;
    void draw(const DrawArgs& args) override;
};

}
#endif