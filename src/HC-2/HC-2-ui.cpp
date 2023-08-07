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
    box.size.x = 360.f;
    box.size.y = 380.f;

    auto tab_bar = createWidget<TabBarWidget>(Vec(PRESET_LEFT, PRESET_TOP - 13.f));
    tab_bar->setSize(Vec(320.f, 13.f));
    tab_bar->addTab("Favorite");
    tab_bar->addTab("User");
    tab_bar->addTab("System");
    tab_bar->selectTab(2);
    addChild(tab_bar);

}

void Hc2ModuleWidget::draw(const DrawArgs& args)
{
    ModuleWidget::draw(args);
    auto vg = args.vg;

    FillRect(vg, 0, 0, box.size.x, box.size.y, RampGray(G_20));

    if (my_module) {
        Circle(vg, 7.5f + (my_module->midi_count % static_cast<unsigned int>(box.size.x - 15.f)), box.size.y - 18.f, 2.f, blue_green_light);
    }

    auto font = GetPluginFontRegular();
    if (FontOk(font)) {
        std::string device_name;
        if (my_module) {
            device_name = FilterDeviceName(my_module->device_name);
        } else {
            device_name = "<Eagan Matrix Device>";
        }
        if (device_name.empty()) {
            device_name = "(no EM device found)";
        }
        SetTextStyle(vg, font, RampGray(G_90), 12.f);
        nvgText(vg, box.size.x/2.f + 25.f, box.size.y - 7.5f, device_name.c_str(), nullptr);

        if (my_module) {
            if (my_module->is_eagan_matrix && (my_module->firmware_version > 0)) {
                RightAlignText(vg, box.size.x - 7.5, box.size.y - 7.5f, format_string("v %.2f", my_module->firmware_version/100.f).c_str(), nullptr);
            }
        } else {
            RightAlignText(vg, box.size.x - 7.5, box.size.y - 7.5f, "v 10.09", nullptr);
        }
    }

    font = GetPluginFontSemiBold();
    if (FontOk(font)) {
        nvgText(vg, .75f, 15.f, "HC-2", nullptr);
    }

    DrawLogo(vg, box.size.x /2.f - 12.f, RACK_GRID_HEIGHT - ONE_HP, RampGray(G_90));
}

}