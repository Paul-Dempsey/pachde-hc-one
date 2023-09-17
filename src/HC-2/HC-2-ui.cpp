#include "HC-2.hpp"
#include "tuning_ui.hpp"
#include "../cc_param.hpp"
#include "../colors.hpp"
#include "../components.hpp"
#include "../misc.hpp"
#include "../port.hpp"
#include "../small_push.hpp"
#include "../switch_4.hpp"
#include "../text.hpp"

namespace pachde {

using Hc2P = Hc2Module::Params;
using Hc2I = Hc2Module::Inputs;
using Hc2L = Hc2Module::Lights;

constexpr const float PAD = 2.f;
constexpr const float MORE_PAD = 4.f;
constexpr const float ROUND_BOX_WIDTH = 105.f;
constexpr const float ROUND_BOX_HEIGHT = 60.f;
constexpr const float ROUND_BOX_HALF = 110.f * .5f;
constexpr const float ROUND_BOX_QUARTER = ROUND_BOX_HALF * .5f;
constexpr const float KNOB_RADIUS = 12.f;
constexpr const float HALF_KNOB = KNOB_RADIUS *.5f;
constexpr const float REL_OFFSET = 20.f;
constexpr const float REL_VOFFSET = 10.f;
constexpr const float CV_COLUMN_OFFSET = 24.f;
constexpr const float ROUND_KNOB_ROW = 32.5f;
constexpr const float ROUND_COL1 = ROUND_BOX_HALF - KNOB_RADIUS - 3.f * PAD;
constexpr const float ROUND_COL2 = ROUND_BOX_HALF + KNOB_RADIUS + 2.f * PAD;
constexpr const float ROUND_COL3 = ROUND_BOX_WIDTH - KNOB_RADIUS + PAD;

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

Hc2ModuleWidget::Hc2ModuleWidget(Hc2Module * module)
{
    my_module = module;
    setModule(module);
    if (my_module) {
        my_module->ui_event_sink = this;
    }
    setPanel(createPanel(asset::plugin(pluginInstance, "res/HC-2.svg")));

    // current preset in title
    preset_label = createStaticTextLabel<DynamicTextLabel>(
        Vec(80.f, 7.5f), 250.f, "My Amazing Preset", TextAlignment::Left, 14.f, true, preset_name_color);
    preset_label->bright();
    addChild(preset_label);

    createRoundingUI(15.f, 40.f);

    // device name
    device_label = createStaticTextLabel<StaticTextLabel>(
        Vec(box.size.x*.5f + 25.f, box.size.y - 14.f), 100.f,
        "", TextAlignment::Left, 12.f, false );
    addChild(device_label);
}

void Hc2ModuleWidget::onPresetChanged(const PresetChangedEvent& e)
{
    preset_label->text(e.preset ? e.preset->name : "");
    rounding_summary->modified();
}

void Hc2ModuleWidget::onRoundingChanged(const RoundingChangedEvent& e)
{
    rounding_summary->modified();
}

void Hc2ModuleWidget::step()
{
    ModuleWidget::step();
 
    if (device_label) {
        std::string device = my_module ? my_module->getDeviceName() : "<Eagan Matrix Device>";
        if (device_label->getText() != device) {
            device_label->text(device);
        }
    }
}

void Hc2ModuleWidget::drawExpanderConnector(const DrawArgs& args)
{
    if (!my_module || my_module->partner_side.empty()) return;
    auto vg = args.vg;

    auto right = my_module->partner_side.right();
    float cy = 80.f;
    if (right) {
        Line(vg, box.size.x - 5.5f, cy, box.size.x , cy, COLOR_BRAND, 1.75f);
        Circle(vg, box.size.x - 5.5f, cy, 2.5f, COLOR_BRAND);
    } else {
        Line(vg, 0.f, cy, 5.5f, cy, COLOR_BRAND, 1.75f);
        Circle(vg, 5.5f, cy, 2.5f, COLOR_BRAND);
    }
}

void drawMap(NVGcontext* vg, uint8_t * map, float x, float y)
{
    auto ref_line_color = nvgHSLAf(210.f/360.f, .5f, .5f, .5f);
    Line (vg, x + 1.25f + 32.f, y, x + 1.25f + 32.f, y + 17.f, ref_line_color, .5f);
    Line (vg, x + 1.25f + 64.f, y, x + 1.25f + 64.f, y + 17.f, ref_line_color, .5f);
    BoxRect(vg, x, y, 254, 18, RampGray(G_35), .5f);
    ++x;
    y += 17.f;
    for (auto n = 0; n < 127; ++n, ++map, x += 2) {
        if (auto v = *map) {
            Line(vg, x, y, x, y - v/8.f, RampGray(G_85), 1.6f);
        }
    }
}

void Hc2ModuleWidget::drawCCMap(const DrawArgs& args, Hc1Module * partner)
{
    assert(partner);
    auto x = box.size.x * .5f - 126.5f;
    //Line(args.vg, x + 1 + 117, 10.f, x + 1 + 117, 48.f, blue_light);
    drawMap(args.vg, partner->ch0_cc_value, x, box.size.y - 15.f - 18.f - 18.f);
    drawMap(args.vg, partner->ch15_cc_value, x, box.size.y - 15.f - 18.f);
}

void Hc2ModuleWidget::draw(const DrawArgs& args)
{
    ModuleWidget::draw(args);

    auto vg = args.vg;
    BoxRect(vg, 15.f, 40.f, ROUND_BOX_WIDTH, ROUND_BOX_HEIGHT, RampGray(G_40), 0.5f);

    auto partner = my_module ? my_module->getPartner() : nullptr;
    if (partner) {
        drawCCMap(args, partner);
    }
    drawExpanderConnector(args);
    DrawLogo(vg, box.size.x /2.f - 12.f, RACK_GRID_HEIGHT - ONE_HP, RampGray(G_90));
}

void Hc2ModuleWidget::appendContextMenu(Menu *menu)
{
    auto partner = my_module ? my_module->getPartner() : nullptr;
    menu->addChild(new MenuSeparator);
    if (partner) {
        menu->addChild(createMenuItem("Clear CC Map", "",
            [partner](){ partner->clearCCValues(); }));
    } else {
        menu->addChild(createMenuItem("- Not Connected - ", "", [](){}, true));
    }
}

}