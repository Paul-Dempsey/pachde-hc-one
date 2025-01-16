// Copyright (C) Paul Chase Dempsey
#include "HC-1.hpp"
#include "HC-1-layout.hpp"
#include "../misc.hpp"
#include "../widgets/cc_param.hpp"
#include "../widgets/components.hpp"
#include "../widgets/indicator_widget.hpp"
#include "../widgets/port.hpp"
#include "../widgets/small_push.hpp"

namespace pachde {

Hc1ModuleWidget::Hc1ModuleWidget(Hc1Module* module)
{
    my_module = module;
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/HC-1.svg")));
    if (my_module) {
        tab = my_module->tab;
        page = my_module->getTabPage(tab);
        my_module->dsp_client = this;
    }
    createUi();
    if (my_module) {
        my_module->subscribeHcEvents(this);
    }
}

Hc1ModuleWidget::~Hc1ModuleWidget()
{
    dsp_widget = nullptr;
    if (my_module) {
        my_module->dsp_client = NULL;
        my_module->unsubscribeHcEvents(this);
        my_module = nullptr;
    }
}

const std::string Hc1ModuleWidget::macroName(int m)
{
    bool dyn = my_module && my_module->ready();
    if (dyn) {
        return my_module->preset0.macro[m];
    }
    switch (m) {
    case 0: return "i";
    case 1: return "ii";
    case 2: return "iii";
    case 3: return "iv";
    case 4: return "v";
    case 5: return "vi";
    default: return "???";
    }
}

void Hc1ModuleWidget::createPresetGrid()
{
    tab_bar = createWidget<TabBarWidget>(Vec(PRESET_LEFT, PRESET_TOP - 13.f));
    tab_bar->setSize(Vec(PRESET_WIDTH, 13.f));
    tab_bar->addTab("User", PresetTab::User, nullptr);
    tab_bar->addTab("Favorite", PresetTab::Favorite, 
        [=](Menu* menu) { 
            addFavoritesMenu(menu);
        });
    tab_bar->addTab("System", PresetTab::System, [=](Menu* menu){
        menu->addChild(createMenuItem("System presets", "", [](){}, true));
        menu->addChild(new MenuSeparator);
        addSystemMenu(menu);
        });
    tab_bar->layout();
    tab_bar->selectTab(tab);
    addChild(tab_bar);

    presets.reserve(24);
    float x = PRESET_LEFT;
    float y = PRESET_TOP;
    for (int n = 0; n < 24; ++n) {
        auto p = createWidget<PresetWidget>(Vec(x, y));
        p->setPresetHolder(this);
        addChild(p);
        presets.push_back(p);
        x += p->box.size.x;
        if (0 == ((n + 1) % 3)) {
            x = PRESET_LEFT;
            y += p->box.size.y;
        }
    }
}

void Hc1ModuleWidget::createPresetPrevNext()
{
    float y = 124.f;
    auto w = createWidgetCentered<SmallSquareButton>(Vec(RIGHT_COLUMN_BUTTONS - 7.f, y));
    w->describe("Previous preset\n  Shift+Click by 10\n  Ctrl+Click for device order");
    if (my_module) {
        w->setHandler([=](bool ctrl, bool shift){
            if (ctrl) {
                my_module->sendControlChange(EM_SettingsChannel, 109, 52);
            } else {
                toRelativePreset(shift ? -10 : -1);
            }
        });
    }
    addChild(w);
    addChild(createStaticTextLabel<StaticTextLabel>(Vec(Vec(RIGHT_COLUMN_BUTTONS - 7.f, y + 5.f)), 25.f, "-", TextAlignment::Center, 10.f, true));

    w = createWidgetCentered<SmallSquareButton>(Vec(RIGHT_COLUMN_BUTTONS + 7.f, y));
    w->describe("Next preset\n  Shift+Click by 10\n  Ctrl+Click for device order");
    if (my_module) {
        w->setHandler([=](bool ctrl, bool shift){
            if (ctrl) {
                my_module->sendControlChange(EM_SettingsChannel, 109, 53);
            } else {
               toRelativePreset(shift ? 10 : 1);
            }
        });
    }
    addChild(w);
    addChild(createStaticTextLabel<StaticTextLabel>(Vec(Vec(RIGHT_COLUMN_BUTTONS + 7.f, y + 5.f)), 25.f, "+", TextAlignment::Center, 10.f, true));
    y += 12.f;

    addChild(createStaticTextLabel<StaticTextLabel>(Vec(Vec(RIGHT_COLUMN_BUTTONS, y)), 25.f, "Preset", TextAlignment::Center, 10.f, true));
}

void Hc1ModuleWidget::createPresetPaging()
{
    // todo: set text only when page changes
    addChild(createDynamicLabel<DynamicTextLabel>(
        Vec(RIGHT_COLUMN_BUTTONS, PRESET_TOP - 9.f), 80.f,
        [=]() { return format_string("%d of %d", 1 + page, my_module ? 1 + my_module->getPresets(tab).size()/24 : 1);  },
        TextAlignment::Center, 10.f, false
        ));

    page_up = createWidgetCentered<SquareButton>(Vec(RIGHT_COLUMN_BUTTONS, PRESET_TOP + 12.f));
    page_up->setSymbol(SquareButtonSymbol::Up);
    page_up->onClick([this](){ pageUp(); });
    page_up->describe("Page up");
    addChild(page_up);

    page_down = createWidgetCentered<SquareButton>(Vec(RIGHT_COLUMN_BUTTONS, PRESET_TOP + 29.f));
    page_down->setSymbol(SquareButtonSymbol::Down);
    page_down->onClick([this](){ pageDown(); });
    page_down->describe("Page down");
    addChild(page_down);
}

void Hc1ModuleWidget::createRoundingLEDs()
{
    float y = 108.f;
    float x = RIGHT_COLUMN_BUTTONS - (2.f * LIGHT_SPREAD) + 1;
    addChild(createLight<TinySimpleLight<RedLight>>(Vec(x, y), my_module, Hc1lt::ROUND_Y_LIGHT)); x += LIGHT_SPREAD;
    addChild(createLight<TinySimpleLight<RedLight>>(Vec(x, y), my_module, Hc1lt::ROUND_INITIAL_LIGHT)); x += LIGHT_SPREAD;
    addChild(createLight<TinySimpleLight<RedLight>>(Vec(x, y), my_module, Hc1lt::ROUND_LIGHT)); x += LIGHT_SPREAD;
    addChild(createLight<TinySimpleLight<RedLight>>(Vec(x, y), my_module, Hc1lt::ROUND_RELEASE_LIGHT));
}

#ifdef TRANSPOSE_BUTTONS
void Hc1ModuleWidget::createTranspose()
{
    // middle C
    addChild(createDynamicLabel<DynamicTextLabel>(
        Vec(RIGHT_COLUMN_BUTTONS, PRESET_TOP + 47.f), PANEL_WIDTH - PRESET_RIGHT,
        [=](){ return format_string("%d", my_module ? my_module->middle_c : 60); },
        TextAlignment::Center, 10.f, false
        ));

    auto y = PRESET_TOP + 65.f;
    auto x = RIGHT_COLUMN_BUTTONS;
    auto pb = createWidgetCentered<SmallPush>(Vec(x, y));
    if (my_module) {
#ifdef ARCH_MAC
        pb->describe("Octave up\nCmd+Click for one semitone");
#else
        pb->describe("Octave up\nCtrl+Click for one semitone");
#endif
        pb->onClick([=](bool ctrl, bool shift) {
            my_module->middle_c += ctrl ? 1 : 12;
            my_module->sendControlChange(EM_SettingsChannel, EMCC_MiddleC, my_module->middle_c);
        });
    }
    addChild(pb);
    addChild(createLightCentered<TinySimpleLight<GreenLight>>(Vec(x, y), my_module, Hc1lt::TRANSPOSE_UP_LIGHT));

    y += 11.5f;

    pb = createWidgetCentered<SmallPush>(Vec(x, y));
    if (my_module) {
        pb->describe("Transpose none");
        pb->onClick([=](bool ctrl, bool shift) {
            my_module->middle_c = 60;
            my_module->sendControlChange(EM_SettingsChannel, EMCC_MiddleC, my_module->middle_c);
        });
    }
    addChild(pb);
    addChild(createLightCentered<TinySimpleLight<GreenLight>>(Vec(x, y), my_module, Hc1lt::TRANSPOSE_NONE_LIGHT));
    y += 11.5f;

    pb = createWidgetCentered<SmallPush>(Vec(x, y));
    if (my_module) {
#ifdef ARCH_MAC
        pb->describe("Octave down\nCmd+Click for one semitone");
#else
        pb->describe("Octave down\nCtrl+Click for one semitone");
#endif
        pb->onClick([=](bool ctrl, bool shift) {
            my_module->middle_c -= ctrl ? 1 : 12;
            my_module->sendControlChange(EM_SettingsChannel, EMCC_MiddleC, my_module->middle_c);
        });
    }
    addChild(pb);
    addChild(createLightCentered<TinySimpleLight<GreenLight>>(Vec(x, y), my_module, Hc1lt::TRANSPOSE_DOWN_LIGHT));
}

#endif

void Hc1ModuleWidget::createMidiSelection()
{
    em_picker = createWidget<EMPicker>(Vec(7.5f, box.size.y - 16.f));
    em_picker->describe("Choose MIDI device");
    if (module) {
        em_picker->setCallback(my_module);
        em_picker->setConnection(my_module->connection); // probably don't have it yet, but why not?
    }
    addChild(em_picker);
}

void Hc1ModuleWidget::createDeviceDisplay()
{
    auto co = GetStockColor(StockColor::pachde_blue_light);
    auto y = box.size.y - 12.f;
    auto center = box.size.x*.5f;

    // hardware model
    addChild(hardware_label = createStaticTextLabel<StaticTextLabel>(
        Vec(center -66.5f, y), 50.f, "<device model>", 
        TextAlignment::Right, 10.f, false, co));

    // device name
    addChild(device_label = createStaticTextLabel<StaticTextLabel>(
        Vec(center + 16.5f, y), 120.f, "<Eagan Matrix Device>",
        TextAlignment::Left, 10.f, false, co
        ));

    // firmware version
    addChild(firmware_label = createStaticTextLabel<StaticTextLabel>(
        Vec(box.size.x - 60.f,  y), 60.f - 7.5f, "v0.0",
        TextAlignment::Right, 10.f, false, co));
}

void Hc1ModuleWidget::createTestNote()
{
    auto pb = createWidgetCentered<SmallPush>(Vec(31.f, box.size.y -7.5f));
    if (my_module) {
        #ifdef ARCH_MAC
            pb->describe("Send test Note\nCmd+click = Note off");
        #else
            pb->describe("Send test Note\nCtrl+click = Note off");
        #endif
        pb->onClick([=](bool ctrl, bool shift) {
            if (ctrl) {
                my_module->sendNoteOff(0, 60, 0);
            } else {
                my_module->sendNoteOn(0, 60, 64);
            }
        });
    }
    addChild(pb);
}

void Hc1ModuleWidget::createStatusDots()
{
    if (!my_module) { return; }

    float left = STATUS_LEFT;
    float y = box.size.y - 7.5f;
    //notes_on
    addChild(createIndicatorCentered(
        40.f, y, "Note",
        [=]() -> const NVGcolor & { return (my_module->notesOn ? purple_light : gray_light); },
        [=]() -> bool { return my_module->notesOn; }));
    // is_eagan_matrix
    addChild(createIndicatorCentered(left, y, "Eagan Matrix",
        [=]() -> const NVGcolor & { return my_module->isEaganMatrix() ? blue_light : yellow_light; }));
    left += STATUS_SPREAD;
    addChild(createIndicatorCentered(left, y, "Midi output device",
        [=]() -> const NVGcolor & { return InitStateColor(my_module->phaseState(InitPhase::DeviceOutput)); },
        [=]() -> std::string { return InitStateName(my_module->phaseState(InitPhase::DeviceOutput)); }));
    left += STATUS_SPREAD;
    addChild(createIndicatorCentered(left, y, "Midi input device",
        [=]() -> const NVGcolor & { return InitStateColor(my_module->phaseState(InitPhase::DeviceInput)); },
        [=]() -> std::string { return InitStateName(my_module->phaseState(InitPhase::DeviceInput)); }));
    left += STATUS_SPREAD;
    addChild(createIndicatorCentered(left, y, "EM device 'hello'",
        [=]() -> const NVGcolor & { return InitStateColor(my_module->phaseState(InitPhase::DeviceHello)); },
        [=]() -> std::string { return InitStateName(my_module->phaseState(InitPhase::DeviceHello)); }));
    left += STATUS_SPREAD;
    addChild(createIndicatorCentered(left, y, "EM device configuration",
        [=]() -> const NVGcolor & { return InitStateColor(my_module->phaseState(InitPhase::DeviceConfig)); },
        [=]() -> std::string { return InitStateName(my_module->phaseState(InitPhase::DeviceConfig)); }));
    left += STATUS_SPREAD;
    addChild(createIndicatorCentered(left, y, "User presets",
        [=]()->const NVGcolor& { return InitStateColor(my_module->phaseState(InitPhase::UserPresets)); },
        [=]() -> std::string { return InitStateName(my_module->phaseState(InitPhase::UserPresets)); }));
    left += STATUS_SPREAD;
    addChild(createIndicatorCentered(left, y, "System presets",
        [=]()->const NVGcolor& { return InitStateColor(my_module->phaseState(InitPhase::SystemPresets)); },
        [=]() -> std::string { return InitStateName(my_module->phaseState(InitPhase::SystemPresets)); }));
    left += STATUS_SPREAD;
    addChild(createIndicatorCentered(left, y, "Favorites",
        [=]()->const NVGcolor& { return InitStateColor(my_module->phaseState(InitPhase::Favorites)); },
        [=]() -> std::string { return InitStateName(my_module->phaseState(InitPhase::Favorites)); }));
    left += STATUS_SPREAD;
    addChild(createIndicatorCentered(left, y, "Configuration",
        [=]()->const NVGcolor& { return InitStateColor(my_module->phaseState(InitPhase::PresetConfig)); },
        [=]() -> std::string { return InitStateName(my_module->phaseState(InitPhase::PresetConfig)); }));
    left += STATUS_SPREAD;
    addChild(createIndicatorCentered(left, y, "Saved preset",
        [=]()->const NVGcolor& { return InitStateColor(my_module->phaseState(InitPhase::SavedPreset)); },
        [=]() -> std::string { return InitStateName(my_module->phaseState(InitPhase::SavedPreset)); }));
    left += STATUS_SPREAD;
    addChild(createIndicatorCentered(left, y, "Request updates",
        [=]()->const NVGcolor& { return InitStateColor(my_module->phaseState(InitPhase::RequestUpdates)); },
        [=]() -> std::string { return InitStateName(my_module->phaseState(InitPhase::RequestUpdates)); }));
    left += STATUS_SPREAD;
    addChild(createIndicatorCentered(left, y, "Heartbeat",
        [=]()->const NVGcolor& { return InitStateColor(my_module->phaseState(InitPhase::Heartbeat)); },
        [=]() -> std::string { return InitStateName(my_module->phaseState(InitPhase::Heartbeat)); }));
}

void Hc1ModuleWidget::createUi()
{
    status_light = createLightCentered<MediumLight<BlueLight>>(Vec(12.f, 12.f), my_module, Hc1Module::HEART_LIGHT);
    addChild(status_light);

    favorite = createWidget<FavoriteWidget>(Vec(box.size.x - (12.f + 6.f), 6.f));
    favorite->setPresetHolder(this);
    addChild(favorite);

    createPresetGrid();
    createPresetPaging();
    createRoundingLEDs();
    createPresetPrevNext();

    addChild(createInputCentered<ColorPort>(Vec(KNOB_LEFT                     - CV_COLUMN_OFFSET, CV_ROW_1), my_module, Hc1in::M1_INPUT));
    addChild(createInputCentered<ColorPort>(Vec(KNOB_LEFT +       KNOB_SPREAD - CV_COLUMN_OFFSET, CV_ROW_1), my_module, Hc1in::M2_INPUT));
    addChild(createInputCentered<ColorPort>(Vec(KNOB_LEFT + 2.f * KNOB_SPREAD - CV_COLUMN_OFFSET, CV_ROW_1), my_module, Hc1in::M3_INPUT));
    addChild(createInputCentered<ColorPort>(Vec(KNOB_LEFT + 3.f * KNOB_SPREAD - CV_COLUMN_OFFSET, CV_ROW_1), my_module, Hc1in::M4_INPUT));
    addChild(createInputCentered<ColorPort>(Vec(KNOB_LEFT + 4.f * KNOB_SPREAD - CV_COLUMN_OFFSET, CV_ROW_1), my_module, Hc1in::M5_INPUT));
    addChild(createInputCentered<ColorPort>(Vec(KNOB_LEFT + 5.f * KNOB_SPREAD - CV_COLUMN_OFFSET, CV_ROW_1), my_module, Hc1in::M6_INPUT));

    addChild(createStaticTextLabel(Vec(KNOB_LEFT + 6.f * KNOB_SPREAD - CV_COLUMN_OFFSET, CV_ROW_1 - LABEL_OFFSET), 40.f, "Ready", TextAlignment::Center, 9.f));
    addChild(createColorOutputCentered(KNOB_LEFT + 6.f * KNOB_SPREAD - CV_COLUMN_OFFSET, CV_ROW_1, Hc1out::READY_TRIGGER, GetStockColor(StockColor::pachde_default_port), my_module));

    addChild(createInputCentered<ColorPort>(Vec(RKNOB_LEFT                     - CV_COLUMN_OFFSET, CV_ROW_2), my_module, Hc1in::R1_INPUT));
    addChild(createInputCentered<ColorPort>(Vec(RKNOB_LEFT +       KNOB_SPREAD - CV_COLUMN_OFFSET, CV_ROW_2), my_module, Hc1in::R2_INPUT));
    addChild(createInputCentered<ColorPort>(Vec(RKNOB_LEFT + 2.f * KNOB_SPREAD - CV_COLUMN_OFFSET, CV_ROW_2), my_module, Hc1in::R3_INPUT));
    addChild(createInputCentered<ColorPort>(Vec(RKNOB_LEFT + 3.f * KNOB_SPREAD - CV_COLUMN_OFFSET, CV_ROW_2), my_module, Hc1in::R4_INPUT));
    addChild(createInputCentered<ColorPort>(Vec(RKNOB_LEFT + 4.f * KNOB_SPREAD - CV_COLUMN_OFFSET, CV_ROW_2), my_module, Hc1in::RMIX_INPUT));
    addChild(createInputCentered<ColorPort>(Vec(RKNOB_LEFT + 5.f * KNOB_SPREAD - CV_COLUMN_OFFSET, CV_ROW_2), my_module, Hc1in::VOLUME_INPUT));

    addChild(createModKnob(Vec(KNOB_LEFT                    , KNOB_ROW_1), my_module, Hc1p::M1_PARAM, Hc1in::M1_INPUT, Hc1p::M1_REL_PARAM));
    addChild(createModKnob(Vec(KNOB_LEFT +       KNOB_SPREAD, KNOB_ROW_1), my_module, Hc1p::M2_PARAM, Hc1in::M2_INPUT, Hc1p::M2_REL_PARAM));
    addChild(createModKnob(Vec(KNOB_LEFT + 2.f * KNOB_SPREAD, KNOB_ROW_1), my_module, Hc1p::M3_PARAM, Hc1in::M3_INPUT, Hc1p::M3_REL_PARAM));
    addChild(createModKnob(Vec(KNOB_LEFT + 3.f * KNOB_SPREAD, KNOB_ROW_1), my_module, Hc1p::M4_PARAM, Hc1in::M4_INPUT, Hc1p::M4_REL_PARAM));
    addChild(createModKnob(Vec(KNOB_LEFT + 4.f * KNOB_SPREAD, KNOB_ROW_1), my_module, Hc1p::M5_PARAM, Hc1in::M5_INPUT, Hc1p::M5_REL_PARAM));
    addChild(createModKnob(Vec(KNOB_LEFT + 5.f * KNOB_SPREAD, KNOB_ROW_1), my_module, Hc1p::M6_PARAM, Hc1in::M6_INPUT, Hc1p::M6_REL_PARAM));
    float y = KNOB_ROW_1 - STATIC_LABEL_OFFSET;
    addChild(createDynamicLabel<DynamicTextLabel>(Vec(KNOB_LEFT,                     y), 100.f, [=](){ return macroName(0); }));
    addChild(createDynamicLabel<DynamicTextLabel>(Vec(KNOB_LEFT +       KNOB_SPREAD, y), 100.f, [=](){ return macroName(1); }));
    addChild(createDynamicLabel<DynamicTextLabel>(Vec(KNOB_LEFT + 2.f * KNOB_SPREAD, y), 100.f, [=](){ return macroName(2); }));
    addChild(createDynamicLabel<DynamicTextLabel>(Vec(KNOB_LEFT + 3.f * KNOB_SPREAD, y), 100.f, [=](){ return macroName(3); }));
    addChild(createDynamicLabel<DynamicTextLabel>(Vec(KNOB_LEFT + 4.f * KNOB_SPREAD, y), 100.f, [=](){ return macroName(4); }));
    addChild(createDynamicLabel<DynamicTextLabel>(Vec(KNOB_LEFT + 5.f * KNOB_SPREAD, y), 100.f, [=](){ return macroName(5); }));

    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<BlueLight>>>(Vec(KNOB_LEFT                     - RB_OFFSET, CV_ROW_1 - RB_VOFFSET), my_module, Hc1p::M1_REL_PARAM, Hc1lt::M1_REL_LIGHT));
    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<BlueLight>>>(Vec(KNOB_LEFT +       KNOB_SPREAD - RB_OFFSET, CV_ROW_1 - RB_VOFFSET), my_module, Hc1p::M2_REL_PARAM, Hc1lt::M2_REL_LIGHT));
    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<BlueLight>>>(Vec(KNOB_LEFT + 2.f * KNOB_SPREAD - RB_OFFSET, CV_ROW_1 - RB_VOFFSET), my_module, Hc1p::M3_REL_PARAM, Hc1lt::M3_REL_LIGHT));
    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<BlueLight>>>(Vec(KNOB_LEFT + 3.f * KNOB_SPREAD - RB_OFFSET, CV_ROW_1 - RB_VOFFSET), my_module, Hc1p::M4_REL_PARAM, Hc1lt::M4_REL_LIGHT));
    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<BlueLight>>>(Vec(KNOB_LEFT + 4.f * KNOB_SPREAD - RB_OFFSET, CV_ROW_1 - RB_VOFFSET), my_module, Hc1p::M5_REL_PARAM, Hc1lt::M5_REL_LIGHT));
    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<BlueLight>>>(Vec(KNOB_LEFT + 5.f * KNOB_SPREAD - RB_OFFSET, CV_ROW_1 - RB_VOFFSET), my_module, Hc1p::M6_REL_PARAM, Hc1lt::M6_REL_LIGHT));

    addChild(createModKnob(Vec(RKNOB_LEFT                    , KNOB_ROW_2), my_module, Hc1p::R1_PARAM, Hc1in::R1_INPUT, Hc1p::R1_REL_PARAM));
    addChild(createModKnob(Vec(RKNOB_LEFT +       KNOB_SPREAD, KNOB_ROW_2), my_module, Hc1p::R2_PARAM, Hc1in::R2_INPUT, Hc1p::R2_REL_PARAM));
    addChild(createModKnob(Vec(RKNOB_LEFT + 2.f * KNOB_SPREAD, KNOB_ROW_2), my_module, Hc1p::R3_PARAM, Hc1in::R3_INPUT, Hc1p::R3_REL_PARAM));
    addChild(createModKnob(Vec(RKNOB_LEFT + 3.f * KNOB_SPREAD, KNOB_ROW_2), my_module, Hc1p::R4_PARAM, Hc1in::R4_INPUT, Hc1p::R4_REL_PARAM));
    addChild(createModKnob(Vec(RKNOB_LEFT + 4.f * KNOB_SPREAD, KNOB_ROW_2), my_module, Hc1p::RMIX_PARAM, Hc1in::RMIX_INPUT, Hc1p::RMIX_REL_PARAM));
    addChild(createModKnob(Vec(RKNOB_LEFT + 5.f * KNOB_SPREAD, KNOB_ROW_2), my_module, Hc1p::VOLUME_PARAM, Hc1in::VOLUME_INPUT, Hc1p::VOLUME_REL_PARAM));

    y = KNOB_ROW_2 - STATIC_LABEL_OFFSET;
    addChild(createDynamicLabel<DynamicTextLabel>(Vec(RKNOB_LEFT,                     y), 80.f, [=](){ return RecirculatorParameterName(my_module ? my_module->recirculatorType() : EM_Recirculator::Reverb, 1); }));
    addChild(createDynamicLabel<DynamicTextLabel>(Vec(RKNOB_LEFT + KNOB_SPREAD,       y), 80.f, [=](){ return RecirculatorParameterName(my_module ? my_module->recirculatorType() : EM_Recirculator::Reverb, 2); }));
    addChild(createDynamicLabel<DynamicTextLabel>(Vec(RKNOB_LEFT + KNOB_SPREAD * 2.f, y), 80.f, [=](){ return RecirculatorParameterName(my_module ? my_module->recirculatorType() : EM_Recirculator::Reverb, 3); }));
    addChild(createDynamicLabel<DynamicTextLabel>(Vec(RKNOB_LEFT + KNOB_SPREAD * 3.f, y), 80.f, [=](){ return RecirculatorParameterName(my_module ? my_module->recirculatorType() : EM_Recirculator::Reverb, 4); }));

    addChild(createStaticTextLabel<StaticTextLabel>(Vec(RKNOB_LEFT + 4.f * KNOB_SPREAD, KNOB_ROW_2 - STATIC_LABEL_OFFSET), 25.f, "Mix"));
    addChild(createStaticTextLabel<StaticTextLabel>(Vec(RKNOB_LEFT + 5.f * KNOB_SPREAD, KNOB_ROW_2 - STATIC_LABEL_OFFSET), 35.f, "Volume"));

    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<BlueLight>>>(Vec(RKNOB_LEFT                     - RB_OFFSET, CV_ROW_2 - RB_VOFFSET), my_module, Hc1p::R1_REL_PARAM, Hc1lt::R1_REL_LIGHT));
    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<BlueLight>>>(Vec(RKNOB_LEFT +       KNOB_SPREAD - RB_OFFSET, CV_ROW_2 - RB_VOFFSET), my_module, Hc1p::R2_REL_PARAM, Hc1lt::R2_REL_LIGHT));
    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<BlueLight>>>(Vec(RKNOB_LEFT + 2.f * KNOB_SPREAD - RB_OFFSET, CV_ROW_2 - RB_VOFFSET), my_module, Hc1p::R3_REL_PARAM, Hc1lt::R3_REL_LIGHT));
    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<BlueLight>>>(Vec(RKNOB_LEFT + 3.f * KNOB_SPREAD - RB_OFFSET, CV_ROW_2 - RB_VOFFSET), my_module, Hc1p::R4_REL_PARAM, Hc1lt::R4_REL_LIGHT));
    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<BlueLight>>>(Vec(RKNOB_LEFT + 4.f * KNOB_SPREAD - RB_OFFSET, CV_ROW_2 - RB_VOFFSET), my_module, Hc1p::RMIX_REL_PARAM, Hc1lt::RMIX_REL_LIGHT));
    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<BlueLight>>>(Vec(RKNOB_LEFT + 5.f * KNOB_SPREAD - RB_OFFSET, CV_ROW_2 - RB_VOFFSET), my_module, Hc1p::VOLUME_REL_PARAM, Hc1lt::VOLUME_REL_LIGHT));

    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<RedLight>>>(Vec(RKNOB_LEFT + 5.f * KNOB_SPREAD + RB_OFFSET,        CV_ROW_2 - RB_VOFFSET), my_module, Hc1p::MUTE_PARAM, Hc1lt::MUTE_LIGHT));
    addChild(createInputCentered<ColorPort>(                                   Vec(RKNOB_LEFT + 5.f * KNOB_SPREAD + RB_OFFSET + 14.f, CV_ROW_2 - RB_VOFFSET), my_module, Hc1in::MUTE_INPUT));
    addChild(createStaticTextLabel<StaticTextLabel>(Vec(RKNOB_LEFT + KNOB_SPREAD * 5.f + 17.f, KNOB_ROW_2), 25.f, "Mute", TextAlignment::Left));

    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<BlueLight>>>(Vec(RECIRC_EXTEND_CENTER, RECIRC_BOX_TOP), my_module, Hc1p::RECIRC_EXTEND_PARAM, Hc1lt::RECIRC_EXTEND_LIGHT));
    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<BlueLight>>>(Vec(RECIRC_ENABLE_CENTER, RECIRC_BOX_TOP), my_module, Hc1p::RECIRC_ENABLE_PARAM, Hc1lt::RECIRC_ENABLE_LIGHT));

    addChild(dsp_widget = createWidget<DspWidget>(Vec(RIGHT_COLUMN_BUTTONS - DSP_WIDTH * .5f, 86.f)));

