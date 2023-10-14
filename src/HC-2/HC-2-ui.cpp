#include "HC-2.hpp"
#include "../colors.hpp"
#include "../misc.hpp"
#include "../text.hpp"
#include "../widgets/cc_param.hpp"
#include "../widgets/components.hpp"
#include "../widgets/pedal_param.hpp"
#include "../widgets/port.hpp"
#include "../widgets/small_push.hpp"
#include "../widgets/switch_4.hpp"
#include "tuning_ui.hpp"

namespace pachde {

#define SHOW_RCC_REFERENCE // show reference lines at key cc values

using Hc2P = Hc2Module::Params;
using Hc2I = Hc2Module::Inputs;
using Hc2O = Hc2Module::Outputs;
using Hc2L = Hc2Module::Lights;

constexpr const float PAD = 2.f;
constexpr const float MORE_PAD = 4.f;
constexpr const float ROUND_BOX_TOP = 35.f;
constexpr const float ROUND_BOX_LEFT = 7.5f;
constexpr const float ROUND_BOX_WIDTH = 105.f;
constexpr const float ROUND_BOX_HEIGHT = 60.f;
constexpr const float ROUND_BOX_HALF = 110.f * .5f;
constexpr const float KNOB_RADIUS = 12.f;
constexpr const float HALF_KNOB = KNOB_RADIUS *.5f;
constexpr const float REL_OFFSET = 20.f;
constexpr const float REL_VOFFSET = 10.f;
constexpr const float CV_COLUMN_OFFSET = 24.f;
constexpr const float ROUND_KNOB_ROW = 32.5f;
constexpr const float ROUND_COL1 = ROUND_BOX_HALF - KNOB_RADIUS - 3.f * PAD;
constexpr const float ROUND_COL2 = ROUND_BOX_HALF + KNOB_RADIUS + 2.f * PAD;
constexpr const float ROUND_COL3 = ROUND_BOX_WIDTH - KNOB_RADIUS + PAD;

constexpr const float PEDAL_BOX_TOP = 35.f;
constexpr const float PEDAL_BOX_LEFT = ROUND_BOX_LEFT + ROUND_BOX_WIDTH + 7.5f;
constexpr const float PEDAL_BOX_WIDTH = 125.f;
constexpr const float PEDAL_BOX_HEIGHT =ROUND_BOX_HEIGHT;

inline uint8_t GetSmallParamValue(rack::app::ModuleWidget* w, int id, uint8_t default_value = 0) {
    auto p = w->getParam(id);
    if (!p) return default_value;
    auto pq = p->getParamQuantity();
    if (!pq) return default_value;
    return U8(pq->getValue());
}

void Hc2ModuleWidget::createRoundingUI(float x, float y)
{
    // Rounding cluster
    addChild(createStaticTextLabel<StaticTextLabel>(Vec(x + MORE_PAD, y + PAD), 40.f, "Rounding", TextAlignment::Left));

    addParam(createParamCentered<SwitchHorz4>(
        Vec(x + ROUND_COL2, y + MORE_PAD + HALF_KNOB),
        module, Hc2P::P_ROUND_KIND));
 
    addChild(createModKnob(
        Vec( x + ROUND_COL1, y + ROUND_KNOB_ROW), 
        module, Hc2P::P_ROUND_RATE, Hc2I::IN_ROUND_RATE, Hc2P::P_ROUND_RATE_REL));
    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<BlueLight>>>(
        Vec( x + ROUND_COL1 - REL_OFFSET, y + ROUND_KNOB_ROW - REL_VOFFSET),
        module, Hc2P::P_ROUND_RATE_REL, Hc2L::L_ROUND_RATE_REL));
    addChild(createInputCentered<ColorPort>(
        Vec( x + ROUND_COL1 - CV_COLUMN_OFFSET, y + ROUND_KNOB_ROW + HALF_KNOB),
        module, Hc2I::IN_ROUND_RATE));

