#include "Pedals.hpp"
#include "../widgets/pedal_param.hpp"
#include "../widgets/port.hpp"
#include "../widgets/small_push.hpp"

namespace pachde {

SymbolWidget::Symbol pSymbolForPedal(PedalType pedal)
{
    switch (pedal) {
    case PedalType::NoPedal: return Symbol::NoPedal;
    case PedalType::SwitchPedal: return Symbol::SwitchPedal;
    case PedalType::ExpressionPedal: return Symbol::ExpressionPedal;
    case PedalType::DamperPedal: return Symbol::DamperPedal;
    case PedalType::TriValuePedal: return Symbol::TriValuePedal;
    case PedalType::CVPedal: return Symbol::CVPedal;
    case PedalType::PotPedal: return Symbol::PotPedal;
    default:
        return Symbol::OtherPedal;
    }
}

// -------------------------------------------------------------

PedalUICore::PedalUICore(PedalCore* module)
{
    core_module = module;
    setModule(module);
    if (core_module) {
        core_module->ui_event_sink = this;
    }
}

void PedalUICore::onDeviceChanged(const DeviceChangedEvent& e)
{
    device_label->text(e.name);
}

void PedalUICore::onPresetChanged(const PresetChangedEvent& e)
{
    if (!core_module) { return; }
    static_cast<PedalParamQuantity*>(core_module->getParamQuantity(PedalCore::P_PEDAL_ASSIGN))->setEnabled(true);
}

void PedalUICore::onPedalChanged(const PedalChangedEvent& e)
{
    pedal_type->setSymbol(pSymbolForPedal(e.pedal.type)); 
    pedal_assign->text(LongPedalAssignment(e.pedal.cc));
    pedal_amount->text(format_string("%d", e.pedal.value));
}

void PedalUICore::onDisconnect(const DisconnectEvent& e)
{
    device_label->text("");
}

constexpr const float JUMP_SPREAD = 50.25f;

void PedalUICore::createUI()
{
    // device name
    addChild(device_label = createStaticTextLabel<StaticTextLabel>(
        Vec(3.25f, 14.f), 180.f, "", TextAlignment::Left, 10.f, false, GetStockColor(StockColor::pachde_blue_medium)));

    /// pedal function knob
    float x_center = box.size.x * .5f;
    addChild(pedal_assign = createStaticTextLabel<StaticTextLabel>(Vec(x_center, 37.f), 60.f, "Sustain", TextAlignment::Center, 10.f, true));
    addChild(createParamCentered<PedalKnob>(Vec(x_center, 64.f), module, PedalCore::P_PEDAL_ASSIGN));

    // value slider
    addChild(slider = createParamCentered<VerticalSlider>(Vec(x_center, 190.f), core_module, PedalCore::P_PEDAL_VALUE));

    float y = 90;
    // jump buttons
    {
        float x = box.size.x - 12.f;
        auto pb = createWidgetCentered<SmallPush>(Vec(x, y));
        if (core_module) {
            pb->describe("Jump to Max");
            pb->onClick([=](bool ctrl, bool shift) {
                auto pq = core_module->getParamQuantity(PedalCore::P_PEDAL_VALUE);
                if (pq) {
                    pq->setValue(pq->getMaxValue());
                }
            });
        }
        addChild(pb);
        y += JUMP_SPREAD;
        pb = createWidgetCentered<SmallPush>(Vec(x, y));
        if (core_module) {
            pb->describe("Jump to 75%");
            pb->onClick([=](bool ctrl, bool shift) {
                auto pq = core_module->getParamQuantity(PedalCore::P_PEDAL_VALUE);
                if (pq) {
                    pq->setValue(pq->getMinValue() + (pq->getMaxValue()-pq->getMinValue())*.75f);
                }
            });
        }
        addChild(pb);
        y += JUMP_SPREAD;
        pb = createWidgetCentered<SmallPush>(Vec(x, y));
        if (core_module) {
            pb->describe("Jump to 50%");
            pb->onClick([=](bool ctrl, bool shift) {
                auto pq = core_module->getParamQuantity(PedalCore::P_PEDAL_VALUE);
                if (pq) {
                    pq->setValue(pq->getMinValue() + (pq->getMaxValue()-pq->getMinValue())*.5f);
                }
            });
        }
        addChild(pb);
        y += 46.5;
        pb = createWidgetCentered<SmallPush>(Vec(x, y));
        if (core_module) {
            pb->describe("Jump to 25%");
            pb->onClick([=](bool ctrl, bool shift) {
                auto pq = core_module->getParamQuantity(PedalCore::P_PEDAL_VALUE);
                if (pq) {
                    pq->setValue(pq->getMinValue() + (pq->getMaxValue()-pq->getMinValue())*.25f);
                }
            });
        }
        addChild(pb);
        y += JUMP_SPREAD;
        pb = createWidgetCentered<SmallPush>(Vec(x, y));
        if (core_module) {
            pb->describe("Jump to Min");
            pb->onClick([=](bool ctrl, bool shift) {
                auto pq = core_module->getParamQuantity(PedalCore::P_PEDAL_VALUE);
                if (pq) {
                    pq->setValue(pq->getMinValue());
                }
            });
        }
        addChild(pb);
    }
    y += 5.f;
    addChild(pedal_amount = createStaticTextLabel<StaticTextLabel>(
        Vec(x_center, y), box.size.x - 15.f, "0", TextAlignment::Center, 12.f, false));

    // Pedal type image
    addChild(pedal_type = createSymbolWidget<SymbolTipWidget>(20.f, 312.f, Symbol::NoPedal));

    // I/O Ports
    y = box.size.y - 27.f;
    addChild(createColorInputCentered(x_center - 15.f, y, PedalCore::I_PEDAL_VALUE, RampGray(G_45), module));
    addChild(createColorOutputCentered(x_center + 15.f, y, PedalCore::O_PEDAL_VALUE, GetStockColor(StockColor::pachde_default_port), module));
}

//
// =============================================================
//

Pedal1UI::Pedal1UI(Pedal1Module* module)
: PedalUICore(module)
{
    pedal_id = 0;
    setPanel(createPanel(asset::plugin(pluginInstance, "res/Pedal1.svg")));
    createUI();
}

Pedal2UI::Pedal2UI(Pedal2Module* module)
: PedalUICore(module)
{
    pedal_id = 1;
    setPanel(createPanel(asset::plugin(pluginInstance, "res/Pedal2.svg")));
    createUI();
}

}