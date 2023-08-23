#include "HC-2.hpp"
#include "../colors.hpp"
#include "../components.hpp"
#include "../misc.hpp"
#include "../tab_bar.hpp"
#include "../text.hpp"

namespace pachde {

constexpr const float PRESET_TOP = 135.f;
constexpr const float PRESET_LEFT = 15.f;

Hc2ModuleWidget::Hc2ModuleWidget(Hc2Module * module)
{
    my_module = module;

    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/HC-2.svg")));

    box.size.x = 360.f;
    box.size.y = 380.f;
}

void Hc2ModuleWidget::drawExtenderConnector(const DrawArgs& args)
{
    if (!my_module || !my_module->partner) return;
    auto vg = args.vg;

    float cx = my_module->partner_side ? box.size.x : 0;
    float cy = box.size.y * .5f;

    Line(vg, cx - 6.f, cy, cx + 6.f, cy, COLOR_BRAND, 1.75f);
    Circle(vg, cx - 6.f, cy, 3.f, COLOR_BRAND);
    //OpenCircle(vg, cx - 6.f, cy, 5.f, COLOR_BRAND, .75f);
    Circle(vg, cx + 6.f, cy, 3.f, COLOR_BRAND);
    //OpenCircle(vg, cx + 6.f, cy, 5.f, COLOR_BRAND, .75f);
}

void drawMap(NVGcontext* vg, uint8_t * map, float x, float y)
{
    // auto font = GetPluginFontRegular();
    // if (FontOk(font)) {
    //     SetTextStyle(vg, font, RampGray(G_95), 10.f);
    //     auto text = format_string("%3d %3d", map[117], map[118]);
    //     nvgText(vg, x + 135.f, y + 10.f, text.c_str(), nullptr);
    // }
    BoxRect(vg, x, y, 254, 18, RampGray(G_35), .5f);
    ++x;
    y += 17.f;
    for (auto n = 0; n < 127; ++n, ++map, x += 2) {
        if (auto v = *map) {
            Line(vg, x, y, x, y - v/8.f, RampGray(G_85), 1.6f);
        }
    }
}

void Hc2ModuleWidget::drawCCMap(const DrawArgs& args)
{
    if (my_module && my_module->partner) {
        auto x = box.size.x * .5f - 126.5f;
        //Line(args.vg, x + 1 + 117, 10.f, x + 1 + 117, 48.f, blue_light);
        drawMap(args.vg, my_module->partner->ch0_cc_value, x, 20.f);
        drawMap(args.vg, my_module->partner->ch15_cc_value, x, 40.f);
    }
}

void Hc2ModuleWidget::draw(const DrawArgs& args)
{
    ModuleWidget::draw(args);
    auto vg = args.vg;

    auto font = GetPluginFontRegular();
    if (FontOk(font)) {
        SetTextStyle(vg, font, RampGray(G_90), 12.f);
    }
    auto device =  my_module ? my_module->getDeviceName() : "";
    if (!device.empty() && FontOk(font)) {
        nvgText(vg, box.size.x/2.f + 25.f, box.size.y - 7.5f, device.c_str(), nullptr);
    }
    drawCCMap(args);

    if (my_module && my_module->partner && FontOk(font))
    {
{
        nvgSave(vg);
        auto text = format_string("%d", my_module->partner->midi_receive_count);
        RightAlignText(vg, box.size.x - 5.f, 30.f, text.c_str(), nullptr);
        nvgRestore(vg);
}
        auto hc1 = my_module->partner;
        auto cc15 = hc1->ch15_cc_value;
        auto cc0 = hc1->ch0_cc_value;
        const char * const ped_indicator = "NSEDTVP";
        auto peds = format_string(
            "Pedals (1 %c %d %d %d-%d) (2 %c %d %d %d-%d) (Sus %d) (Sos %d) (Sos2 %d)",
            ped_indicator[cc15[37] & 0x07], 
            cc15[52], cc0[cc15[52]],
            cc15[76], cc15[77],

            ped_indicator[(cc15[37] >> 3) & 0x07],
            cc15[53], cc0[cc15[53]],
            cc15[78], cc15[79],

            cc0[64], cc0[66], cc0[69]
        );
        nvgText(vg, 7.5, 80.f, peds.c_str(), nullptr);

        auto text = format_string("C4=%d | OCT %d | POLY %d%c T%d D%d V%d",
            cc15[44],
            cc0[8],
            (cc15[39] & 0x1f), 
            (cc15[39] & 0x40 ? '+': ' '),
            cc15[71], cc15[72], cc15[73]
            );
        nvgText(vg, 7.5, 94.f, text.c_str(), nullptr);

        const char * const round_kind = "_RYy";
        auto cc = cc0;
        auto rk = round_kind[(cc[61] & 0x6) >> 1];
        auto req = cc[65];
        auto reqs = 
            req == 0 ? "off":
            req == 64 ? "enabled" :
            req == 127 ? "equal" : "(unknown)";
        text = format_string("Rounding %s RR %d RI %d Rk %c", reqs, cc[25], cc[28], rk);
        nvgText(vg, 7.5, 109.f, text.c_str(), nullptr);

        text = format_string("device = %s", InitStateName(hc1->device_state));
        nvgText(vg, 7.5, 124.f, text.c_str(), nullptr);
        text = format_string("presets = %s", InitStateName(hc1->preset_state));
        nvgText(vg, 7.5, 139.f, text.c_str(), nullptr);
        text = format_string("config = %s", InitStateName(hc1->config_state));
        nvgText(vg, 7.5, 154.f, text.c_str(), nullptr);
        text = format_string("saved = %s", InitStateName(hc1->saved_preset_state));
        nvgText(vg, 7.5, 170.f, text.c_str(), nullptr);
        text = format_string("updates = %s", InitStateName(hc1->requested_updates));
        nvgText(vg, 7.5, 185.f, text.c_str(), nullptr);
        if (hc1->broken) {
            nvgText(vg, 7.5, 200.f, "BROKE", nullptr);
        }

    }

    drawExtenderConnector(args);
    DrawLogo(vg, box.size.x /2.f - 12.f, RACK_GRID_HEIGHT - ONE_HP, RampGray(G_90));
}

void Hc2ModuleWidget::appendContextMenu(Menu *menu)
{
    menu->addChild(new MenuSeparator);
    if (my_module && my_module->partner) {
        menu->addChild(createMenuItem("Clear CC Map", "",
            [this](){ my_module->partner->clearCCValues(); }));
    } else {
        menu->addChild(createMenuItem("- Not Connected - ", "", [](){}, true));
    }
}

}