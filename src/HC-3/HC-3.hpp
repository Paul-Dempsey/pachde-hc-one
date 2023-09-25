#pragma once
#ifndef HC3_HPP_INCLUDED
#define HC3_HPP_INCLUDED
#include <stdint.h>
#include "../plugin.hpp"
#include "../HC-1/HC-1.hpp"
#include "../presets.hpp"
#include "../square_button.hpp"

namespace pachde {

struct Hc3Module : Module
{
    enum Params {
        SELECTED_PARAM,
        NUM_PARAMS
    };
    enum Inputs{NUM_INPUTS};
    enum Outputs{NUM_OUTPUTS};
    enum Lights
    {
        ENUMS(SETLIST, 16),
        NUM_LIGHTS
    };

    int loaded_id = 5; //for testing, later -1, meaning (none).
    std::vector<std::string> files;

    Hc3Module();
    void clearFiles();

    json_t *dataToJson() override;
    void dataFromJson(json_t *root) override;
    void onReset() override;
    void process(const ProcessArgs& args) override;
};

struct Hc3ModuleWidget : ModuleWidget
{
    Hc3Module* my_module;
    Hc3ModuleWidget(Hc3Module* module);
    DrawSquareButton drawButton;

    // IHandleHcEvents
    // void onPresetChanged(const PresetChangedEvent& e) override;
    // void onRoundingChanged(const RoundingChangedEvent& e) override;

    //void step() override;
    //void drawExpanderConnector(const DrawArgs& args);
    void draw(const DrawArgs& args) override;
    void appendContextMenu(Menu *menu) override;

};

#endif
}