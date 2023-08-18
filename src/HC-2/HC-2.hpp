#pragma once
#ifndef HC2_HPP_INCLUDED
#define HC2_HPP_INCLUDED
#include <stdint.h>
#include "../plugin.hpp"
#include "../em_midi.hpp"
#include "../presets.hpp"
#include "../HC-1/HC-1.hpp"

#define VERBOSE_LOG
#include "../debug_log.hpp"

using namespace em_midi;
namespace pachde {

struct Hc2Module : Module
{

    Hc1Module* partner = nullptr;
    bool partner_side = false;

    Hc2Module();
    void onExpanderChange(const ExpanderChangeEvent& e) override;

    void refreshPartner();

    // json_t * dataToJson() override;
    // void dataFromJson(json_t *root) override;
    std::string getDeviceName();
    //void process(const ProcessArgs& args) override;
};

struct Hc2ModuleWidget : ModuleWidget
{
    Hc2Module * my_module;

    explicit Hc2ModuleWidget(Hc2Module * module);
    void drawExtenderConnector(const DrawArgs& args);
    void drawCCMap(const DrawArgs& args);
    void draw(const DrawArgs& args) override;
};

}
#endif
