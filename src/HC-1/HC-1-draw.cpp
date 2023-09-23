#include "HC-1.hpp"
#include "HC-1-layout.hpp"
#include "../em_pedal.hpp"
namespace pachde {
//#define SHOW_PRESET0 // debug: show preset0 on module

const NVGcolor& InitStateColor(InitState state)
{
    switch (state) {
    case InitState::Uninitialized: return gray_light;
    case InitState::Pending: return orange_light;
    case InitState::Complete: return blue_light;
    case InitState::Broken: return red_light;
    default: return no_light;
    }
}

const NVGcolor& StatusColor(StatusItem led)
{
    switch (led) {
        case StatusItem::ledOff: return no_light;
        case StatusItem::ledBlue: return blue_light;
        case StatusItem::ledRed: return red_light;
        case StatusItem::ledBrightGreen:  return bright_green_light;
        case StatusItem::ledGreen: return green_light;
        case StatusItem::ledWhite: return white_light;
        case StatusItem::ledYellow: return yellow_light;
        case StatusItem::ledPurple: return purple_light;
        case StatusItem::ledBlueGreen:  return bright_green_light;
        default: return gray_light;
    }
}

// TODO: Candidate for caching. Only updates on heartbeat.
void Hc1ModuleWidget::drawDSP(NVGcontext* vg)
{
    const float h = 10.f;
    const float w = 2.5f;
    const float width = w * 3.f + 5.f;
    const float height = h + 3.f;
    float x = RIGHT_COLUMN_BUTTONS - width * .5f;
    float y = 90.f;

    FillRect(vg, x, y, width, height, RampGray(G_30));

    bool module_ready = my_module && my_module->ready();
    const uint8_t tdsp[] = { 65, 30, 75 };
    const uint8_t* pdsp = module_ready ? &my_module->dsp[0] : &tdsp[0];

    if (!module_ready) {
        BoxRect(vg, x, y, width, height, green_light, .5f);
    }

    x += 1.5f;
    y += 1.5f;
    for (auto n = 0; n < 3; n++) {
        auto pct = pdsp[n];
        auto co = pct < 85 ? green_light : red_light;
        auto bh = h * (pct / 100.f);
        FillRect(vg, x, y + h - bh, w, bh, co);
        x += w + 1;
    }
}

void Hc1ModuleWidget::drawStatusDots(NVGcontext* vg)
{
    float left = 60.f;
    float spacing = 6.25f;
    float y = box.size.y - 7.5f;

    if (my_module) {
        // note
        Dot(vg, left, y, my_module->notesOn ? purple_light : gray_light, my_module->notesOn);
        left += spacing;
        //device_output_state
        Dot(vg, left, y, InitStateColor(my_module->device_output_state));
        left += spacing;
        // device_input_state
        Dot(vg, left, y, InitStateColor(my_module->device_input_state));
        left += spacing;
        //eagan matrix
        Dot(vg, left, y, my_module->is_eagan_matrix ? blue_light : yellow_light);
        left += spacing;
        // //device_state
        // Dot(vg, left, y, InitStateColor(my_module->device_hello_state));
        // left += spacing;
        //system_preset_state
        Dot(vg, left, y, InitStateColor(my_module->system_preset_state));
        left += spacing;
        //user_preset_state
        Dot(vg, left, y, InitStateColor(my_module->user_preset_state));
        left += spacing;
        //apply_favorite_state
        Dot(vg, left, y, InitStateColor(my_module->apply_favorite_state));
        left += spacing;
        //config_state
        Dot(vg, left, y, InitStateColor(my_module->config_state));
        left += spacing;
        //saved_preset_state
        if (my_module->restore_saved_preset){
            Dot(vg, left, y, InitStateColor(my_module->saved_preset_state));
        }
        left += spacing;
        //request_updates_state
        Dot(vg, left, y, InitStateColor(my_module->request_updates_state));
        left += spacing;
        //handshake
        if (my_module->heart_beating) {
            Dot(vg, left, y, InitStateColor(my_module->handshake));
        }
        left += spacing;
        // note (debugging)
        // auto n = format_string("%d", my_module->note);
        // nvgText(vg, left + 5.f, box.size.y - 1.5f, n.c_str(), nullptr);
    } else {
        auto co = InitStateColor(InitState::Complete);
        Dot(vg, left, y, gray_light, false);
        left += spacing;
        //device_output_state
        Dot(vg, left, y, co);
        left += spacing;
        // device_input_state
        Dot(vg, left, y, co);
        left += spacing;
        //eagan matrix
        Dot(vg, left, y, blue_light);
        left += spacing;
        // //device_state
        // Dot(vg, left, y, InitStateColor(my_module->device_hello_state));
        // left += spacing;
        //system_preset_state
        Dot(vg, left, y, co);
        left += spacing;
        //user_preset_state
        Dot(vg, left, y, co);
        left += spacing;
        //config_state
        Dot(vg, left, y, co);
        left += spacing;
        //saved_preset_state
        Dot(vg, left, y, co);
        left += spacing;
        //request_updates_state
        Dot(vg, left, y, co);
        left += spacing;
        //handshake
        Dot(vg, left, y, co);
    }
}

void Hc1ModuleWidget::drawLayer(const DrawArgs& args, int layer)
{
    ModuleWidget::drawLayer(args, layer);
    if (1 != layer) return;

    auto vg = args.vg;
    auto bold_font = GetPluginFontSemiBold();
    auto font_normal = GetPluginFontRegular();
    assert(FontOk(bold_font));
    assert(FontOk(font_normal));
    std::string text;
    SetTextStyle(vg, font_normal, RampGray(G_85), 12.f);
    if (my_module) {
        if (my_module->broken) {
            SetTextStyle(vg, bold_font, GetStockColor(StockColor::Fuchsia), 16.f);
            text = "[MIDI error - please wait]";
        } else
        if (InitState::Uninitialized == my_module->device_output_state) {
            text = "... looking for EM output ...";
        } else
        if (InitState::Uninitialized == my_module->device_input_state) {
            text = ".. looking for EM input ...";
        } else
        if (my_module->is_gathering_presets()) {
            text = format_string("... gathering %s preset %d ...", my_module->in_user_names ? "User" : "System", my_module->in_user_names ? my_module->user_presets.size() : my_module->system_presets.size());
        } else
        if (InitState::Uninitialized == my_module->system_preset_state) {
            text = "... preparing system presets ...";
        } else
        if (InitState::Uninitialized == my_module->user_preset_state) {
            text = "... preparing user presets ...";
        } else
        if (InitState::Uninitialized == my_module->apply_favorite_state) {
            text = "... preparing favorites ...";
        } else
        if (InitState::Uninitialized == my_module->config_state) {
            text = "... preparing preset details ...";
        } else
        if (InitState::Pending == my_module->config_state) {
            text = "... processing preset details ...";
        } else
        if (my_module->current_preset) {
            SetTextStyle(vg, bold_font, preset_name_color, 16.f);
            text = my_module->current_preset->name;
        } else {
            text = "";
        }
    } else {
        SetTextStyle(vg, bold_font, preset_name_color, 16.f);
        text = "My Amazing Preset";
    }
    CenterText(vg, box.size.x/2.f, 15.f, text.c_str(), nullptr);

    // MIDI animation
    if (my_module) {
        const float y = PRESET_BOTTOM + 1.75f;
        auto cx = PRESET_LEFT + fmodf(my_module->midi_send_count / 20.f, 320.f);
        CircularHalo(vg, cx, y, 2.f, 8.5f, purple_light);
        Circle(vg, cx, y, 1.25f, purple_light);

        cx = PRESET_LEFT + fmodf(my_module->midi_receive_count / 20.f, 320.f);
        CircularHalo(vg, cx, y, 2.f, 8.5f, green_light);
        Circle(vg, cx, y, 1.25f, green_light);
    }
    drawDSP(vg);
}

void drawPedalKnobAssignment(NVGcontext * vg, uint8_t ped, const char * text)
{
    float x = 0.f, y = KNOB_ROW_1;
    if (in_range<uint8_t>(ped, 12, 17)) {
        x = KNOB_LEFT + (ped-12) * KNOB_SPREAD;
    } else if (in_range<uint8_t>(ped, 20, 24)) {
        y = KNOB_ROW_2;
        x = RKNOB_LEFT + (ped-20) * KNOB_SPREAD;
    }
    if (x > 0.f) {
        nvgText(vg, x + RB_OFFSET - 7.5f, y - RB_VOFFSET + 4.f, text, nullptr);
    }
}

void drawPedalAssignment(NVGcontext* vg, float x, float y, char ped_char, uint8_t ped, uint8_t ped_value)
{
    auto text =  format_string("p%c %s", ped_char, ShortPedalAssignment(ped).c_str());
    nvgTextAlign(vg, NVGalign::NVG_ALIGN_LEFT);
    nvgText(vg, PRESET_RIGHT + 1.f, y, text.c_str(), nullptr);
    Line(vg, x, y+1, x, y+1 - ped_value/16.f, GetStockColor(StockColor::Sea_green_Dark), 1.5f);
}

void Hc1ModuleWidget::drawPedals(NVGcontext* vg, std::shared_ptr<rack::window::Font> font, bool stockPedals)
{    
    SetTextStyle(vg, font, RampGray(G_85), 10.f);
    if (stockPedals) {
        drawPedalAssignment(vg, box.size.x - 3.f, PRESET_BOTTOM - 18.f, '1', 64, 0);
        drawPedalAssignment(vg, box.size.x - 3.f, PRESET_BOTTOM - 4.5f, '2', 66, 0);
    } else {
        auto ped1 = my_module->ch15_cc_value[52];
        auto ped2 = my_module->ch15_cc_value[53];
        if (ped1 == ped2) {
            drawPedalKnobAssignment(vg, ped1, "1,2");
        } else {
            drawPedalKnobAssignment(vg, ped1, "1");
            drawPedalKnobAssignment(vg, ped2, "2");
        }
        drawPedalAssignment(vg, box.size.x - 3.f, PRESET_BOTTOM - 18.f, '1', ped1, my_module->ch0_cc_value[ped1]);
        drawPedalAssignment(vg, box.size.x - 3.f, PRESET_BOTTOM - 4.5f, '2', ped2, my_module->ch0_cc_value[ped2]);
    }
}

void Hc1ModuleWidget::drawExpanderConnector(NVGcontext* vg)
{
    if (my_module && my_module->expanders.any()) {
        bool right_expander = my_module->expanders.right();
        float left, right;
        if (right_expander) {
            left = box.size.x - 5.5f;
            right = box.size.x;
        } else {
            left = 0.f;
            right = 5.5f;
        }
        float y = 80.f;
        Line(vg,   left, y, right, y, COLOR_BRAND, 1.75f);
        Circle(vg, left, y, 2.5f, COLOR_BRAND);
    }
}

void Hc1ModuleWidget::draw(const DrawArgs& args)
{
    ModuleWidget::draw(args);
    auto vg = args.vg;

    bool stock = !my_module || !my_module->ready();
    auto rt = stock ? EM_Recirculator::Reverb : my_module->recirculatorType();

    drawExpanderConnector(vg);

    auto font = GetPluginFontRegular();
    if (FontOk(font)) {
#if defined SHOW_PRESET0
        if (my_module)
        {
            SetTextStyle(vg, font, orange_light, 9.f);
            auto text = my_module->preset0.describe_short();
            RightAlignText(vg, PRESET_RIGHT -2.f, PRESET_TOP - 15.f, text.c_str(), nullptr);
        }
#endif
        
        // recirculator
        { // todo: move lines to SVG
            Line(vg, RECIRC_BOX_LEFT, RECIRC_BOX_TOP, RECIRC_BOX_CENTER - (RECIRC_TITLE_WIDTH * .5f), RECIRC_BOX_TOP, RampGray(G_35), .5f);
            Line(vg, RECIRC_LIGHT_CENTER + 15.f, RECIRC_BOX_TOP, RECIRC_BOX_RIGHT, RECIRC_BOX_TOP, RampGray(G_35), .5f);
            Line(vg, RECIRC_BOX_LEFT,  RECIRC_BOX_TOP,    RECIRC_BOX_LEFT,  RECIRC_BOX_BOTTOM, RampGray(G_35), .5f);
            Line(vg, RECIRC_BOX_RIGHT, RECIRC_BOX_TOP,    RECIRC_BOX_RIGHT, RECIRC_BOX_BOTTOM, RampGray(G_35), .5f);
            Line(vg, RECIRC_BOX_LEFT,  RECIRC_BOX_BOTTOM, RECIRC_BOX_RIGHT, RECIRC_BOX_BOTTOM, RampGray(G_35), .5f);
            SetTextStyle(vg, font, RampGray(G_90), 12.f);
            CenterText(vg, RECIRC_BOX_CENTER, RECIRC_BOX_TOP + 2.f, RecirculatorName(rt), nullptr);
        }

        drawPedals(vg, font, stock);
    }

    drawStatusDots(vg);
    if (!my_module) {
        DrawLogo(args.vg, box.size.x*.5f - 120, 30.f, Overlay(COLOR_BRAND), 14.0);
    }
    DrawLogo(vg, box.size.x /2.f - 12.f, RACK_GRID_HEIGHT - ONE_HP, RampGray(G_90));
}

}