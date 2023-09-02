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

    auto vg = args.vg;

    bool selected = holder && holder->isCurrentPreset(preset);
    //bool user_preset = preset && (127 != preset->bank_hi);
    if (preset && pressed) {
        FillRect(vg, 0, 0, box.size.x, box.size.y, preset_name_color);
    }
    if (preset && (pressed || selected)) {
        BoxRect(vg, .75, .75, box.size.x - 1.5f, box.size.y-1.5f, preset_name_color, .75f);
    } else if (preset && hovered) {
        FillRect(vg, 1, 1, box.size.x-2, box.size.y-2, RampGray(G_30));
    } else {
        //BoxRect(vg, 0, 0, box.size.x, box.size.y, RampGray(G_40), .5f);
        FillRect(vg, 1, 1, box.size.x-2, box.size.y-2, RampGray(G_20));
    }

    TipWidget::draw(args);

    if (preset) {
        auto font = GetPluginFontRegular();
        if (FontOk(font)) {
            nvgSave(vg);
            nvgScissor(vg, 2.f, 2.f, box.size.x-4.f, box.size.y-4.f);
            SetTextStyle(vg, font, pressed ? RampGray(G_15) : RampGray(G_90), 12.f);
            nvgText(vg, 2.5f, 11.5f, preset->name.c_str(), nullptr);
            nvgRestore(vg);

            auto code = *preset->get_category_list().cbegin();
            SetTextStyle(vg, font, pressed ? GetStockColor(StockColor::pachde_blue_dark) : GetStockColor(StockColor::pachde_blue_light), 9.f);
            RightAlignText(vg, box.size.x-4.f, box.size.y-4.f, (const char *)&code, ((const char *)&code) + 2);
        }
        if (preset->favorite) {
            FillHeart(vg, 4.f, 16.5f, 6.f, PORT_PINK);
        }
        if (0 == preset->bank_hi) {
            FillPerson(vg, preset->favorite ? 12.f : 4.f, 15.5f, 8.f, nvgHSL(210.f/360.f, .5f, .5f));
        }
    }
}

}