#ifdef TRANSPOSE_BUTTONS
    createTranspose();
#endif
    createMidiSelection();
    createTestNote();
    createStatusDots();
    createDeviceDisplay();
}

void Hc1ModuleWidget::pageUp()
{
    if (!my_module) return;
    assert(page >= 0);
    if (0 == page) return;
    my_module->setTabPage(tab, --page);
    updatePresetWidgets();
    page_up->enable(page > 0);
    page_down->enable(static_cast<unsigned>(page*24) < my_module->getPresets(tab).size()-24);
}

void Hc1ModuleWidget::pageDown()
{
    if (!my_module) return;
    auto mp = my_module->getPresets(tab);
    auto max = mp.size();
    if (static_cast<unsigned>((1 + page) * 24) > max) return;
    my_module->setTabPage(tab, ++page);
    updatePresetWidgets();
    page_up->enable(true);
    page_down->enable(static_cast<unsigned>(page*24) < max-24);
}

//
// IDsp
//
void Hc1ModuleWidget::set_dsp_ready(bool ready)
{
    if (dsp_widget) dsp_widget->set_dsp_ready(ready);
}
void Hc1ModuleWidget::set_dsp_value(int index, uint8_t value)
{
    if (dsp_widget) dsp_widget->set_dsp_value(index, value);
}

