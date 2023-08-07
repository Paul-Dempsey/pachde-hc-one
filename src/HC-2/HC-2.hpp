#pragma once
#ifndef HC2_HPP_INCLUDED
#define HC2_HPP_INCLUDED
#include <stdint.h>
#include "../plugin.hpp"
#include "../em_midi.hpp"
#include "../presets.hpp"

#define VERBOSE_LOG
#include "../debug_log.hpp"

using namespace em_midi;
namespace pachde {

struct Hc2Module : Module, ISendMidi, midi::Input
{
    midi::Output midiOutput;
    std::string device_name;
    Preset preset0;
    std::vector< std::shared_ptr<MinPreset> > presets;
    std::vector< std::shared_ptr<MinPreset> > user_presets;
    uint16_t firmware_version = 0;
    uint64_t midi_count = 0;

    bool tick_tock = true;
    bool is_eagan_matrix = false;

    Hc2Module();
    void onReset() override
    {
        preset0.clear();
        presets.clear();
        user_presets.clear();
        is_eagan_matrix = false;
        firmware_version = 0;
        midi_count = 0;
        midiOutput.reset();
        Input::reset();
        findEM();
    }
    // json_t * dataToJson() override;
    // void dataFromJson(json_t *root) override;

    void process(const ProcessArgs& args) override;

    //midi::Input
    void onMessage(const midi::Message& message) override;

    // ISendMidi
    void sendNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) override;
    void sendNoteOff(uint8_t channel, uint8_t note, uint8_t velocity) override;
    void sendControlChange(uint8_t channel, uint8_t cc, uint8_t value) override;
    void sendProgramChange(uint8_t channel, uint8_t program) override;

    void findEMOut();
    void findEM();

};

struct Hc2ModuleWidget : ModuleWidget
{
    Hc2Module * my_module;
    std::vector<Widget*> presets;

    Hc2ModuleWidget(Hc2Module * module);
    void draw(const DrawArgs& args) override;
};

}
#endif
