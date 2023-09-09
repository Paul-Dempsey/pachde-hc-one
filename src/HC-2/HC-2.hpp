#pragma once
#ifndef HC2_HPP_INCLUDED
#define HC2_HPP_INCLUDED
#include <stdint.h>
#include "../plugin.hpp"
#include "../em_midi.hpp"
#include "../presets.hpp"
#include "../HC-1/HC-1.hpp"

// #define VERBOSE_LOG
// #include "../debug_log.hpp"

using namespace em_midi;
namespace pachde {

struct Hc2Module : Module
{
    ExpanderPresence partner_side = Expansion::None;
    Hc1Module* getPartner();
    
    Hc2Module();
    void onExpanderChange(const ExpanderChangeEvent& e) override;

    const char * getDeviceName();
    // json_t * dataToJson() override;
    // void dataFromJson(json_t *root) override;
    //void process(const ProcessArgs& args) override;
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
