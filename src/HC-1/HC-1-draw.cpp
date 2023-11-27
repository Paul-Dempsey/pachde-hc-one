#include "HC-1.hpp"
#include "HC-1-layout.hpp"

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
    if (my_module) { return; }

    float left = STATUS_LEFT;
    float y = box.size.y - 7.5f;

    // note
    Dot(vg, 40.f, y, gray_light, false);

    auto co = InitStateColor(InitState::Complete);
    //device_output_state
    Dot(vg, left, y, co);
    left += STATUS_SPREAD;
    // device_input_state
    Dot(vg, left, y, co);
    left += STATUS_SPREAD;
    //eagan matrix
    Dot(vg, left, y, blue_light);
    left += STATUS_SPREAD;
    //device_hello_state
    Dot(vg, left, y, blue_light);
    left += STATUS_SPREAD;
    //system_preset_state
    Dot(vg, left, y, co);
    left += STATUS_SPREAD;
    //user_preset_state
    Dot(vg, left, y, co);
    left += STATUS_SPREAD;
    //config_state
    Dot(vg, left, y, co);
    left += STATUS_SPREAD;
    //saved_preset_state
    Dot(vg, left, y, co);
    left += STATUS_SPREAD;
    //request_updates_state
    Dot(vg, left, y, co);
    left += STATUS_SPREAD;
    //handshake
    Dot(vg, left, y, co);
}

std::string Hc1ModuleWidget::getBannerText(NVGcontext * vg, std::shared_ptr<rack::window::Font> normal, std::shared_ptr<rack::window::Font> bold)
{
    if (!my_module) return "";

    if (my_module->ready()) {
        if (my_module->current_preset) {
            SetTextStyle(vg, bold, preset_name_color, 16.f);
            return my_module->current_preset->name;
        } else {
            return "<no preset>";
        }        
    }

    if (my_module->broken) {
        SetTextStyle(vg, bold, GetStockColor(StockColor::Fuchsia), 16.f);
        return "[Error - please wait]";
    }

    if (my_module->phase_pause) {
        auto next = NextPhase(my_module->current_phase);
        while (InitPhase::Done != next && my_module->finished(next)) {
            next = NextPhase(next);
        }
        return format_string("Pause before %s ...", PhaseName(next));
    }

    switch (my_module->current_phase) {
    case InitPhase::None: return "initializing ...";
    case InitPhase::DeviceOutput:
            if (!my_module->device_claim.empty()) {
                MidiDeviceConnectionInfo info;
                if (info.parse(my_module->device_claim)) {
                    std::string text = "looking for ";
                    text.append(info.input_device_name);
                    text.append(" ...");
                    return text;
                }
            }
            return "looking for an Eagan Matrix device ...";
    case InitPhase::DeviceInput: return format_string("looking for %s input ...", my_module->connection->info.input_device_name.c_str());
    case InitPhase::DeviceHello: return "Waiting for EM handshake response ...";
    case InitPhase::DeviceConfig: return "Processing EM device configuration ...";
    case InitPhase::CachedPresets: return "Loading cached presets";
    case InitPhase::UserPresets: return format_string("Gathering User preset %d of %d slots...", my_module->user_presets.size(), my_module->slot_count);
    case InitPhase::SystemPresets: return format_string("Gathering System preset %d of %d slots ...", my_module->system_presets.size(), my_module->slot_count);
    case InitPhase::Favorites: return "Processing favorites ...";
    case InitPhase::SavedPreset: return "Restoring saved preset";
    case InitPhase::PresetConfig: return "Processing preset details ...";
    case InitPhase::RequestUpdates: return "Requesting device updates";
    case InitPhase::Heartbeat: return "Waiting for EM heartbeat response ...";
    case InitPhase::Done: return "Ready";
    }
    return "...";
}

