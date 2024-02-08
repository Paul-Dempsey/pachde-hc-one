#include "Tilt.hpp"
#include "../colors.hpp"
#include "../misc.hpp"
#include "../text.hpp"
#include "../widgets/cc_param.hpp"
#include "../widgets/components.hpp"
#include "../widgets/port.hpp"
#include "../widgets/small_push.hpp"
#include "../widgets/switch_4.hpp"

namespace pachde {

using TP = TiltModule::Params;
using TI = TiltModule::Inputs;
using TO = TiltModule::Outputs;
using TL = TiltModule::Lights;

constexpr const float KNOB_SPREAD = 80.f;
constexpr const float CENTER = 22.5f;
constexpr const float PAD = 1.f;
constexpr const float MORE_PAD = 4.f;
constexpr const float REL_OFFSET = 20.f;
constexpr const float REL_VOFFSET = 10.f;
constexpr const float CV_COLUMN_OFFSET = 24.f;
constexpr const float CV_ROW_OFFSET = 6.f;
constexpr const float STATIC_LABEL_OFFSET = 29.5f;

TiltModuleWidget::TiltModuleWidget(TiltModule * module)
: my_module(module)
{
    setModule(module);
    if (module) {
        my_module->ui_event_sink = this;
    }
    setPanel(createPanel(asset::plugin(pluginInstance, "res/Tilt.svg")));
    addChild(partner_picker = createPartnerPicker());

    addChild(createLightCentered<SmallSimpleLight<BlueLight>>(Vec(CENTER, 40.f), my_module, TL::L_TEQ));

    float x_rel = CENTER - VK_REL_VOFFSET;
    float y = 88.f;

    addChild(createModKnob(
        Vec(CENTER, y), 
        module, TP::P_TEQ_TILT, TI::IN_TEQ_TILT, TP::P_TEQ_TILT_REL));
    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<BlueLight>>>(Vec(x_rel, y + VK_REL_VOFFSET), my_module, TP::P_TEQ_TILT_REL, TL::L_TEQ_TILT_REL));
    addChild(createInputCentered<ColorPort>(Vec(CENTER, y + VK_CV_VOFFSET), my_module, TI::IN_TEQ_TILT));

    y += KNOB_SPREAD;
    addChild(createModKnob(
        Vec(CENTER, y), 
        module, TP::P_TEQ_FREQ, TI::IN_TEQ_FREQ, TP::P_TEQ_FREQ_REL));
    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<BlueLight>>>(Vec(x_rel, y + VK_REL_VOFFSET), my_module, TP::P_TEQ_FREQ_REL, TL::L_TEQ_FREQ_REL));
    addChild(createInputCentered<ColorPort>(Vec(CENTER, y + VK_CV_VOFFSET), my_module, TI::IN_TEQ_FREQ));

    y += KNOB_SPREAD;
    addChild(createModKnob(
        Vec(CENTER, y), 
        module, TP::P_TEQ_MIX, TI::IN_TEQ_MIX, TP::P_TEQ_MIX_REL));
    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<BlueLight>>>(Vec(x_rel, y + VK_REL_VOFFSET), my_module, TP::P_TEQ_MIX_REL, TL::L_TEQ_MIX_REL));
    addChild(createInputCentered<ColorPort>(Vec(CENTER, y + VK_CV_VOFFSET), my_module, TI::IN_TEQ_MIX));

}

Hc1Module* TiltModuleWidget::getPartner()
{
    return module ? my_module->getPartner() : nullptr;
}

void TiltModuleWidget::onDeviceChanged(const DeviceChangedEvent& e)
{
    partner_picker->onDeviceChanged(e);
}

void TiltModuleWidget::onDisconnect(const DisconnectEvent& e)
{
    partner_picker->onDisconnect(e);
}

void TiltModuleWidget::appendContextMenu(Menu *menu)
{
    if (!my_module) return;
    menu->addChild(new MenuSeparator);
    my_module->partner_binding.appendContextMenu(menu);
}

}