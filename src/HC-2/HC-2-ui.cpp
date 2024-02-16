// Copyright (C) Paul Chase Dempsey
#include "HC-2.hpp"
#include "HC-2-layout.hpp"
#include "header_widget.hpp"
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

using Hc2P = Hc2Module::Params;
using Hc2I = Hc2Module::Inputs;
using Hc2O = Hc2Module::Outputs;
using Hc2L = Hc2Module::Lights;

inline uint8_t GetSmallParamValue(rack::app::ModuleWidget* w, int id, uint8_t default_value = 0) {
    auto p = w->getParam(id);
    if (!p) return default_value;
    auto pq = p->getParamQuantity();
    if (!pq) return default_value;
    return U8(pq->getValue());
}

Hc2ModuleWidget::Hc2ModuleWidget(Hc2Module * module)
{
    my_module = module;
    setModule(module);
    if (my_module) {
        my_module->ui_event_sink = this;
    }
    setPanel(createPanel(asset::plugin(pluginInstance, "res/HC-2.svg")));
    addChild(partner_picker = createPartnerPicker());

    auto x = box.size.x * .5f;
    addChild(createCCMap<CCMap>(x, box.size.y - 24.f - 20.f, true, CCMapChannel::One, this));
    addChild(createCCMap<CCMap>(x, box.size.y - 24.f, true, CCMapChannel::Sixteen, this));
}

void Hc2ModuleWidget::onDeviceChanged(const DeviceChangedEvent& e)
{
    partner_picker->onDeviceChanged(e);
}

void Hc2ModuleWidget::onDisconnect(const DisconnectEvent& e)
{
    partner_picker->onDisconnect(e);
}

Hc1Module* Hc2ModuleWidget::getPartner()
{
    if (!my_module) return nullptr;
    return my_module->getPartner();
}

// void Hc2ModuleWidget::draw(const DrawArgs& args)
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

void Hc2ModuleWidget::appendContextMenu(Menu *menu)
{
    if (!my_module) return;
    auto partner = getPartner();
    menu->addChild(new MenuSeparator);
    if (partner) {
        menu->addChild(createMenuItem("Clear CC Map", "",
            [partner](){ partner->clearCCValues(); }));
    } else {
        menu->addChild(createMenuItem("- Not Connected - ", "", [](){}, true));
    }
    my_module->partner_binding.appendContextMenu(menu);
}

}