#include "Compress.hpp"
#include "Compress-layout.hpp"
#include "../colors.hpp"
#include "../misc.hpp"
#include "../text.hpp"
#include "../widgets/cc_param.hpp"
#include "../widgets/components.hpp"
#include "../widgets/port.hpp"
#include "../widgets/small_push.hpp"
#include "../widgets/switch_4.hpp"

namespace pachde {

#define SHOW_RCC_REFERENCE // show reference lines at key cc values

using CmpP = CompressModule::Params;
using CmpI = CompressModule::Inputs;
using CmpO = CompressModule::Outputs;
using CmpL = CompressModule::Lights;

inline uint8_t GetSmallParamValue(rack::app::ModuleWidget* w, int id, uint8_t default_value = 0) {
    auto p = w->getParam(id);
    if (!p) return default_value;
    auto pq = p->getParamQuantity();
    if (!pq) return default_value;
    return U8(pq->getValue());
}

void CompressModuleWidget::createCompressorUI()
{
    auto center = box.size.x * .5f;
    addChild(createLightCentered<SmallSimpleLight<BlueLight>>(Vec(center, 40), my_module, CmpL::L_COMPRESSOR));

    float y = 92.f;
    // Threshold
    addChild(createModKnob(
        Vec(center, y), 
        module, CmpP::P_COMP_THRESHOLD, CmpI::IN_COMP_THRESHOLD, CmpP::P_COMP_THRESHOLD_REL));
    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<BlueLight>>>(Vec(center - REL_OFFSET, y + REL_VOFFSET), my_module, CmpP::P_COMP_THRESHOLD_REL, CmpL::L_COMP_THRESHOLD_REL));
    addChild(createInputCentered<ColorPort>(Vec(center, y + CV_VOFFSET), my_module, CmpI::IN_COMP_THRESHOLD));

    // Attack
    y += KNOB_SPREAD;
    addChild(createModKnob(
        Vec(center, y), 
        module, CmpP::P_COMP_ATTACK, CmpI::IN_COMP_ATTACK, CmpP::P_COMP_ATTACK_REL));
    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<BlueLight>>>(Vec(center - REL_OFFSET, y + REL_VOFFSET), my_module, CmpP::P_COMP_ATTACK_REL, CmpL::L_COMP_ATTACK_REL));
    addChild(createInputCentered<ColorPort>(Vec(center, y + CV_VOFFSET), my_module, CmpI::IN_COMP_ATTACK));

    // Ratio
    y += KNOB_SPREAD;
    addChild(createModKnob(
        Vec(center, y), 
        module, CmpP::P_COMP_RATIO, CmpI::IN_COMP_RATIO, CmpP::P_COMP_RATIO_REL));
    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<BlueLight>>>(Vec(center - REL_OFFSET, y + REL_VOFFSET), my_module, CmpP::P_COMP_RATIO_REL, CmpL::L_COMP_RATIO_REL));
    addChild(createInputCentered<ColorPort>(Vec(center, y + CV_VOFFSET), my_module, CmpI::IN_COMP_RATIO));

    // Mix
    y += KNOB_SPREAD;
    addChild(createModKnob(
        Vec(center, y), 
        module, CmpP::P_COMP_MIX, CmpI::IN_COMP_MIX, CmpP::P_COMP_MIX_REL));
    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<BlueLight>>>(Vec(center - REL_OFFSET, y + REL_VOFFSET), my_module, CmpP::P_COMP_MIX_REL, CmpL::L_COMP_MIX_REL));
    addChild(createInputCentered<ColorPort>(Vec(center, y + CV_VOFFSET), my_module, CmpI::IN_COMP_MIX));
}


CompressModuleWidget::CompressModuleWidget(CompressModule * module)
{
    my_module = module;
    setModule(module);
    if (my_module) {
        my_module->ui_event_sink = this;
    }
    setPanel(createPanel(asset::plugin(pluginInstance, "res/Compress.svg")));
    addChild(partner_picker = createPartnerPicker(7.f, 14.f, 180.f, module ? &module->partner_binding : nullptr));

    createCompressorUI();
}

void CompressModuleWidget::onDeviceChanged(const DeviceChangedEvent& e)
{
    partner_picker->onDeviceChanged(e);
}

void CompressModuleWidget::onDisconnect(const DisconnectEvent& e)
{
    partner_picker->onDisconnect(e);
}

Hc1Module* CompressModuleWidget::getPartner()
{
    if (!my_module) return nullptr;
    return my_module->getPartner();
}

// void CompressModuleWidget::draw(const DrawArgs& args)
// {
//     ModuleWidget::draw(args);

//     //auto partner = getPartner();
//     //if (partner) {
//         // system_data
//         // auto font = GetPluginFontRegular();
//         // SetTextStyle(args.vg, font, RampGray(G_90), 10.f);
//         // std::string data;
//         // if (partner->system_data.empty()) {
//         //     data = "(empty)";
//         // } else {
//         //     for (auto b:partner->system_data) {
//         //         auto hex = format_string("%02x", b);
//         //         data.append(hex);
//         //     }
//         // }
//         // nvgText(args.vg, 7.5, box.size.y - 64, data.c_str(), nullptr);
//     //}

// }

void CompressModuleWidget::appendContextMenu(Menu *menu)
{
    if (!my_module) { return; }
    menu->addChild(new MenuSeparator);
    menu->addChild(createMenuItem("Absolute CV", "", [=](){ my_module->absoluteCV(); }));
    menu->addChild(createMenuItem("Relative CV", "", [=](){ my_module->relativeCV(); }));
}

}