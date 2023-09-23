#include "hc-3.hpp"
#include "../HcOne.hpp"

namespace pachde {

constexpr const float LABEL_COLUMN = 28.f;
constexpr const float LABEL_VOFFSET = 3.f;
constexpr const float START_ROW = 47.5f;
constexpr const float ITEM_INTERVAL = 20.f;
constexpr const float DIVIDER_OFFSET = 5.f;

Hc3ModuleWidget::Hc3ModuleWidget(Hc3Module* module)
:   my_module(module)
{
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/HC-3.svg")));
    float y = START_ROW;
    float x = 15.f;
    for (auto i = 0; i < 16; ++i) {
        addChild(createLightCentered<SmallLight<BlueLight>>(Vec(x,y), module, Hc3Module::Lights::SETLIST + i));
        y += ITEM_INTERVAL;
        if (i == 7) {
            y += DIVIDER_OFFSET;
        }
    }
}

// void Hc3ModuleWidget::step()
// {
//     ModuleWidget::step();
// }

// void Hc3ModuleWidget::drawExpanderConnector(const DrawArgs& args)
// {
// }

void Hc3ModuleWidget::drawLayer(const DrawArgs& args, int layer)
{
    ModuleWidget::drawLayer(args, layer);
    if (layer != 1 
        || !my_module 
        || my_module->loaded_id < 0
        || my_module->files[my_module->loaded_id].empty()) {
        return;
    }
    auto id = my_module->loaded_id;
    auto vg = args.vg;
    auto font = GetPluginFontSemiBold();
    const float y = START_ROW + LABEL_VOFFSET + (id * ITEM_INTERVAL) + (DIVIDER_OFFSET * (id > 7));
    SetTextStyle(vg, font, preset_name_color, 9.f);
    nvgTextAlign(vg, NVG_ALIGN_LEFT);
    nvgText(vg, LABEL_COLUMN, y, my_module->files[id].c_str(), nullptr);
}

void Hc3ModuleWidget::draw(const DrawArgs& args)
{
    ModuleWidget::draw(args);
    auto vg = args.vg;
    auto font = GetPluginFontRegular();
    SetTextStyle(vg, font, RampGray(G_85), 9.f);

    if (my_module) {
        auto one = HcOne::get();
        auto hc1 = one->getSoleHc1();
        std::string info = "";
        if (hc1) {
            info = hc1->deviceName();
        } else {
            info = format_string("%d", one->Hc1count());
        }
        CenterText(vg, box.size.x*.5f, 30.f, info.c_str(), nullptr);
        nvgTextAlign(vg, NVG_ALIGN_LEFT);

        float y = START_ROW + LABEL_VOFFSET;
        for (auto n = 0; n < 16; ++n) {
            if (!my_module->files[n].empty() && my_module->loaded_id != n) {
                nvgText(vg, LABEL_COLUMN, y, my_module->files[n].c_str(), nullptr);
            }
            y += ITEM_INTERVAL + ((n == 7) * DIVIDER_OFFSET);
        }
    } else {
        CenterText(vg, box.size.x*.5f, 30.f, "<device name>", nullptr);
    }
//    drawExpanderConnector(args);
    DrawLogo(vg, box.size.x*.5f - 8.f, RACK_GRID_HEIGHT - ONE_HP, RampGray(G_90));
}

void Hc3ModuleWidget::appendContextMenu(Menu *menu)
{

}

}