    auto p = createParamCentered<TuningKnob>(Vec( x + ROUND_COL2, y + ROUND_KNOB_ROW), module, Hc2P::P_ROUND_TUNING);
    p->setImage();
    addChild(p);

    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<BlueLight>>>(
        Vec( x + ROUND_COL3, y + ROUND_KNOB_ROW - REL_VOFFSET),
        module, Hc2P::P_ROUND_INITIAL, Hc2L::L_ROUND_INITIAL));
    addChild(createInputCentered<ColorPort>(
        Vec( x + ROUND_COL3, y + ROUND_KNOB_ROW + HALF_KNOB),
        module, Hc2I::IN_ROUND_INITIAL));

    rounding_summary = createLazyDynamicTextLabel(
        Vec(x + ROUND_BOX_HALF, y + ROUND_BOX_HEIGHT - 11.f),
        Vec(100.f, 12.f),
        [=]() {
            bool initial = GetSmallParamValue(this, Hc2P::P_ROUND_INITIAL);
            auto rk = describeRoundKindShort(static_cast<RoundKind>(GetSmallParamValue(this, Hc2P::P_ROUND_KIND)));
            auto rr = GetSmallParamValue(this,Hc2P::P_ROUND_RATE);
            PackedTuning tuning = static_cast<PackedTuning>(GetSmallParamValue(this, Hc2P::P_ROUND_TUNING));
            auto ts = describeTuning(unpackTuning(tuning));
            return format_string("%s %s %d %s",
                rk.c_str(), (initial ? "I" : "\u2022"), rr, ts.c_str());
        },
        9.f, false, TextAlignment::Center,
        GetStockColor(StockColor::Gold), true)
        ;
    addChild(rounding_summary);
}

void Hc2ModuleWidget::createPedalUI(float x, float y)
{
    y += PAD;
    addChild(createStaticTextLabel<StaticTextLabel>(Vec(x + 30.f, y), 30.f, "Pedal 1", TextAlignment::Center));
    addChild(createStaticTextLabel<StaticTextLabel>(Vec(x + 92.f, y), 30.f, "Pedal 2", TextAlignment::Center));

    x += 7.5;
    y += 16.f;
    addChild(pedal1_type = createSymbolWidget<SymbolTipWidget>(x, y + 5.5f, Symbol::NoPedal));

    x += 35.f;
    addChild(createParamCentered<PedalKnob>(Vec(x, y + 10.f), module, Hc2P::P_PEDAL1));
    addChild(pedal1_assign = createStaticTextLabel<StaticTextLabel>(Vec(x, y + 24.f), 60.f, "Sustain", TextAlignment::Center, 10.f, true));

    x += 25.f;
    addChild(pedal2_type = createSymbolWidget<SymbolTipWidget>(x, y + 5.5f, Symbol::NoPedal));
    x += 35.f;
    addChild(createParamCentered<PedalKnob>(Vec(x, y + 10.f), module, Hc2P::P_PEDAL2));
    addChild(pedal2_assign = createStaticTextLabel<StaticTextLabel>(Vec(x, y + 24.f), 60.f, "Sostenuto", TextAlignment::Center, 10.f, true));
}

Hc2ModuleWidget::Hc2ModuleWidget(Hc2Module * module)
{
    my_module = module;
    setModule(module);
    if (my_module) {
        my_module->ui_event_sink = this;
    }
    setPanel(createPanel(asset::plugin(pluginInstance, "res/HC-2.svg")));

    addChild(device_label = createStaticTextLabel<StaticTextLabel>(
        Vec(7.f, 14.f), 180.f, "", TextAlignment::Left, 10.f, false, GetStockColor(StockColor::pachde_blue_medium)));

    createRoundingUI(ROUND_BOX_LEFT, ROUND_BOX_TOP);
    createPedalUI(PEDAL_BOX_LEFT, PEDAL_BOX_TOP);

    auto x = box.size.x * .5f;
    addChild(createCCMap<CCMap>(x, box.size.y - 24.f - 20.f, true, CCMapChannel::One, this));
    addChild(createCCMap<CCMap>(x, box.size.y - 24.f, true, CCMapChannel::Sixteen, this));
}

SymbolWidget::Symbol SymbolForPedal(PedalType pedal)
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

void Hc2ModuleWidget::onPresetChanged(const PresetChangedEvent& e)
{
    if (my_module) {
        static_cast<PedalParamQuantity*>(my_module->getParamQuantity(Hc2P::P_PEDAL1))->setEnabled(true);
        static_cast<PedalParamQuantity*>(my_module->getParamQuantity(Hc2P::P_PEDAL2))->setEnabled(true);
    }
    rounding_summary->modified();
}

