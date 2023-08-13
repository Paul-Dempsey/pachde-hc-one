#include "preset_widget.hpp"
#include "misc.hpp"
#include "text.hpp"
#include "components.hpp"
#include "colors.hpp"

namespace pachde {

//const NVGcolor user_preset_color = nvgRGB(0x7e, 0x60, 0x9f);
//const NVGcolor user_preset_color = nvgRGB(0x3d, 0x21, 0x5d);

void PresetWidget::draw(const DrawArgs& args)
{
    TipWidget::draw(args);

    auto vg = args.vg;

    bool selected = holder && holder->isCurrentPreset(preset);
    //bool user_preset = preset && (127 != preset->bank_hi);
    if (preset && pressed) {
        FillRect(vg, 0, 0, box.size.x, box.size.y, preset_name_color);
    }
    if (preset && (pressed || selected)) {
        BoxRect(vg, .5, .5, box.size.x - 1.f, box.size.y-1.f, preset_name_color, 1.f);
    } else {
        BoxRect(vg, 0, 0, box.size.x, box.size.y, RampGray(G_40), .5f);
    }
    if (preset) {
        auto font = GetPluginFontRegular();
        if (FontOk(font)) {
            nvgSave(vg);
            nvgScissor(vg, 1.f, 1.f, box.size.x-2.f, box.size.y-2.f);
            SetTextStyle(vg, font, pressed ? RampGray(G_15) : RampGray(G_90), 12.f);
            nvgText(vg, 1.5f, 10.5f, preset->name.c_str(), nullptr);
            nvgRestore(vg);
        }
        if (!preset->isSysPreset()) {
            FillPerson(vg, 2.3f, 13.5f, 8.f, RampGray(G_65));
        }
        if (preset->favorite) {
            FillHeart(vg, 10.f, 13.5f, 6.f, PORT_PINK);
        } else {
            StrokeHeart(vg, 10.f, 13.5f, 6.f, PORT_PINK, .5f);
        }
    }
}

}