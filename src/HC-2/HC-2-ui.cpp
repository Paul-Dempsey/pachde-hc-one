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

void Hc2ModuleWidget::createTiltEqUI(float x, float y)
{
    addChild(createHeaderWidget(x, y, TEQ_BOX_WIDTH, KNOB_BOX_HEIGHT));
    addChild(createStaticTextLabel<StaticTextLabel>(Vec(x + TEQ_BOX_WIDTH*.5f, y + PAD), 60.f, "Tilt EQ", TextAlignment::Center));
    addChild(createLightCentered<SmallSimpleLight<BlueLight>>(Vec(x + KNOB_COL1 + 2.f * KNOB_SPREAD, y + 8.f), my_module, Hc2L::L_TEQ));

    float cx = x + KNOB_COL1;
    addChild(createStaticTextLabel<StaticTextLabel>(Vec(cx, y + KNOB_ROW - STATIC_LABEL_OFFSET), 50.f, "Tilt", TextAlignment::Center));
    addChild(createModKnob(
        Vec( cx, y + KNOB_ROW), 
        module, Hc2P::P_TEQ_TILT, Hc2I::IN_TEQ_TILT, Hc2P::P_TEQ_TILT_REL));
    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<BlueLight>>>(Vec(cx - REL_OFFSET, y + KNOB_ROW - REL_VOFFSET), my_module, Hc2P::P_TEQ_TILT_REL, Hc2L::L_TEQ_TILT_REL));
    addChild(createInputCentered<ColorPort>(Vec(cx - CV_COLUMN_OFFSET, y + KNOB_ROW + CV_ROW_OFFSET), my_module, Hc2I::IN_TEQ_TILT));

    cx += KNOB_SPREAD;
    addChild(createStaticTextLabel<StaticTextLabel>(Vec(cx, y + KNOB_ROW - STATIC_LABEL_OFFSET), 55.f, "Frequency", TextAlignment::Center));
    addChild(createModKnob(
        Vec( cx, y + KNOB_ROW), 
        module, Hc2P::P_TEQ_FREQ, Hc2I::IN_TEQ_FREQ, Hc2P::P_TEQ_FREQ_REL));
    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<BlueLight>>>(Vec(cx - REL_OFFSET, y + KNOB_ROW - REL_VOFFSET), my_module, Hc2P::P_TEQ_FREQ_REL, Hc2L::L_TEQ_FREQ_REL));
    addChild(createInputCentered<ColorPort>(Vec(cx - CV_COLUMN_OFFSET, y + KNOB_ROW + CV_ROW_OFFSET), my_module, Hc2I::IN_TEQ_FREQ));

    cx += KNOB_SPREAD;
    addChild(createStaticTextLabel<StaticTextLabel>(Vec(cx, y + KNOB_ROW - STATIC_LABEL_OFFSET), 30.f, "Mix", TextAlignment::Center));
    addChild(createModKnob(
        Vec( cx, y + KNOB_ROW), 
        module, Hc2P::P_TEQ_MIX, Hc2I::IN_TEQ_MIX, Hc2P::P_TEQ_MIX_REL));
    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<BlueLight>>>(Vec(cx - REL_OFFSET, y + KNOB_ROW - REL_VOFFSET), my_module, Hc2P::P_TEQ_MIX_REL, Hc2L::L_TEQ_MIX_REL));
    addChild(createInputCentered<ColorPort>(Vec(cx - CV_COLUMN_OFFSET, y + KNOB_ROW + CV_ROW_OFFSET), my_module, Hc2I::IN_TEQ_MIX));

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

    createTiltEqUI(TEQ_BOX_LEFT, TEQ_BOX_TOP);

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
    auto partner = getPartner();
    menu->addChild(new MenuSeparator);
    if (partner) {
        menu->addChild(createMenuItem("Clear CC Map", "",
            [partner](){ partner->clearCCValues(); }));
    } else {
        menu->addChild(createMenuItem("- Not Connected - ", "", [](){}, true));
    }
}

}