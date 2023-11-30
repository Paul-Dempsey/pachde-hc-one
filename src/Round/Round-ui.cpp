#include "Round.hpp"
#include "../widgets/port.hpp"
#include "../widgets/cc_param.hpp"
#include "../widgets/switch_4.hpp"
#include "../colors.hpp"
#include "../misc.hpp"
#include "../text.hpp"
#include "tuning_ui.hpp"

namespace pachde {

using RP = RoundModule::Params;
using RI = RoundModule::Inputs;
using RL = RoundModule::Lights;

constexpr const float KNOB_RADIUS = 12.f;

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

    float center = box.size.x * .5f;

    // Type
    float y = 60.f;
    addParam(createParamCentered<SwitchHorz4>(
        Vec(center, y),
        module, RP::P_ROUND_KIND));
    y += 10.f;
    addChild(type_text = createLazyDynamicTextLabel(
        Vec(center, y),
        Vec(100.f, 12.f),
        [=]() { return describeRoundKind(static_cast<RoundKind>(GetSmallParamValue(this, RP::P_ROUND_KIND))); },
        9.f, false, TextAlignment::Center,GetStockColor(StockColor::Gold), false));

    // Tuning
    y = 130.f;
    auto p = createParamCentered<TuningKnob>(Vec(center, y), module, RP::P_ROUND_TUNING);
    p->setImage();
    addChild(p);
    y += KNOB_RADIUS + 4.f;
    addChild(tuning_text = createLazyDynamicTextLabel(
        Vec(center, y),
        Vec(100.f, 12.f),
        [=]() {
            PackedTuning tuning = static_cast<PackedTuning>(GetSmallParamValue(this, RP::P_ROUND_TUNING));
            return describeTuning(unpackTuning(tuning));
        },
        9.f, false, TextAlignment::Center,GetStockColor(StockColor::Gold), false));

    // Initial
    y = 190.f;
    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<BlueLight>>>(
        Vec(center, y),
        module, RP::P_ROUND_INITIAL, RL::L_ROUND_INITIAL));
    y += 16.f;
    addChild(createInputCentered<ColorPort>(
        Vec(center, y),
        module, RI::IN_ROUND_INITIAL));

    // Rate
    y = 265.f;
    addChild(createModKnob(
        Vec( center, y), 
        module, RP::P_ROUND_RATE, RI::IN_ROUND_RATE, RP::P_ROUND_RATE_REL));
    y += 18.f;
    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<BlueLight>>>(
        Vec(center - 14.f, y),
        module, RP::P_ROUND_RATE_REL, RL::L_ROUND_RATE_REL));
    y += KNOB_RADIUS;
    addChild(createInputCentered<ColorPort>(
        Vec( center, y),
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