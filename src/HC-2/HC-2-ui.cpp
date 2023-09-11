#include "HC-2.hpp"
#include "../cc_param.hpp"
#include "../colors.hpp"
#include "../components.hpp"
#include "../em_types_ui.hpp"
#include "../misc.hpp"
#include "../port.hpp"
#include "../small_push.hpp"
//#include "../tab_bar.hpp"
#include "../text.hpp"

namespace pachde {

constexpr const float PRESET_TOP = 135.f;
constexpr const float PRESET_LEFT = 15.f;

constexpr const float KNOB_LEFT   = 45.f;
constexpr const float KNOB_ROW_1  = 54.f;
constexpr const float KNOB_SPREAD = 54.25f;
constexpr const float CV_ROW_1 = KNOB_ROW_1 + 6.f;

constexpr const float LABEL_OFFSET = 20.f;
constexpr const float STATIC_LABEL_OFFSET = 29.5f;
constexpr const float CV_COLUMN_OFFSET = 24.f;
constexpr const float RB_OFFSET = 20.f;
constexpr const float RB_VOFFSET = 15.f;

Hc2ModuleWidget::Hc2ModuleWidget(Hc2Module * module)
{
    my_module = module;

    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/HC-2.svg")));

    addChild(createStaticTextLabel<TextLabel>(Vec(KNOB_LEFT, KNOB_ROW_1 - STATIC_LABEL_OFFSET), 60.f, "Rounding"));
    addChild(createMidiKnob(Vec(KNOB_LEFT, KNOB_ROW_1), module, Hc2Module::Params::ROUND_RATE_PARAM, Hc2Module::Inputs::ROUND_RATE_INPUT, Hc2Module::Params::ROUND_RATE_REL_PARAM));
    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<BlueLight>>>(Vec(KNOB_LEFT - RB_OFFSET, CV_ROW_1 - RB_VOFFSET), module, Hc2Module::Params::ROUND_RATE_REL_PARAM, Hc2Module::Lights::ROUND_RATE_REL_LIGHT));
    addChild(createInputCentered<ColorPort>(Vec(KNOB_LEFT - CV_COLUMN_OFFSET, CV_ROW_1), module, Hc2Module::Inputs::ROUND_RATE_INPUT));
    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<BlueLight>>>(Vec(KNOB_LEFT + RB_OFFSET, CV_ROW_1 - RB_VOFFSET), module, Hc2Module::Params::ROUND_INITIAL_PARAM, Hc2Module::Lights::ROUND_INITIAL_LIGHT));

    //addChild(createMidiKnob(Vec(KNOB_LEFT + KNOB_SPREAD, KNOB_ROW_1), module, Hc2Module::Params::ROUND_TUNING_PARAM, 

    // device name
    addChild(createTextLabel<pachde::TextLabel>(
        Vec(box.size.x*.5f + 25.f, box.size.y - 14.f), 100.f,
        TextAlignment::Left, 12.f,
        [=]() {
            std::string device_name;
            device_name = my_module ? my_module->getDeviceName() : "<Eagan Matrix Device>";
            if (device_name.empty()) {
                device_name = "(no Eagan Matrix available)";
            }
            return device_name; 
        },
        false
        ));
}

