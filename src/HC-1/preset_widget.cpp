#include "preset_widget.hpp"

namespace pachde {

//const NVGcolor user_preset_color = nvgRGB(0x7e, 0x60, 0x9f);
const NVGcolor user_preset_color = nvgRGB(0x3d, 0x21, 0x5d);
void PresetWidget::draw(const DrawArgs& args)
{
    auto vg = args.vg;

    bool selected = module && preset && (0 == preset->name.compare(module->preset0.name()));
    bool user_preset = preset && (127 != preset->bank_hi);
    if (pressed) {
        FillRect(vg, 0, 0, box.size.x, box.size.y, preset_name_color);
    } else if (user_preset) {
        FillRect(vg, 0, 0, box.size.x, box.size.y, user_preset_color);
    }
    if (pressed || selected) {
        BoxRect(vg, .5, .5, box.size.x - 1.f, box.size.y-1.f, preset_name_color, 1.f);
    } else {
        BoxRect(vg, 0, 0, box.size.x, box.size.y, RampGray(G_50), .75f);
    }
    if (preset) {
        auto font = GetPluginFontRegular();
        if (FontOk(font)) {
            nvgSave(vg);
            nvgScissor(vg, 0, 0, box.size.x, box.size.y);
            SetTextStyle(vg, font, pressed ? RampGray(G_15) : RampGray(G_90), 12.f);
            auto abbrev = AbbreviatedName(preset->name);
            nvgText(vg, 1.2f, 10.5f, abbrev.c_str(), nullptr);
            nvgRestore(vg);
        }
    }
}

}