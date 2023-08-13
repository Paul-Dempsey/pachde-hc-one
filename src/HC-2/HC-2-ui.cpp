#include "hc-2.hpp"
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

void Hc2ModuleWidget::drawCCMap(const DrawArgs& args)
{
    auto vg = args.vg;

    float cc_top = 24.f;
    float cc_left_0 = 8.5;
    float cc_left_15 = cc_left_0 + 127.f + 2.f;
    BoxRect(vg, 7.5f, cc_top, 260.f, 18.f, RampGray(G_35), .5f);
    Line(vg, 7.5f + 130.f, cc_top, 7.5f + 131.f, cc_top + 18.f, RampGray(G_35), .5f);
    if (my_module && my_module->partner) {
        auto cc0 = my_module->partner->ch0_cc_value;
        auto cc15 = my_module->partner->ch15_cc_value;
        for (auto n = 0; n < 127; ++n, ++cc0, ++cc15) {
            auto v = *cc0;
            if (v) { Line(vg, cc_left_0 + n, cc_top + 16.f, cc_left_0 + n, cc_top + 16.f - v/16.f, RampGray(G_85)); }
            v = *cc15;
            if (v) { Line(vg, cc_left_15 + n, cc_top + 16.f, cc_left_15 + n, cc_top + 16.f - v/16.f, RampGray(G_85)); }
        }
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
        auto ccs = my_module->partner->ch15_cc_value;
        auto cc0 = my_module->partner->ch0_cc_value;
        const char * const ped_indicator = "NSEDTVP";
        auto peds = format_string(
            "Pedals (1 %c %d %d %d-%d) (2 %c %d %d %d-%d) (Sus %d) (Sos %d) (Sos2 %d)",
            ped_indicator[ccs[37] & 0x07], 
            ccs[52], cc0[ccs[52]],
            ccs[76], ccs[77],
            ped_indicator[(ccs[37] >> 3) & 0x07],
            ccs[53], cc0[ccs[53]],
            ccs[78], ccs[79],
            ccs[64], ccs[66], ccs[69]
        );
        nvgText(vg, 7.5, 80.f, peds.c_str(), nullptr);

        auto text = format_string("C4=%d | OCT %d | POLY %d%c T%d D%d V%d",
            ccs[44],
            cc0[8],
            (ccs[39] & 0x1f), 
            (ccs[39] & 0x40 ? '+': ' '),
            ccs[71], ccs[72], ccs[73]
            );
        nvgText(vg, 7.5, 94.f, text.c_str(), nullptr);

        const char * const round_kind = "_RYy";
        auto rk = round_kind[(ccs[61] & 0x6) >> 1];
        auto req = ccs[65];
        auto reqs = 
            req == 0 ? "off":
            req == 64 ? "enabled" :
            req == 127 ? "equal" : "(unknown)";
        text = format_string("Rounding %s RR %d RI %d Rk %c", reqs, ccs[25], ccs[28], rk);
        nvgText(vg, 7.5, 109.f, text.c_str(), nullptr);

    }

    drawExtenderConnector(args);
    DrawLogo(vg, box.size.x /2.f - 12.f, RACK_GRID_HEIGHT - ONE_HP, RampGray(G_90));
}

}