void Hc1ModuleWidget::drawLayer(const DrawArgs& args, int layer)
{
    ModuleWidget::drawLayer(args, layer);
    if (1 != layer) return;

    auto vg = args.vg;
    auto bold_font = GetPluginFontSemiBold();
    assert(FontOk(bold_font));
    auto normal_font = GetPluginFontRegular();
    assert(FontOk(normal_font));

    if (my_module) {
        SetTextStyle(vg, normal_font, RampGray(G_85), 12.f);
        auto text = getBannerText(vg, normal_font, bold_font);
        CenterText(vg, box.size.x/2.f, 15.f, text.c_str(), nullptr);
    } else {
        SetTextStyle(vg, bold_font, preset_name_color, 16.f);
        CenterText(vg, box.size.x/2.f, 15.f, "My Amazing Preset", nullptr);
    }

    // MIDI animation
    if (my_module) {
        float y = PRESET_BOTTOM + 1.75f;
        float cx;
        cx = PRESET_LEFT + static_cast<float>((my_module->midi_receive_count / 15) % 320);
        CircularHalo(vg, cx, y, 2.f, 8.5f, green_light);
        Circle(vg, cx, y, 1.25f, green_light);
        y += .75;
        cx = PRESET_LEFT + static_cast<float>((my_module->midi_send_count / 15) % 320);
        CircularHalo(vg, cx, y, 2.f, 8.5f, purple_light);
        Circle(vg, cx, y, 1.25f, purple_light);

        // No note display not until we can track bend and pressure
        // if (my_module->notesOn) {
        //     y = PRESET_TOP - 16.25f;
        //     cx = PRESET_LEFT + (my_module->note * 320.f / 128.f);
        //     CircularHalo(vg, cx, y, 2.f, 8.5f, yellow_light);
        //     Circle(vg, cx, y, 1.25f, yellow_light);
        // }
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

void Hc1ModuleWidget::drawPedalAssignment(
    NVGcontext* vg, float x, float y,
    char ped_char, uint8_t ped, uint8_t ped_value)
{
    nvgTextAlign(vg, NVGalign::NVG_ALIGN_LEFT);
    auto text = format_string("p%c %s", ped_char, ShortPedalAssignment(ped).c_str());
    nvgText(vg, x, y, text.c_str(), nullptr);
    y += 3.f;
    Line(vg, x, y, x + ((ped_value * (box.size.x - x - 3.f)) / 127.f), y,
        GetStockColor(StockColor::Sea_green_Dark), 2.25f);
}

void Hc1ModuleWidget::drawPedals(NVGcontext* vg, std::shared_ptr<rack::window::Font> font, bool stockPedals)
{    
    SetTextStyle(vg, font, GetStockColor(StockColor::Gold), 10.f);
    if (stockPedals) {
        drawPedalAssignment(vg, PRESET_RIGHT + 1.f, PRESET_BOTTOM - 19.f, '1', 64, 0);
        drawPedalAssignment(vg, PRESET_RIGHT + 1.f, PRESET_BOTTOM - 4.5f, '2', 66, 0);
    } else {
        auto ped1 = my_module->em.pedal1.cc;
        auto ped2 = my_module->em.pedal2.cc;
        if (ped1 == ped2) {
            drawPedalKnobAssignment(vg, ped1, "1,2");
        } else {
            drawPedalKnobAssignment(vg, ped1, "1");
            drawPedalKnobAssignment(vg, ped2, "2");
        }
        drawPedalAssignment(vg, PRESET_RIGHT + 1.f, PRESET_BOTTOM - 19.f, '1', ped1, my_module->em.pedal1.value);
        drawPedalAssignment(vg, PRESET_RIGHT + 1.f, PRESET_BOTTOM - 4.5f, '2', ped2, my_module->em.pedal2.value);
    }
}

void Hc1ModuleWidget::draw(const DrawArgs& args)
{
    ModuleWidget::draw(args);
    auto vg = args.vg;

    bool stock = !module || !my_module->ready();
    auto rt = stock ? EM_Recirculator::Reverb : my_module->recirculatorType();

    auto font = GetPluginFontRegular();
    if (FontOk(font)) {
#if defined SHOW_PRESET0
        if (module)
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
        // if (module) {
        //     auto t = format_string("%d", my_module->midi_send_count);
        //     RightAlignText(vg, box.size.x - 2.f, box.size.y *.5f, t.c_str(), nullptr);
        // }
#if not defined NDEBUG
        Line(vg, PRESET_LEFT, PRESET_TOP - 15.f, PRESET_LEFT + 60, PRESET_TOP - 15.f, GetStockColor(StockColor::pachde_blue_medium), 2.f);
#endif
    }

    drawStatusDots(vg);
    if (!module) {
        DrawLogo(args.vg, box.size.x*.5f - 120, 30.f, Overlay(COLOR_BRAND), 14.0);
    }
}

}