#include "HC-1.hpp"
#include "cc_param.hpp"
#include "../components.hpp"
#include "../misc.hpp"
#include "../small_push.hpp"
#include "../port.hpp"
#include "../tab_bar.hpp"
#include "../preset_widget.hpp"

namespace pachde {

struct MidiKnob : RoundSmallBlackKnob
{
    void step() override {
        RoundSmallBlackKnob::step();
        auto pq = dynamic_cast<CCParamQuantity*>(getParamQuantity());
        if (pq) {
            //bool dirty = pq->last_value != pq->getValue();
            pq->syncValue();
            // if (dirty) {
            //     this->fb->dirty = true;
            // }
        }
    }
};

const NVGcolor& StatusColor(StatusItem led) {
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

constexpr const float KNOB_LEFT   = 45.f;
constexpr const float KNOB_SPREAD = 54.f;
constexpr const float KNOB_ROW_1  = 42.f;
constexpr const float KNOB_ROW_2  = 85.f;

constexpr const float CV_COLUMN_OFFSET = 21.f;
constexpr const float CV_ROW_1 = KNOB_ROW_1 + 6.f;
constexpr const float CV_ROW_2 = KNOB_ROW_2 + 6.f;

constexpr const float PRESET_TOP = 135.f;
constexpr const float PRESET_LEFT = 15.f;

Hc1ModuleWidget::Hc1ModuleWidget(Hc1Module *module)
{
    my_module = module;
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/HC-1.svg")));

    status_light = createLightCentered<MediumLight<BlueLight>>(Vec(12.f, 12.f), my_module, Hc1Module::HEART_LIGHT);
    addChild(status_light);

    addChild(createInputCentered<ColorPort>(Vec(KNOB_LEFT                     - CV_COLUMN_OFFSET, CV_ROW_1), module, Hc1Module::M1_INPUT));
    addChild(createInputCentered<ColorPort>(Vec(KNOB_LEFT +       KNOB_SPREAD - CV_COLUMN_OFFSET, CV_ROW_1), module, Hc1Module::M2_INPUT));
    addChild(createInputCentered<ColorPort>(Vec(KNOB_LEFT + 2.f * KNOB_SPREAD - CV_COLUMN_OFFSET, CV_ROW_1), module, Hc1Module::M3_INPUT));
    addChild(createInputCentered<ColorPort>(Vec(KNOB_LEFT + 3.f * KNOB_SPREAD - CV_COLUMN_OFFSET, CV_ROW_1), module, Hc1Module::M4_INPUT));
    addChild(createInputCentered<ColorPort>(Vec(KNOB_LEFT + 4.f * KNOB_SPREAD - CV_COLUMN_OFFSET, CV_ROW_1), module, Hc1Module::M5_INPUT));
    addChild(createInputCentered<ColorPort>(Vec(KNOB_LEFT + 5.f * KNOB_SPREAD - CV_COLUMN_OFFSET, CV_ROW_1), module, Hc1Module::M6_INPUT));
    addChild(createInputCentered<ColorPort>(Vec(KNOB_LEFT                     - CV_COLUMN_OFFSET, CV_ROW_2), module, Hc1Module::R1_INPUT));
    addChild(createInputCentered<ColorPort>(Vec(KNOB_LEFT +       KNOB_SPREAD - CV_COLUMN_OFFSET, CV_ROW_2), module, Hc1Module::R2_INPUT));
    addChild(createInputCentered<ColorPort>(Vec(KNOB_LEFT + 2.f * KNOB_SPREAD - CV_COLUMN_OFFSET, CV_ROW_2), module, Hc1Module::R3_INPUT));
    addChild(createInputCentered<ColorPort>(Vec(KNOB_LEFT + 3.f * KNOB_SPREAD - CV_COLUMN_OFFSET, CV_ROW_2), module, Hc1Module::R4_INPUT));
    addChild(createInputCentered<ColorPort>(Vec(KNOB_LEFT + 4.f * KNOB_SPREAD - CV_COLUMN_OFFSET, CV_ROW_2), module, Hc1Module::RMIX_INPUT));

    addChild(createParamCentered<MidiKnob>(Vec(KNOB_LEFT                    , KNOB_ROW_1), module, Hc1Module::M1_PARAM));
    addChild(createParamCentered<MidiKnob>(Vec(KNOB_LEFT +       KNOB_SPREAD, KNOB_ROW_1), module, Hc1Module::M2_PARAM));
    addChild(createParamCentered<MidiKnob>(Vec(KNOB_LEFT + 2.f * KNOB_SPREAD, KNOB_ROW_1), module, Hc1Module::M3_PARAM));
    addChild(createParamCentered<MidiKnob>(Vec(KNOB_LEFT + 3.f * KNOB_SPREAD, KNOB_ROW_1), module, Hc1Module::M4_PARAM));
    addChild(createParamCentered<MidiKnob>(Vec(KNOB_LEFT + 4.f * KNOB_SPREAD, KNOB_ROW_1), module, Hc1Module::M5_PARAM));
    addChild(createParamCentered<MidiKnob>(Vec(KNOB_LEFT + 5.f * KNOB_SPREAD, KNOB_ROW_1), module, Hc1Module::M6_PARAM));
    addChild(createParamCentered<MidiKnob>(Vec(KNOB_LEFT                    , KNOB_ROW_2), module, Hc1Module::R1_PARAM));
    addChild(createParamCentered<MidiKnob>(Vec(KNOB_LEFT +       KNOB_SPREAD, KNOB_ROW_2), module, Hc1Module::R2_PARAM));
    addChild(createParamCentered<MidiKnob>(Vec(KNOB_LEFT + 2.f * KNOB_SPREAD, KNOB_ROW_2), module, Hc1Module::R3_PARAM));
    addChild(createParamCentered<MidiKnob>(Vec(KNOB_LEFT + 3.f * KNOB_SPREAD, KNOB_ROW_2), module, Hc1Module::R4_PARAM));
    addChild(createParamCentered<MidiKnob>(Vec(KNOB_LEFT + 4.f * KNOB_SPREAD, KNOB_ROW_2), module, Hc1Module::RMIX_PARAM));

    auto tab_bar = createWidget<TabBarWidget>(Vec(PRESET_LEFT, PRESET_TOP - 13.f));
    tab_bar->setSize(Vec(320.f, 13.f));
    tab_bar->addTab("Favorite");
    tab_bar->addTab("User");
    tab_bar->addTab("System");
    tab_bar->selectTab(2);
    addChild(tab_bar);

    presets.reserve(64);
    int x = PRESET_LEFT;
    int y = PRESET_TOP;
    for (int n = 0; n < 64; ++n) {
        auto p = createWidget<PresetWidget>(Vec(x, y));
        p->setPresetHolder(module);
        addChild(p);
        presets.push_back(p);
        x += p->box.size.x;
        if (0 == ((n + 1) % 8)) {
            x = PRESET_LEFT;
            y += p->box.size.y;
        }
    }

    auto pm = createWidget<PickMidi>(Vec(7.5f, box.size.y - 20.f));
    pm->describe("Choose Midi input");
    if (my_module) {
        pm->setMidiPort(&my_module->midiInput);
    }
    addChild(pm);

    pm = createWidget<PickMidi>(Vec(20.f, box.size.y - 20.f));
    pm->describe("Choose Midi output");
    if (my_module) {
        pm->setMidiPort(&my_module->midiOutput);
    }
    addChild(pm);

    auto pb = createWidget<SmallPush>(Vec(0, 0));
    pb->box.size.x = 14.f;
    pb->box.size.y = 14.f;
    pb->center(Vec(45.f, box.size.y -10.f));
    if (module) {
        #ifdef ARCH_MAC
            pb->describe("Send test Note\nCmd+click = Note off");
        #else
            pb->describe("Send test Note\nCtrl+click = Note off");
        #endif
        pb->onClick([module](bool ctrl, bool shift) {
            if (ctrl) {
                module->sendNoteOff(0, 60, 0);
            } else {
                module->sendNoteOn(0, 60, 64);
            }
        });
    }
    addChild(pb);

    // pb = createWidget<SmallPush>(Vec(0,0));
    // pb->box.size.x = 14.f;
    // pb->box.size.y = 14.f;
    // pb->center(Vec(box.size.x - 7.5f - 7.f, 120.f + 7.f));
    // pb->describe("Previous preset");
    // if (module) {
    //     pb->onClick([module](bool, bool) {
    //         DebugLog("[PREVIOUS]");
    //     });
    // }
    // addChild(pb);

    // pb = createWidget<SmallPush>(Vec(0,0));
    // pb->box.size.x = 14.f;
    // pb->box.size.y = 14.f;
    // pb->center(Vec( box.size.x - 7.5f - 7.f, 140.f + 7.f));
    // pb->describe("Next preset");
    // if (module) {
    //     pb->onClick([module](bool, bool) {
    //         DebugLog("[NEXT]");
    //     });
    // }
    // addChild(pb);
}

void Hc1ModuleWidget::step()
{
    ModuleWidget::step();
    if (my_module)
    {
        auto co = my_module->isEaganMatrix() ? my_module->ledColor : red_light;
        if (!IS_SAME_COLOR(co, status_light->baseColors[0])) {
            status_light->baseColors[0] = co;
            my_module->getLight(Hc1Module::HEART_LIGHT).setBrightness(1.f);
            DirtyEvent e;
            status_light->onDirty(e);
        }
        if (!have_preset_widgets && my_module->hasPresets()) {
            populatePresetWidgets();
        } else if (have_preset_widgets && !my_module->hasPresets()) {
            clearPresetWidgets();
        }
    }
}

void Hc1ModuleWidget::clearPresetWidgets() {
    for (auto n = 0; n < 64; ++n) {
        reinterpret_cast<PresetWidget*>(presets[n])->setPreset(nullptr);
    }
    have_preset_widgets = false;
}


void Hc1ModuleWidget::populatePresetWidgets()
{
    if (!my_module) return;
    assert(!have_preset_widgets);
    int nth = 0;
    for (auto mp: my_module->presets) {
        auto p = reinterpret_cast<PresetWidget*>(presets[nth]);
        p->setPreset(mp);
        if (64 == ++nth) break;
    }
    have_preset_widgets = true;
}

void Hc1ModuleWidget::drawLayer(const DrawArgs& args, int layer)
{
    ModuleWidget::drawLayer(args, layer);
    if (1 == layer) {
        auto vg = args.vg;
        auto font = GetPluginFontSemiBold();
        if (FontOk(font)) {
            SetTextStyle(vg, font, preset_name_color, 16.f);
            if (my_module) {
                std::string text = my_module->broken
                    ? "[MIDI error-Please wait for recovery...]"
                    : my_module->is_gathering_presets()
                        ? format_string("[Gathering Presets... %d]", my_module->presets.size())
                        : my_module->preset0.name();
                CenterText(vg, box.size.x/2.f, 20.f, text.c_str(), nullptr);
            } else {
                CenterText(vg, box.size.x/2.f, 20.f, "(Preset Name)", nullptr);
            }
        }

        // FavoriteWidget to be
        StrokeHeart(vg, box.size.x - (12.f + 3.5f), 4.5f, 12.f, PORT_PINK, .75f);

        // DSP status
        auto h = 10.f;
        auto w = 2.5f;
        auto x = box.size.x - 7.5f - 3.f*w - 2;
        auto y = box.size.y - 20.f - h;
        FillRect(vg, x-1.25f, y-1.25f, w*3+5.f, h+5.f, RampGray(G_30));
        uint8_t tdsp[] = {65, 30, 75};
        uint8_t* pdsp = (my_module && my_module->hasPresets()) ? &my_module->dsp[0] : &tdsp[0];
        for (auto n = 0; n < 3; n++) {
            auto pct = pdsp[n];
            auto co = pct < 85 ? green_light : red_light;
            auto bh = h * (pct / 100.f);
            FillRect(vg, x, y + h - bh, w, bh, co);
            x += w + 1;
        }
    }
}

void Hc1ModuleWidget::draw(const DrawArgs& args)
{
    ModuleWidget::draw(args);
    auto vg = args.vg;

    if (my_module) {
        Circle(vg, 12.f + (my_module->tick_tock ? 1.5f : -1.5f), 26.f, 1.25f, COLOR_MAGENTA);
    }
    if (my_module && my_module->anyPending()) {
        // raw MIDI animation
        Circle(vg, PRESET_LEFT + ((my_module->midi_count / 50) % 320), 3.f, 2.f, blue_green_light);
    }
    auto font = GetPluginFontRegular();
    if (FontOk(font)) {
        std::string device_name;
        if (my_module) {
            device_name = FilterDeviceName(my_module->deviceName());
        } else {
            device_name = "<Eagan Matrix Device>";
        }
        if (device_name.empty()) {
            device_name = "(no Eagan Matrix device found)";
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

    // labels
    font = GetPluginFontSemiBold();
    if (FontOk(font)) {
        SetTextStyle(vg, font, RampGray(G_90), 12.f);
        float y = KNOB_ROW_1 + 22.f;
        if (my_module && my_module->hasConfig()) {
            CenterText(vg, KNOB_LEFT, y, my_module->preset0.macro[0].c_str(), nullptr);
            CenterText(vg, KNOB_LEFT +       KNOB_SPREAD, y, my_module->preset0.macro[1].c_str(), nullptr);
            CenterText(vg, KNOB_LEFT + 2.f * KNOB_SPREAD, y, my_module->preset0.macro[2].c_str(), nullptr);
            CenterText(vg, KNOB_LEFT + 3.f * KNOB_SPREAD, y, my_module->preset0.macro[3].c_str(), nullptr);
            CenterText(vg, KNOB_LEFT + 4.f * KNOB_SPREAD, y, my_module->preset0.macro[4].c_str(), nullptr);
            CenterText(vg, KNOB_LEFT + 5.f * KNOB_SPREAD, y, my_module->preset0.macro[5].c_str(), nullptr);
        } else {
            CenterText(vg, KNOB_LEFT, y, "i", nullptr);
            CenterText(vg, KNOB_LEFT +       KNOB_SPREAD, y, "ii", nullptr);
            CenterText(vg, KNOB_LEFT + 2.f * KNOB_SPREAD, y, "iii", nullptr);
            CenterText(vg, KNOB_LEFT + 3.f * KNOB_SPREAD, y, "iv", nullptr);
            CenterText(vg, KNOB_LEFT + 4.f * KNOB_SPREAD, y, "v", nullptr);
            CenterText(vg, KNOB_LEFT + 5.f * KNOB_SPREAD, y, "vi", nullptr);
        }
        auto rt = my_module ? my_module->recirculatorType() : EM_Recirculator::Reverb;
        y = KNOB_ROW_2 + 22.f;
        CenterText(vg, KNOB_LEFT, y, RecirculatorParameterName(rt, 1).c_str(), nullptr);
        CenterText(vg, KNOB_LEFT + KNOB_SPREAD, y, RecirculatorParameterName(rt, 2).c_str(), nullptr);
        CenterText(vg, KNOB_LEFT + KNOB_SPREAD * 2.f, y, RecirculatorParameterName(rt, 3).c_str(), nullptr);
        CenterText(vg, KNOB_LEFT + KNOB_SPREAD * 3.f, y, RecirculatorParameterName(rt, 4).c_str(), nullptr);
        CenterText(vg, KNOB_LEFT + KNOB_SPREAD * 4.f, y, "Mix", nullptr);

        nvgTextAlign(vg, NVG_ALIGN_LEFT);
        nvgText(vg, KNOB_LEFT + KNOB_SPREAD * 4.f + 23.f, y - 15.f, RecirculatorName(rt).c_str(), nullptr);
    }

    // status
    if (my_module) {
        float left = 60.f;
        float spacing = 6.25f;
        float y = box.size.y - 7.5f;
        bool on;
        Dot(vg, left, y, my_module->is_eagan_matrix ? green_light : orange_light);

        on = my_module->hasDevice();
        Dot(vg, left + spacing, y, on ? blue_light : orange_light, on);

        on = my_module->handshakePending();
        Dot(vg, left + 2.f * spacing, y, on ? orange_light : blue_light);

        on = my_module->in_user_names || my_module->in_sys_names || my_module->hasPresets();
        Dot(vg, left + 3.f * spacing, y, 
              my_module->broken        ? red_light
            : my_module->hasPresets()  ? blue_light
            : my_module->in_user_names ? yellow_light
            : my_module->in_sys_names  ? orange_light
            : gray_light
            , on
            );

        on = my_module->notesOn > 0;
        Dot(vg, left + 4.f * spacing, y, on ? purple_light : gray_light, on);
        Dot(vg, left + 5.f * spacing, y, (my_module->broken || my_module->stateError()) ? red_light : my_module->anyPending() ? yellow_light : green_light);
    }

    DrawLogo(vg, box.size.x /2.f - 12.f, RACK_GRID_HEIGHT - ONE_HP, RampGray(G_90));
}

// Add options to your module's menu here
// void appendContextMenu(Menu *menu) override
//{
//}

} //pachde