void Hc2ModuleWidget::drawExpanderConnector(const DrawArgs& args)
{
    if (!my_module || my_module->partner_side.empty()) return;
    auto vg = args.vg;

    auto right = my_module->partner_side.right();
    float cy = box.size.y * .5f;
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

std::string PedalAssign(uint8_t a) {
    switch (a) {
    case 12: return "i";
    case 13: return "ii";
    case 14: return "iii";
    case 15: return "iv";
    case 16: return "v";
    case 17: return "vi";
    case 20: return "R1";
    case 21: return "R2";
    case 22: return "R3";
    case 23: return "R4";
    case 24: return "RMix";
    case 64: return "Sustain";
    case 66: return "Sostenuto";
    }
    return format_string("%3d", a);
}

void Hc2ModuleWidget::draw(const DrawArgs& args)
{
    ModuleWidget::draw(args);
    auto vg = args.vg;
    auto partner = my_module ? my_module->getPartner() : nullptr;
    // auto font = GetPluginFontRegular();
    // if (FontOk(font)) {
    //     SetTextStyle(vg, font, RampGray(G_90), 12.f);

    //     auto device =  partner ? my_module->getDeviceName() : nullptr;
    //     if (device) {
    //         nvgText(vg, box.size.x/2.f + 25.f, box.size.y - 4.f, device, nullptr);
    //     }
    // }
    if (partner) {
        drawCCMap(args, partner);
    }
    // if (partner && FontOk(font))
    // {
    //     auto y = 80.f;
    //     const float row_interval = 14.f;
    //     auto cc15 = partner->ch15_cc_value;
    //     auto cc0 = partner->ch0_cc_value;
    //     const char * const ped_indicator = "NSEDTVP";
    //     auto ped = cc15[52];
    //     auto peds = format_string(
    //         "Pedal 1 %c %s(%3d) %3d..%3d",
    //         ped_indicator[cc15[37] & 0x07], 
    //         PedalAssign(ped).c_str(), cc0[ped],
    //         cc15[76], cc15[77]);
    //     nvgText(vg, 7.5, y, peds.c_str(), nullptr);
    //     y += row_interval;

    //     ped = cc15[53];
    //     peds = format_string(
    //         "Pedal 2 %c %s(%3d) %3d..%3d)",
    //         ped_indicator[(cc15[37] >> 3) & 0x07],
    //         PedalAssign(ped).c_str(), cc0[ped],
    //         cc15[78], cc15[79]);
    //     nvgText(vg, 7.5, y, peds.c_str(), nullptr);
    //     y += row_interval;

    //     peds = format_string("Sus %d Sos %d Sos2 %d", cc15[64], cc15[66], cc15[69]);
    //     nvgText(vg, 7.5, y, peds.c_str(), nullptr);
    //     y += row_interval;

    //     // Apparently just Kenton
    //     // peds = format_string("Jack1 %3d Jack2 %3d", cc15[29], cc15[30]);
    //     // nvgText(vg, 7.5, y, peds.c_str(), nullptr);
    //     // y += row_interval;

    //     auto text = format_string("C4=%d | OCT %d | POLY %d%c T%d D%d V%d",
    //         cc15[44],
    //         cc0[8],
    //         (cc15[39] & 0x1f), 
    //         (cc15[39] & 0x40 ? '+': ' '),
    //         cc15[71], cc15[72], cc15[73]
    //         );
    //     nvgText(vg, 7.5, y, text.c_str(), nullptr);
    //     y += row_interval;

    //     const char * const round_kind = "_RYy";
    //     auto cc = cc0;
    //     auto rk = round_kind[(cc[61] & 0x6) >> 1];
    //     auto req = cc[65];
    //     auto reqs = 
    //         req == 0 ? "off":
    //         req == 64 ? "enabled" :
    //         req == 127 ? "equal" : "(unknown)";
        
    //     text = format_string("Rounding %s RR %d RI %d Rk %c", reqs, cc[25], cc[28], rk);
    //     nvgText(vg, 7.5, y, text.c_str(), nullptr);
    //     y += row_interval;

    //     // text = format_string("device = %s", InitStateName(hc1->device_hello_state));
    //     // nvgText(vg, 7.5, y, text.c_str(), nullptr);
    //     // y += row_interval;
    //     // text = format_string("system presets = %s", InitStateName(hc1->system_preset_state));
    //     // nvgText(vg, 7.5, y, text.c_str(), nullptr);
    //     // y += row_interval;
    //     // text = format_string("user presets = %s", InitStateName(hc1->user_preset_state));
    //     // nvgText(vg, 7.5, y, text.c_str(), nullptr);
    //     // y += row_interval;
    //     // text = format_string("config = %s", InitStateName(hc1->config_state));
    //     // nvgText(vg, 7.5, y, text.c_str(), nullptr);
    //     // y += row_interval;
    //     // text = format_string("saved = %s", InitStateName(hc1->saved_preset_state));
    //     // nvgText(vg, 7.5, y, text.c_str(), nullptr);
    //     // y += row_interval;
    //     // text = format_string("updates = %s", InitStateName(hc1->request_updates_state));
    //     // nvgText(vg, 7.5, y, text.c_str(), nullptr);
    //     // y += row_interval;
    //     if (partner->broken) {
    //         nvgText(vg, 7.5, y, "BROKE", nullptr);
    //         y += row_interval;
    //     }
    //}

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