//
// IPresetHolder
//
void Hc1ModuleWidget::setPreset(std::shared_ptr<Preset> preset)
{
    if (! my_module) return;
    my_module->setPreset(preset);
}
bool Hc1ModuleWidget::isCurrentPreset(std::shared_ptr<Preset> preset)
{
    return my_module ? my_module->isCurrentPreset(preset) : false;
}
void Hc1ModuleWidget::addFavorite(std::shared_ptr<Preset> preset)
{
    if (! my_module) return;
    my_module->addFavorite(preset);
    updatePresetWidgets();
}
void Hc1ModuleWidget::unFavorite(std::shared_ptr<Preset> preset)
{
    if (! my_module) return;
    my_module->unFavorite(preset);
    updatePresetWidgets();
}
void Hc1ModuleWidget::moveFavorite(std::shared_ptr<Preset> preset, FavoriteMove motion)
{
    if (! my_module) return;
    my_module->moveFavorite(preset, motion);
    updatePresetWidgets();
}

//
// IHandleHcEvents
//
void Hc1ModuleWidget::onPresetChanged(const PresetChangedEvent& e)
{
    firmware_label->text(format_string("v%03.2f", (my_module ? my_module->em.firmware_version : 0)/100.f));
    hardware_label->text(my_module ? HardwareName(my_module->em.hardware) : "");
    favorite->setPreset(my_module->current_preset);
    showCurrentPreset(true);
}

