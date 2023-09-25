#include "hc-3.hpp"
#include "../HcOne.hpp"
#include "../misc.hpp"
#include "preset_file_widget.hpp"

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
        addChild(createPFWidget<PresetFileWidget>(Vec(x - 7.5, y - 7.5), module, i, &drawButton));
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

void Hc3ModuleWidget::draw(const DrawArgs& args)
{
    ModuleWidget::draw(args);
    auto vg = args.vg;
    auto font = GetPluginFontRegular();
    SetTextStyle(vg, font, RampGray(G_85), 9.f);

    if (module) {
        auto one = HcOne::get();
        auto hc1 = one->getSoleHc1();
        std::string info = "";
        if (hc1) {
            info = hc1->deviceName();
        } else if (one->Hc1count() > 1) {
            info = "<multiple HC-!>";
        } else {
            info = "<no HC-1>";
        }
        CenterText(vg, box.size.x*.5f, 30.f, info.c_str(), nullptr);
    } else {
        CenterText(vg, box.size.x*.5f, 30.f, "<device name>", nullptr);
    }
//    drawExpanderConnector(args);
    DrawLogo(vg, box.size.x*.5f - 8.f, RACK_GRID_HEIGHT - ONE_HP, RampGray(G_90));
}

void Hc3ModuleWidget::appendContextMenu(Menu *menu)
{
    if (!module) return;
    ///pick companion hc1

    menu->addChild(new MenuSeparator);

    auto count = std::count_if(my_module->files.cbegin(), my_module->files.cend(), [](const std::string& s){ return !s.empty(); });
    bool any = count > 0;
    menu->addChild(createMenuItem("Clear", "", [=](){ my_module->clearFiles(); }));
    
    menu->addChild(createMenuItem("Sort", "", [=](){
        std::sort(my_module->files.begin(), my_module->files.end(), alpha_order);
    }, !any));

    menu->addChild(createMenuItem("Compact", "", [=](){
        int gap = 0;
        std::vector<std::string> items;
        items.reserve(16);
        int n = 0;
        for (auto s: my_module->files) {
            if (s.empty()) {
                ++gap;
            } else {
                items.push_back(s);
                if (n == my_module->loaded_id) {
                    my_module->loaded_id = n - gap;
                }
            }
            ++n;
        }
        for (n = 0; n < gap; ++n) {
            items.push_back("");
        }
        my_module->files = items;
    }, !any || 16 == count));
}

}