void Hc2ModuleWidget::onPedalChanged(const PedalChangedEvent& e)
{
    switch (e.pedal.jack) {
    case 0: 
        pedal1_type->setSymbol(SymbolForPedal(e.pedal.type)); 
        pedal1_assign->text(LongPedalAssignment(e.pedal.cc));
        break;
    case 1:
        pedal2_type->setSymbol(SymbolForPedal(e.pedal.type));
        pedal2_assign->text(LongPedalAssignment(e.pedal.cc));
        break;
    default: break;
    }
}

void Hc2ModuleWidget::onRoundingChanged(const RoundingChangedEvent& e)
{
    rounding_summary->modified();
}

void Hc2ModuleWidget::onDeviceChanged(const DeviceChangedEvent& e)
{
    device_label->text(e.device ? e.device->info.friendly(false) : "");
}

void Hc2ModuleWidget::onDisconnect(const DisconnectEvent& e)
{
    device_label->text("");
}

Hc1Module* Hc2ModuleWidget::getPartner()
{
    if (!my_module) return nullptr;
    return my_module->getPartner();
}

// NVGcolor ref_line_color = nvgHSLAf(210.f/360.f, .5f, .5f, .5f);

// void drawMap(NVGcontext* vg, uint8_t * map, float x, float y)
// {
//     Line (vg, x + 1.25f + 32.f, y, x + 1.25f + 32.f, y + 17.f, ref_line_color, .5f);
//     Line (vg, x + 1.25f + 64.f, y, x + 1.25f + 64.f, y + 17.f, ref_line_color, .5f);
//     BoxRect(vg, x, y, 254, 18, RampGray(G_35), .5f);
//     ++x;
//     y += 17.f;
//     for (auto n = 0; n < 127; ++n, ++map, x += 2) {
//         if (auto v = *map) {
//             Line(vg, x, y, x, y - v/8.f, RampGray(G_85), 1.6f);
//         }
//     }
// }

// #ifdef SHOW_RCC_REFERENCE
// std::vector<uint8_t> reference_points = { EMCC_Download };
// #endif

// void Hc2ModuleWidget::drawCCMap(const DrawArgs& args, Hc1Module * partner)
// {
//     assert(partner);
//     auto x = box.size.x * .5f - 126.5f;
//     auto y0 = box.size.y - 15.f - 18.f - 18.f; //51
// #ifdef SHOW_RCC_REFERENCE
//     for (auto n: reference_points) {
//         Line (args.vg, x + 1.25f + 2.f*n, y0, x + 1.25f + 2.f*n, y0 - 5.f, GetStockColor(StockColor::Magenta), .75f);
//     }
// #endif
//     drawMap(args.vg, partner->ch0_cc_value, x, y0);
//     drawMap(args.vg, partner->ch15_cc_value, x, box.size.y - 15.f - 18.f);
// }

void Hc2ModuleWidget::draw(const DrawArgs& args)
{
    ModuleWidget::draw(args);

    auto vg = args.vg;
    BoxRect(vg, ROUND_BOX_LEFT, ROUND_BOX_TOP, ROUND_BOX_WIDTH, ROUND_BOX_HEIGHT, RampGray(G_40), 0.5f);
    BoxRect(vg, PEDAL_BOX_LEFT, PEDAL_BOX_TOP, PEDAL_BOX_WIDTH, PEDAL_BOX_HEIGHT, RampGray(G_40), 0.5f);

    //auto partner = getPartner();
    //if (partner) {
    //    drawCCMap(args, partner);

        // system_data
        // auto font = GetPluginFontRegular();
        // SetTextStyle(args.vg, font, RampGray(G_90), 10.f);
        // std::string data;
        // if (partner->system_data.empty()) {
        //     data = "(empty)";
        // } else {
        //     for (auto b:partner->system_data) {
        //         auto hex = format_string("%02x", b);
        //         data.append(hex);
        //     }
        // }
        // nvgText(args.vg, 7.5, box.size.y - 64, data.c_str(), nullptr);
    //}

}

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