void Hc1ModuleWidget::onDeviceChanged(const DeviceChangedEvent& e)
{
    bool have_device = e.device && (-1 != e.device->input_device_id);
    em_picker->setConnection(e.device);
    em_picker->describe(have_device ? e.device->info.friendly(TextFormatLength::Long) : "Choose an Eagan Matrix device");
    device_label->text(have_device ? e.device->info.friendly(TextFormatLength::Short) : "<Eagan Matrix device>");
    hardware_label->text(my_module ? HardwareName(my_module->em.hardware) : "");
    firmware_label->text(format_string("v%03.2f", (my_module ? my_module->em.firmware_version : 0)/100.f));
}

void Hc1ModuleWidget::onDisconnect(const DisconnectEvent& e)
{
    firmware_label->text("");
    hardware_label->text("");
    device_label->text("");
}

void Hc1ModuleWidget::onFavoritesFileChanged(const FavoritesFileChangedEvent& e)
{
    if (tab == PresetTab::Favorite) {
        updatePresetWidgets();
    }
}

void Hc1ModuleWidget::toCategory(uint16_t code)
{
    if (!my_module) return;
    my_module->setPresetOrder(PresetOrder::Category);
    setTab(PresetTab::System);
    if (my_module->current_preset && code == my_module->current_preset->primaryCategory()) {
        return;
    }
    auto mp = my_module->getPresets(tab);
    auto index = std::distance(mp.cbegin(), std::find_if(mp.cbegin(), mp.cend(), [=](std::shared_ptr<Preset> p){ return code == p->primaryCategory(); }));
    auto size = static_cast<ptrdiff_t>(mp.size());
    if (index == size) {
        return; // not found
    }
    my_module->setPreset(mp[index]);
}

