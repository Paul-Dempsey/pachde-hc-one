#pragma once
#ifndef HC2_HPP_INCLUDED
#define HC2_HPP_INCLUDED
#include <stdint.h>
#include "../plugin.hpp"
#include "../em_midi.hpp"
#include "../presets.hpp"
#include "../HC-1/HC-1.hpp"
#include "../em_types.hpp"

// #define VERBOSE_LOG
// #include "../debug_log.hpp"

using namespace em_midi;
namespace pachde {

struct Hc2Module : Module, ISendMidi
{
    enum Params
    {
        ROUND_RATE_PARAM,    // 0..127 cc25
        ROUND_INITIAL_PARAM, // 0..1 cc 28
        ROUND_KIND_PARAM,    // 0..3
//        ROUND_TUNING_PARAM,  // PackedTuning

        ROUND_RATE_REL_PARAM,
        NUM_PARAMS,
    };
    enum Inputs
    {
        ROUND_RATE_INPUT,
        NUM_INPUTS
    };
    enum Outputs
    {
        NUM_OUTPUTS
    };
    enum Lights
    {
        ROUND_RATE_REL_LIGHT,
        ROUND_INITIAL_LIGHT,
        NUM_LIGHTS
    };
    Rounding rounding;

    ExpanderPresence partner_side = Expansion::None;
    Hc1Module* getPartner();
    // cv processing
    const int CV_INTERVAL = 64;
    int check_cv = 0;
    
    Hc2Module();
    void onExpanderChange(const ExpanderChangeEvent& e) override;

    const char * getDeviceName();
    // json_t * dataToJson() override;
    // void dataFromJson(json_t *root) override;
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
};

struct Hc2ModuleWidget : ModuleWidget
{
    Hc2Module * my_module;

    explicit Hc2ModuleWidget(Hc2Module * module);
    void drawExpanderConnector(const DrawArgs& args);
    void drawCCMap(const DrawArgs& args, Hc1Module * partner);
    void draw(const DrawArgs& args) override;
    void appendContextMenu(Menu *menu) override;
};

}
#endif
