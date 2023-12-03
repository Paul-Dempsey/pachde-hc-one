#include "Round.hpp"
#include "../widgets/port.hpp"
#include "../widgets/cc_param.hpp"
#include "../widgets/switch_4.hpp"
#include "../colors.hpp"
#include "../knob_layout.hpp"
#include "../misc.hpp"
#include "../text.hpp"
#include "tuning_ui.hpp"

namespace pachde {

using RP = RoundModule::Params;
using RI = RoundModule::Inputs;
using RL = RoundModule::Lights;

constexpr const float KNOB_SPREAD = 80.f;
constexpr const float LIGHT_SPREAD = 4.f;
constexpr const float CENTER = 22.5f;

inline uint8_t GetSmallParamValue(rack::app::ModuleWidget* w, int id, uint8_t default_value = 0) {
    auto p = w->getParam(id);
    if (!p) return default_value;
    auto pq = p->getParamQuantity();
    if (!pq) return default_value;
    return U8(pq->getValue());
}

RoundModuleWidget::RoundModuleWidget(RoundModule * module)
: my_module(module)
{
    setModule(module);
    if (module) {
        my_module->ui_event_sink = this;
    }
    setPanel(createPanel(asset::plugin(pluginInstance, "res/Round.svg")));
    addChild(partner_picker = createPartnerPicker(7.f, 14.f, 180.f, module ? &module->partner_binding : nullptr));

    { // Rounding LEDs
        float x = CENTER - (2.f * LIGHT_SPREAD) + 1;
        addChild(createLight<TinySimpleLight<RedLight>>(Vec(x, 40.f), my_module, RL::ROUND_Y_LIGHT));
        x += LIGHT_SPREAD;
        addChild(createLight<TinySimpleLight<RedLight>>(Vec(x, 40.f), my_module, RL::ROUND_INITIAL_LIGHT));
        x += LIGHT_SPREAD;
        addChild(createLight<TinySimpleLight<RedLight>>(Vec(x, 40.f), my_module, RL::ROUND_LIGHT));
        x += LIGHT_SPREAD;
        addChild(createLight<TinySimpleLight<RedLight>>(Vec(x, 40.f), my_module, RL::ROUND_RELEASE_LIGHT));
    }

    // Type
    float y = 86.f;
    addParam(createParamCentered<SwitchHorz4>(
        Vec(CENTER, y),
        module, RP::P_ROUND_KIND));
    addChild(type_text = createLazyDynamicTextLabel(
        Vec(CENTER, y + 12.f),
        Vec(100.f, 12.f),
        [=]() { return describeRoundKind(static_cast<RoundKind>(GetSmallParamValue(this, RP::P_ROUND_KIND))); },
        9.f, false, TextAlignment::Center,GetStockColor(StockColor::Gold), false));

    // Tuning
    y = 88.f + KNOB_SPREAD;
    auto p = createParamCentered<TuningKnob>(Vec(CENTER, y), module, RP::P_ROUND_TUNING);
    p->setImage();
    addChild(p);
    addChild(tuning_text = createLazyDynamicTextLabel(
        Vec(CENTER, y + KNOB_RADIUS + 4.f),
        Vec(100.f, 12.f),
        [=]() {
            PackedTuning tuning = static_cast<PackedTuning>(GetSmallParamValue(this, RP::P_ROUND_TUNING));
            return describeTuning(unpackTuning(tuning));
        },
        9.f, false, TextAlignment::Center,GetStockColor(StockColor::Gold), false));

    // Initial
    y += KNOB_SPREAD;
    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<BlueLight>>>(
        Vec(CENTER, y),
        module, RP::P_ROUND_INITIAL, RL::L_ROUND_INITIAL));
    addChild(createInputCentered<ColorPort>(
        Vec(CENTER, y + 15.f),
        module, RI::IN_ROUND_INITIAL));

    // Rate
    y += KNOB_SPREAD;
    addChild(createModKnob(
        Vec(CENTER, y), 
        module, RP::P_ROUND_RATE, RI::IN_ROUND_RATE, RP::P_ROUND_RATE_REL));
    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<BlueLight>>>(
        Vec(CENTER - VK_REL_OFFSET, y + VK_REL_VOFFSET),
        module, RP::P_ROUND_RATE_REL, RL::L_ROUND_RATE_REL));
    addChild(createInputCentered<ColorPort>(
        Vec(CENTER, y + VK_CV_VOFFSET),
        module, RI::IN_ROUND_RATE));

}

Hc1Module* RoundModuleWidget::getPartner()
{
    if (!module) return nullptr;
    return my_module->getPartner();
}


void RoundModuleWidget::onPresetChanged(const PresetChangedEvent& e)
{
    tuning_text->modified();
    type_text->modified();
}

void RoundModuleWidget::onRoundingChanged(const RoundingChangedEvent &e)
{
    tuning_text->modified();
    type_text->modified();
}

void RoundModuleWidget::onDeviceChanged(const DeviceChangedEvent &e)
{
    partner_picker->onDeviceChanged(e);
}

void RoundModuleWidget::onDisconnect(const DisconnectEvent& e)
{
    partner_picker->onDisconnect(e);
}


}