void Hc1ModuleWidget::toRelativePreset(int delta)
{
    if (!my_module) return;
    auto mp = my_module->getPresets(tab);
    auto index = std::distance(mp.cbegin(), std::find_if(mp.cbegin(), mp.cend(), [=](std::shared_ptr<Preset> p){ return isCurrentPreset(p); }));
    auto size = static_cast<ptrdiff_t>(mp.size());
    if (index == size) {
        return; // not found
    }
    index = (index + delta);
    if (index < 0) {
        index = size - 1;
    } else if (index >= size) {
        index = 0; // wrap to start (clipped)
    }
    my_module->setPreset(mp[index]);
}

bool Hc1ModuleWidget::showCurrentPreset(bool changeTab)
{
    if (!my_module || !my_module->current_preset)  { return false; }
    // check if we're already on the page with the current preset
    {
        auto it = std::find_if(presets.cbegin(), presets.cend(), [=](PresetWidget* pw) { 
            return pw->preset ? isCurrentPreset(pw->preset) : false;
        });
        if (it != presets.cend()) { return true; }
    }
    std::vector<PresetTab> search_tab;
    search_tab.push_back(tab);
    if (tab != PresetTab::System  ) { search_tab.push_back(PresetTab::System); }
    if (tab != PresetTab::Favorite) { search_tab.push_back(PresetTab::Favorite); }
    if (tab != PresetTab::User    ) { search_tab.push_back(PresetTab::User); }
    for (auto n = 0; n < 3; ++n) {
        if (!changeTab && search_tab[n] != tab) return false;
        auto mp = my_module->getPresets(search_tab[n]);
        auto it = std::find_if(mp.cbegin(), mp.cend(), [=](std::shared_ptr<Preset> p){ return isCurrentPreset(p); });
        if (it != mp.cend()) {
            if (search_tab[n] != tab) {
                setTab(search_tab[n]);
            }
            int new_page = (it - mp.cbegin()) / 24;
            if (new_page != page) {
                page = new_page;
                updatePresetWidgets();
            }
            page_down->enable(static_cast<unsigned>(page*24) < mp.size()-24);
            my_module->setTabPage(tab, page);
            return true;
        }
    }
    return false;
}

void Hc1ModuleWidget::clearPresetWidgets()
{
    for (auto p: presets) {
        p->setPreset(nullptr);
    }
    have_preset_widgets = false;
}

void Hc1ModuleWidget::updatePresetWidgets()
{
    if (!my_module) return;

    auto p = presets.begin();
    auto mp = my_module->getPresets(tab);
    // if (my_module->preset_filter.isFiltered()) {
    // } else {
        auto start = mp.cbegin() + 24 * page;
        // sanity check page
        if (start >= mp.cend()) {
            page = 0;
            start = mp.cbegin();
        }
        for (auto it = start; 
            (p != presets.end()) && (it != mp.cend());
            ++p, ++it)
        {
            (*p)->setPreset(*it);
        }
        while (p < presets.end()) {
            (*p++)->setPreset(nullptr);
        }
        have_preset_widgets = true;
        if (mp.size() <= 24){
            page_up->enable(false);
            page_down->enable(false);
        } else {
            page_up->enable(page > 0);
            page_down->enable(start <  mp.cend()-(24 * page));
        }
//    }
}

void Hc1ModuleWidget::populatePresetWidgets()
{
    if (!my_module) return;
    clearPresetWidgets();
    updatePresetWidgets();
}

void  Hc1ModuleWidget::setTab(PresetTab new_tab, bool force)
{
    if (!force && (new_tab == this->tab)) return;
    if (my_module) my_module->tab = new_tab;
    tab = new_tab;
    tab_bar->selectTab(tab);
    page = my_module ? my_module->getTabPage(tab) : 0;
    updatePresetWidgets();
    showCurrentPreset(false);
}

void Hc1ModuleWidget::onHoverScroll(const HoverScrollEvent& e)
{
    auto top = tab_bar->box.getTop();
    Rect preset_area = {PRESET_LEFT, top, PRESET_WIDTH, PRESET_BOTTOM - tab_bar->box.getTop() }; 
    Rect preset_nav = {PRESET_RIGHT, top, box.size.x - PRESET_RIGHT, page_down->box.getBottom() - top + 15.f};
    if (preset_area.contains(e.pos) || preset_nav.contains(e.pos)) {
        auto dx = e.scrollDelta;
        if (dx.y < 0.f) {
            pageDown();
        } else if (dx.y > 0.f) {
            pageUp();
        }
        e.consume(this);
    } else {
        ModuleWidget::onHoverScroll(e);
    }
}

void Hc1ModuleWidget::step()
{
    ModuleWidget::step();
    if (my_module)
    {
        tab_bar->setExtra(PresetTab::Favorite, !my_module->favoritesFile.empty());

        auto co = (my_module->isEaganMatrix()) ? my_module->ledColor : red_light;
        if (!IS_SAME_COLOR(co, status_light->baseColors[0])) {
            status_light->baseColors[0] = co;
            my_module->getLight(Hc1Module::HEART_LIGHT).setBrightness(1.f);
            DirtyEvent e;
            status_light->onDirty(e);
        }

        Hc1Module::RestoreData * restore = nullptr;
        if (my_module->restore_ui_data && my_module->hasSystemPresets() && my_module->hasUserPresets()) {
            restore = my_module->restore_ui_data;
            my_module->restore_ui_data = nullptr;

            tab = restore->tab;
            page = restore->page[static_cast<size_t>(tab)];
            my_module->tab = tab;
            my_module->setTabPage(tab, page);
            delete restore;
            updatePresetWidgets();
            return;
        }

        auto selected = tab_bar->getSelectedTab();
        if (selected != tab) {
            setTab(selected);
        }
        switch (tab) {
            case PresetTab::System:
                if (!have_preset_widgets && my_module->hasSystemPresets()) {
                    populatePresetWidgets();
                } else if (have_preset_widgets && !my_module->hasSystemPresets()) {
                    clearPresetWidgets();
                }
                break;
            case PresetTab::User:
                if (!have_preset_widgets && my_module->hasUserPresets()) {
                    populatePresetWidgets();
                } else if (have_preset_widgets && !my_module->hasUserPresets()) {
                    clearPresetWidgets();
                }
                break;
            case PresetTab::Favorite:
                if (!have_preset_widgets && my_module->hasFavorites()) {
                    populatePresetWidgets();
                } else if (have_preset_widgets && !my_module->hasFavorites()) {
                    clearPresetWidgets();
                }
                break;
        }
    }
}


} //pachde