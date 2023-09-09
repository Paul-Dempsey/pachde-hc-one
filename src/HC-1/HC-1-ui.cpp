#include "HC-1.hpp"
#include "cc_param.hpp"
#include "../components.hpp"
#include "../misc.hpp"
#include "../open_file.hpp"
#include "../port.hpp"
#include "../small_push.hpp"

namespace pachde {

//#define SHOW_PRESET0

std::string PedalAssignment(uint8_t ped) {
    switch (ped) {
    case 12: return "i";
    case 13: return "ii";
    case 14: return "iii";
    case 15: return "iv";
    case 16: return "v";
    case 17: return "vi";
    case 20: return "R1";
    case 21: return "R2";
    case 22: return "R3";
    case 23: return "R4";
    case 24: return "RMix";
    case 64: return "Sus";
    case 66: return "Sos";
    }
    return format_string("%d", ped);
}

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

struct PDSmallButton : app::SvgSwitch {
    PDSmallButton() {
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/TinyPush_up.svg")));
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/TinyPush_down.svg")));
    }
};

template <typename TLight>
struct PDLightButton : app::SvgSwitch {
	app::ModuleLightWidget* light;

	PDLightButton() {
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/TinyPush_up.svg")));
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/TinyPush_down.svg")));
		light = new TLight;
		// Move center of light to center of box
		light->box.pos = box.size.div(2).minus(light->box.size.div(2));
		addChild(light);
	}

	app::ModuleLightWidget* getLight() {
		return light;
	}
};
template <typename TLight>
using SmallLightButton = PDLightButton<TLight>;

template <typename TLight>
struct PDLightLatch : PDLightButton<TLight> {
	PDLightLatch() {
		this->momentary = false;
		this->latch = true;
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

constexpr const float PRESET_TOP = 38.f;
constexpr const float PRESET_LEFT = 7.5f;
constexpr const float PRESET_WIDTH = 320.f;
constexpr const float PRESET_RIGHT = PRESET_LEFT + PRESET_WIDTH;
constexpr const float PRESET_BOTTOM = PRESET_TOP + 8.f * 27.f;

constexpr const float RIGHT_COLUMN_BUTTONS = PRESET_RIGHT + (360.f - PRESET_RIGHT)*.5f;
constexpr const float KNOB_LEFT   = 45.f;
constexpr const float KNOB_SPREAD = 54.25f;
constexpr const float KNOB_ROW_1  = 288.f;
constexpr const float KNOB_ROW_2  = 346.f;
constexpr const float RKNOB_LEFT  = KNOB_LEFT; //- KNOB_SPREAD *.5f;

constexpr const float LABEL_OFFSET = 20.f;

constexpr const float CV_COLUMN_OFFSET = 24.f;
constexpr const float RB_OFFSET = 20.f;
constexpr const float RB_VOFFSET = 15.f;
constexpr const float CV_ROW_1 = KNOB_ROW_1 + 6.f;
constexpr const float CV_ROW_2 = KNOB_ROW_2 + 6.f;

constexpr const float RECIRC_BOX_TOP = KNOB_ROW_2 - LABEL_OFFSET - 14.f;
constexpr const float RECIRC_BOX_LEFT = RKNOB_LEFT - 35.5f;
constexpr const float RECIRC_BOX_WIDTH = KNOB_SPREAD * 5.f;
constexpr const float RECIRC_BOX_RIGHT = RECIRC_BOX_LEFT + RECIRC_BOX_WIDTH;
constexpr const float RECIRC_BOX_HEIGHT = 52.5f;
constexpr const float RECIRC_BOX_BOTTOM = RECIRC_BOX_TOP + RECIRC_BOX_HEIGHT;
constexpr const float RECIRC_TITLE_WIDTH = 70.f;
constexpr const float RECIRC_BOX_CENTER = RECIRC_BOX_LEFT + RECIRC_BOX_WIDTH *.5f;
constexpr const float RECIRC_LIGHT_CENTER = RECIRC_BOX_CENTER + (RECIRC_TITLE_WIDTH * .5f) + 5.f;

Hc1ModuleWidget::Hc1ModuleWidget(Hc1Module *module)
{
    my_module = module;
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/HC-1.svg")));

    if (my_module) {
        tab = my_module->tab;
        page = my_module->getTabPage(tab);
    }

    status_light = createLightCentered<MediumLight<BlueLight>>(Vec(12.f, 12.f), my_module, Hc1Module::HEART_LIGHT);
    addChild(status_light);

    favorite = createWidget<FavoriteWidget>(Vec(box.size.x - (12.f + 6.f), 6.f));
    favorite->setPresetHolder(this);
    addChild(favorite);

    addChild(createInputCentered<ColorPort>(Vec(KNOB_LEFT                     - CV_COLUMN_OFFSET, CV_ROW_1), module, Hc1Module::M1_INPUT));
    addChild(createInputCentered<ColorPort>(Vec(KNOB_LEFT +       KNOB_SPREAD - CV_COLUMN_OFFSET, CV_ROW_1), module, Hc1Module::M2_INPUT));
    addChild(createInputCentered<ColorPort>(Vec(KNOB_LEFT + 2.f * KNOB_SPREAD - CV_COLUMN_OFFSET, CV_ROW_1), module, Hc1Module::M3_INPUT));
    addChild(createInputCentered<ColorPort>(Vec(KNOB_LEFT + 3.f * KNOB_SPREAD - CV_COLUMN_OFFSET, CV_ROW_1), module, Hc1Module::M4_INPUT));
    addChild(createInputCentered<ColorPort>(Vec(KNOB_LEFT + 4.f * KNOB_SPREAD - CV_COLUMN_OFFSET, CV_ROW_1), module, Hc1Module::M5_INPUT));
    addChild(createInputCentered<ColorPort>(Vec(KNOB_LEFT + 5.f * KNOB_SPREAD - CV_COLUMN_OFFSET, CV_ROW_1), module, Hc1Module::M6_INPUT));

    addChild(createInputCentered<ColorPort>(Vec(RKNOB_LEFT                     - CV_COLUMN_OFFSET, CV_ROW_2), module, Hc1Module::R1_INPUT));
    addChild(createInputCentered<ColorPort>(Vec(RKNOB_LEFT +       KNOB_SPREAD - CV_COLUMN_OFFSET, CV_ROW_2), module, Hc1Module::R2_INPUT));
    addChild(createInputCentered<ColorPort>(Vec(RKNOB_LEFT + 2.f * KNOB_SPREAD - CV_COLUMN_OFFSET, CV_ROW_2), module, Hc1Module::R3_INPUT));
    addChild(createInputCentered<ColorPort>(Vec(RKNOB_LEFT + 3.f * KNOB_SPREAD - CV_COLUMN_OFFSET, CV_ROW_2), module, Hc1Module::R4_INPUT));
    addChild(createInputCentered<ColorPort>(Vec(RKNOB_LEFT + 4.f * KNOB_SPREAD - CV_COLUMN_OFFSET, CV_ROW_2), module, Hc1Module::RMIX_INPUT));
    addChild(createInputCentered<ColorPort>(Vec(RKNOB_LEFT + 5.f * KNOB_SPREAD - CV_COLUMN_OFFSET, CV_ROW_2), module, Hc1Module::VOLUME_INPUT));

    addChild(createParamCentered<MidiKnob>(Vec(KNOB_LEFT                    , KNOB_ROW_1), module, Hc1Module::M1_PARAM));
    addChild(createParamCentered<MidiKnob>(Vec(KNOB_LEFT +       KNOB_SPREAD, KNOB_ROW_1), module, Hc1Module::M2_PARAM));
    addChild(createParamCentered<MidiKnob>(Vec(KNOB_LEFT + 2.f * KNOB_SPREAD, KNOB_ROW_1), module, Hc1Module::M3_PARAM));
    addChild(createParamCentered<MidiKnob>(Vec(KNOB_LEFT + 3.f * KNOB_SPREAD, KNOB_ROW_1), module, Hc1Module::M4_PARAM));
    addChild(createParamCentered<MidiKnob>(Vec(KNOB_LEFT + 4.f * KNOB_SPREAD, KNOB_ROW_1), module, Hc1Module::M5_PARAM));
    addChild(createParamCentered<MidiKnob>(Vec(KNOB_LEFT + 5.f * KNOB_SPREAD, KNOB_ROW_1), module, Hc1Module::M6_PARAM));

    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<BlueLight>>>(Vec(KNOB_LEFT                     - RB_OFFSET, CV_ROW_1 - RB_VOFFSET), module, Hc1Module::M1_REL_PARAM, Hc1Module::M1_REL_LIGHT));
    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<BlueLight>>>(Vec(KNOB_LEFT +       KNOB_SPREAD - RB_OFFSET, CV_ROW_1 - RB_VOFFSET), module, Hc1Module::M2_REL_PARAM, Hc1Module::M2_REL_LIGHT));
    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<BlueLight>>>(Vec(KNOB_LEFT + 2.f * KNOB_SPREAD - RB_OFFSET, CV_ROW_1 - RB_VOFFSET), module, Hc1Module::M3_REL_PARAM, Hc1Module::M3_REL_LIGHT));
    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<BlueLight>>>(Vec(KNOB_LEFT + 3.f * KNOB_SPREAD - RB_OFFSET, CV_ROW_1 - RB_VOFFSET), module, Hc1Module::M4_REL_PARAM, Hc1Module::M4_REL_LIGHT));
    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<BlueLight>>>(Vec(KNOB_LEFT + 4.f * KNOB_SPREAD - RB_OFFSET, CV_ROW_1 - RB_VOFFSET), module, Hc1Module::M5_REL_PARAM, Hc1Module::M5_REL_LIGHT));
    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<BlueLight>>>(Vec(KNOB_LEFT + 5.f * KNOB_SPREAD - RB_OFFSET, CV_ROW_1 - RB_VOFFSET), module, Hc1Module::M6_REL_PARAM, Hc1Module::M6_REL_LIGHT));

    addChild(createParamCentered<MidiKnob>(Vec(RKNOB_LEFT                    , KNOB_ROW_2), module, Hc1Module::R1_PARAM));
    addChild(createParamCentered<MidiKnob>(Vec(RKNOB_LEFT +       KNOB_SPREAD, KNOB_ROW_2), module, Hc1Module::R2_PARAM));
    addChild(createParamCentered<MidiKnob>(Vec(RKNOB_LEFT + 2.f * KNOB_SPREAD, KNOB_ROW_2), module, Hc1Module::R3_PARAM));
    addChild(createParamCentered<MidiKnob>(Vec(RKNOB_LEFT + 3.f * KNOB_SPREAD, KNOB_ROW_2), module, Hc1Module::R4_PARAM));
    addChild(createParamCentered<MidiKnob>(Vec(RKNOB_LEFT + 4.f * KNOB_SPREAD, KNOB_ROW_2), module, Hc1Module::RMIX_PARAM));
    addChild(createParamCentered<MidiKnob>(Vec(RKNOB_LEFT + 5.f * KNOB_SPREAD, KNOB_ROW_2), module, Hc1Module::VOLUME_PARAM));

    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<BlueLight>>>(Vec(RKNOB_LEFT                     - RB_OFFSET, CV_ROW_2 - RB_VOFFSET), module, Hc1Module::R1_REL_PARAM, Hc1Module::R1_REL_LIGHT));
    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<BlueLight>>>(Vec(RKNOB_LEFT +       KNOB_SPREAD - RB_OFFSET, CV_ROW_2 - RB_VOFFSET), module, Hc1Module::R2_REL_PARAM, Hc1Module::R2_REL_LIGHT));
    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<BlueLight>>>(Vec(RKNOB_LEFT + 2.f * KNOB_SPREAD - RB_OFFSET, CV_ROW_2 - RB_VOFFSET), module, Hc1Module::R3_REL_PARAM, Hc1Module::R3_REL_LIGHT));
    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<BlueLight>>>(Vec(RKNOB_LEFT + 3.f * KNOB_SPREAD - RB_OFFSET, CV_ROW_2 - RB_VOFFSET), module, Hc1Module::R4_REL_PARAM, Hc1Module::R4_REL_LIGHT));
    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<BlueLight>>>(Vec(RKNOB_LEFT + 4.f * KNOB_SPREAD - RB_OFFSET, CV_ROW_2 - RB_VOFFSET), module, Hc1Module::RMIX_REL_PARAM, Hc1Module::RMIX_REL_LIGHT));
    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<BlueLight>>>(Vec(RKNOB_LEFT + 5.f * KNOB_SPREAD - RB_OFFSET, CV_ROW_2 - RB_VOFFSET), module, Hc1Module::VOLUME_REL_PARAM, Hc1Module::VOLUME_REL_LIGHT));

    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<RedLight>>>(Vec(RKNOB_LEFT + 5.f * KNOB_SPREAD + RB_OFFSET, CV_ROW_2 - RB_VOFFSET), module, Hc1Module::MUTE_PARAM, Hc1Module::MUTE_LIGHT));
    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<BlueLight>>>(Vec(RECIRC_LIGHT_CENTER, RECIRC_BOX_TOP), module, Hc1Module::RECIRC_EXTEND_PARAM, Hc1Module::RECIRC_EXTEND_LIGHT));

    presets.reserve(24);
    int x = PRESET_LEFT;
    int y = PRESET_TOP;
    for (int n = 0; n < 24; ++n) {
        auto p = createWidget<PresetWidget>(Vec(x, y));
        p->setPresetHolder(module);
        addChild(p);
        presets.push_back(p);
        x += p->box.size.x;
        if (0 == ((n + 1) % 3)) {
            x = PRESET_LEFT;
            y += p->box.size.y;
        }
    }

    page_up = createWidgetCentered<SquareButton>(Vec(RIGHT_COLUMN_BUTTONS, PRESET_TOP + 12.f));
    page_up->setSymbol(SquareButtonSymbol::Up);
    page_up->onClick([this](){ pageUp(); });
    addChild(page_up);

    page_down = createWidgetCentered<SquareButton>(Vec(RIGHT_COLUMN_BUTTONS, PRESET_TOP + 29.f));
    page_up->setSymbol(SquareButtonSymbol::Down);
    page_down->onClick([this](){ pageDown(); });
    addChild(page_down);

    { // Transpose buttons
        auto y = PRESET_TOP + 65.f;
        auto x = RIGHT_COLUMN_BUTTONS;
        auto pb = createWidgetCentered<SmallPush>(Vec(x, y));
        if (module) {
#ifdef ARCH_MAC
            pb->describe("Octave up\nCmd+Click for one semitone");
#else
            pb->describe("Octave up\nCtrl+Click for one semitone");
#endif
            pb->onClick([module](bool ctrl, bool shift) {
                module->middle_c += ctrl ? 1 : 12;
                module->sendControlChange(EM_SettingsChannel, EMCC_MiddleC, module->middle_c);
            });
        }
        addChild(pb);
        y += 12.5f;
 
        pb = createWidgetCentered<SmallPush>(Vec(x, y));
        if (module) {
            pb->describe("Transpose none");
            pb->onClick([module](bool ctrl, bool shift) {
                module->middle_c = 60;
                module->sendControlChange(EM_SettingsChannel, EMCC_MiddleC, module->middle_c);
            });
        }
        addChild(pb);
        y += 12.5f;

        pb = createWidgetCentered<SmallPush>(Vec(x, y));
        if (module) {
#ifdef ARCH_MAC
            pb->describe("Octave down\nCmd+Click for one semitone");
#else
            pb->describe("Octave down\nCtrl+Click for one semitone");
#endif
            pb->onClick([module](bool ctrl, bool shift) {
                module->middle_c -= ctrl ? 1 : 12;
                module->sendControlChange(EM_SettingsChannel, EMCC_MiddleC, module->middle_c);
            });
        }
        addChild(pb);
    }

    // Filter
    // {
    //     auto light = createLightCentered<SmallLight<BlueLight>>(Vec(box.size.x -15.f, PRESET_BOTTOM - 24.f), my_module, Hc1Module::Lights::FILTER_LIGHT);
    //     light->baseColors[0] = PORT_VIOLET;
    //     if (my_module) {
    //         my_module->getLight(Hc1Module::Lights::FILTER_LIGHT).setBrightness(my_module->preset_filter.isFiltered() * 1.f);
    //     }
    //     addChild(light);
        
    //     auto filter = createWidget<SquareButton>(Vec(box.size.x - 23.f, PRESET_BOTTOM - 18.f));
    //     filter->setSymbol(SquareButtonSymbol::Funnel);
    //     filter->onClick([this]() {
    //         if (!my_module) return;
    //         my_module->filter_presets = !my_module->filter_presets;
    //     });
    //     addChild(filter);
    // }

    tab_bar = createWidget<TabBarWidget>(Vec(PRESET_LEFT, PRESET_TOP - 13.f));
    tab_bar->setSize(Vec(PRESET_WIDTH, 13.f));
    tab_bar->addTab("User", PresetTab::User);
    tab_bar->addTab("Favorite", PresetTab::Favorite);
    tab_bar->addTab("System", PresetTab::System);
    tab_bar->layout();
    tab_bar->selectTab(tab);
    addChild(tab_bar);

    auto pm = createWidget<PickMidi>(Vec(7.5f, box.size.y - 16.f));
    pm->describe("Midi input");
    if (my_module) {
        pm->setMidiPort(my_module);
    }
    addChild(pm);

    pm = createWidget<PickMidi>(Vec(20.f, box.size.y - 16.f));
    pm->describe("Midi output");
    if (my_module) {
        pm->setMidiPort(&my_module->midi_output);
    }
    addChild(pm);

    auto pb = createWidgetCentered<SmallPush>(Vec(45.f, box.size.y -8.5f));
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

bool Hc1ModuleWidget::showCurrentPreset()
{
    if (!my_module->current_preset) return false;

    // check if we're already on the page with the current preset
    {
        auto it = std::find_if(presets.cbegin(), presets.cend(), [=](PresetWidget* pw) { 
            return pw->preset ? isCurrentPreset(pw->preset) : false;
        });
        if (it != presets.cend()) return true;
    }

    auto mp = my_module->getPresets(tab);
    auto it = std::find_if(mp.cbegin(), mp.cend(), [=](std::shared_ptr<Preset> p){ return isCurrentPreset(p); });
    if (it == mp.cend()) return false;  // current preset is on another tab

    page = (it - mp.cbegin()) / 24;
    updatePresetWidgets();
    page_up->enable(page > 0);
    page_down->enable(static_cast<unsigned>(page*24) < mp.size()-24);
    return true;
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
    showCurrentPreset();
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
        if (my_module->current_preset) {
            favorite->setPreset(my_module->current_preset);
        }

        auto co = my_module->isEaganMatrix() ? my_module->ledColor : red_light;
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
            page = restore->page[tab];
            my_module->tab = tab;
            my_module->setTabPage(tab, page);
            delete restore;
            updatePresetWidgets();
            return;
        }

        auto selected = static_cast<PresetTab>(tab_bar->getSelectedTab());
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
                if (!have_preset_widgets && (my_module->hasSystemPresets() && my_module->hasUserPresets())) {
                    populatePresetWidgets();
                } else if (have_preset_widgets && !(my_module->hasSystemPresets() || my_module->hasUserPresets())) {
                    clearPresetWidgets();
                }
                break;
        }
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

    if (my_module) {
        const float y = PRESET_BOTTOM + 1.75f;
        auto cx = PRESET_LEFT + fmodf(my_module->midi_send_count / 20.f, 320.f);
        CircularHalo(vg, cx, y, 2.f, 8.5f, purple_light);
        Circle(vg, cx, y, 1.25f, purple_light);

        cx = PRESET_LEFT + fmodf(my_module->midi_receive_count / 20.f, 320.f);
        CircularHalo(vg, cx, y, 2.f, 8.5f, green_light);
        Circle(vg, cx, y, 1.25f, green_light);
    }

    // DSP status
    //if (!my_module || my_module->heart_beating)
    {
        const float h = 10.f;
        const float w = 2.5f;
        const float y = KNOB_ROW_1 - 5.f;
        float x = box.size.x - 10.f - 3.f * w - 2;
        FillRect(vg, x - 1.25f, y - 1.25f, w * 3.f + 5.f, h + 5.f, RampGray(G_30));
        const uint8_t tdsp[] = {65, 30, 75};
        const uint8_t* pdsp = (my_module && my_module->ready()) ? &my_module->dsp[0] : &tdsp[0];

        if (pdsp == &tdsp[0]) {
            BoxRect(vg, x - 1.5f, y - 1.5f, w * 3.f + 5.f, h + 4.f, green_light, .5f);
        }

        for (auto n = 0; n < 3; n++) {
            auto pct = pdsp[n];
            auto co = pct < 85 ? green_light : red_light;
            auto bh = h * (pct / 100.f);
            FillRect(vg, x, y + h - bh, w, bh, co);
            x += w + 1;
        }
    }
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
    auto text =  format_string("p%c %s", ped_char, PedalAssignment(ped).c_str());
    nvgTextAlign(vg, NVGalign::NVG_ALIGN_LEFT);
    nvgText(vg, PRESET_RIGHT + 1.f, y, text.c_str(), nullptr);
    Line(vg, x, y+1, x, y+1 - ped_value/16.f, GetStockColor(StockColor::Sea_green_Dark), 1.5f);
}

void Hc1ModuleWidget::draw(const DrawArgs& args)
{
    ModuleWidget::draw(args);
    auto vg = args.vg;

    bool stock = !my_module || !my_module->ready();
    auto rt = my_module ? my_module->recirculatorType() : EM_Recirculator::Reverb;

    // expander connector
    if (my_module && my_module->expanders.any()) {
        bool right = my_module->expanders.right();
        float cy = box.size.y * .5f;
        if (right) {
            Line(vg, box.size.x - 5.5f, cy, box.size.x , cy, COLOR_BRAND, 1.75f);
            Circle(vg, box.size.x - 5.5f, cy, 2.5f, COLOR_BRAND);
        } else {
            Line(vg, 0.f, cy, 5.5f, cy, COLOR_BRAND, 1.75f);
            Circle(vg, 5.5f, cy, 2.5f, COLOR_BRAND);
        }
    }

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

        SetTextStyle(vg, font, RampGray(G_90), 12.f);

        { // page
            auto pg = format_string("%d of %d", 1 + page, my_module ? my_module->getPresets(tab).size()/24 : 1);
            CenterText(vg, RIGHT_COLUMN_BUTTONS, PRESET_TOP, pg.c_str(), nullptr);
        }

        { // middle C
            auto c_note = format_string("%d",my_module ? my_module->middle_c : 60);
            CenterText(vg, RIGHT_COLUMN_BUTTONS, PRESET_TOP + 55.f, c_note.c_str(), nullptr);
        }
        
        { // device
            std::string device_name;
            if (my_module) {
                device_name = my_module->deviceName();
            } else {
                device_name = "<Eagan Matrix Device>";

            }
            if (device_name.empty()) {
                device_name = "(no Eagan Matrix available)";
            }
            nvgTextAlign(vg, NVGalign::NVG_ALIGN_LEFT);
            nvgText(vg, box.size.x*.5f + 25.f, box.size.y - 4.f, device_name.c_str(), nullptr);
        }

        // firmware version
        if (my_module && my_module->is_eagan_matrix && (my_module->firmware_version > 0)) {
            RightAlignText(vg, box.size.x - 7.5, box.size.y - 4.f, format_string("v %.2f", my_module->firmware_version/100.f).c_str(), nullptr);
        } else {
            RightAlignText(vg, box.size.x - 7.5, box.size.y - 4.f, "v 00.00", nullptr);
        }

        // recirculator
        {
            Line(vg, RECIRC_BOX_LEFT, RECIRC_BOX_TOP, RECIRC_BOX_CENTER - (RECIRC_TITLE_WIDTH * .5f), RECIRC_BOX_TOP, RampGray(G_35), .5f);
            Line(vg, RECIRC_LIGHT_CENTER + 15.f, RECIRC_BOX_TOP, RECIRC_BOX_RIGHT, RECIRC_BOX_TOP, RampGray(G_35), .5f);
            Line(vg, RECIRC_BOX_LEFT,  RECIRC_BOX_TOP,    RECIRC_BOX_LEFT,  RECIRC_BOX_BOTTOM, RampGray(G_35), .5f);
            Line(vg, RECIRC_BOX_RIGHT, RECIRC_BOX_TOP,    RECIRC_BOX_RIGHT, RECIRC_BOX_BOTTOM, RampGray(G_35), .5f);
            Line(vg, RECIRC_BOX_LEFT,  RECIRC_BOX_BOTTOM, RECIRC_BOX_RIGHT, RECIRC_BOX_BOTTOM, RampGray(G_35), .5f);
            SetTextStyle(vg, font, RampGray(G_90), 12.f);
            CenterText(vg, RECIRC_BOX_CENTER, RECIRC_BOX_TOP + 2.f, RecirculatorName(rt), nullptr);
        }

        // pedals
        SetTextStyle(vg, font, RampGray(G_65), 12.f);
        if (stock) {
            drawPedalAssignment(vg, box.size.x - 3.f, PRESET_BOTTOM - 16.f, '1', 64, 0);
            drawPedalAssignment(vg, box.size.x - 3.f, PRESET_BOTTOM - 2.5f, '2', 66, 0);
        } else {
            auto ped1 = my_module->ch15_cc_value[52];
            auto ped2 = my_module->ch15_cc_value[53];
            if (ped1 == ped2) {
                drawPedalKnobAssignment(vg, ped1, "1,2");
            } else {
                drawPedalKnobAssignment(vg, ped1, "1");
                drawPedalKnobAssignment(vg, ped2, "2");
            }
            drawPedalAssignment(vg, box.size.x - 3.f, PRESET_BOTTOM - 16.f, '1', ped1, my_module->ch0_cc_value[ped1]);
            drawPedalAssignment(vg, box.size.x - 3.f, PRESET_BOTTOM - 2.5f, '2', ped2, my_module->ch0_cc_value[ped2]);
        }
    }

    // labels
    font = GetPluginFontSemiBold();
    if (FontOk(font)) {
        SetTextStyle(vg, font, RampGray(G_90), 12.f);
        float y = KNOB_ROW_1 - LABEL_OFFSET;
        CenterText(vg, KNOB_LEFT,                     y, stock ? "i"   : my_module->preset0.macro[0].c_str(), nullptr);
        CenterText(vg, KNOB_LEFT +       KNOB_SPREAD, y, stock ? "ii"  : my_module->preset0.macro[1].c_str(), nullptr);
        CenterText(vg, KNOB_LEFT + 2.f * KNOB_SPREAD, y, stock ? "iii" : my_module->preset0.macro[2].c_str(), nullptr);
        CenterText(vg, KNOB_LEFT + 3.f * KNOB_SPREAD, y, stock ? "iv"  : my_module->preset0.macro[3].c_str(), nullptr);
        CenterText(vg, KNOB_LEFT + 4.f * KNOB_SPREAD, y, stock ? "v"   : my_module->preset0.macro[4].c_str(), nullptr);
        CenterText(vg, KNOB_LEFT + 5.f * KNOB_SPREAD, y, stock ? "vi"  : my_module->preset0.macro[5].c_str(), nullptr);
        y = KNOB_ROW_2 - LABEL_OFFSET;
        CenterText(vg, RKNOB_LEFT,                     y, RecirculatorParameterName(rt, 1), nullptr);
        CenterText(vg, RKNOB_LEFT + KNOB_SPREAD,       y, RecirculatorParameterName(rt, 2), nullptr);
        CenterText(vg, RKNOB_LEFT + KNOB_SPREAD * 2.f, y, RecirculatorParameterName(rt, 3), nullptr);
        CenterText(vg, RKNOB_LEFT + KNOB_SPREAD * 3.f, y, RecirculatorParameterName(rt, 4), nullptr);
        CenterText(vg, RKNOB_LEFT + KNOB_SPREAD * 4.f, y, "Mix", nullptr);
        CenterText(vg, RKNOB_LEFT + KNOB_SPREAD * 5.f, y, "Volume", nullptr);

        nvgTextAlign(vg, NVGalign::NVG_ALIGN_LEFT);
        nvgText(vg, RKNOB_LEFT + KNOB_SPREAD * 5.f + 17.f, CV_ROW_2, "Mute", nullptr);
    }

    // status
    if (my_module) {
        float left = 60.f;
        float spacing = 6.25f;
        float y = box.size.y - 7.5f;

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
    }

    if (!my_module) {
        DrawLogo(args.vg, box.size.x*.5f - 120, 30.f, Overlay(COLOR_BRAND), 14.0);
    }
    DrawLogo(vg, box.size.x /2.f - 12.f, RACK_GRID_HEIGHT - ONE_HP, RampGray(G_90));
}

// void AddCategoryFilterItem(Menu *menu, Hc1Module * my_module, std::string name, uint16_t code)
// {
//     menu->addChild(createCheckMenuItem(name, "", 
//         [=](){ return !my_module->preset_filter.isFiltered() || my_module->preset_filter.isShow(ST); },
//         [=](){ my_module->preset_filter.toggleShow(ST); }
//     ));
// }

void Hc1ModuleWidget::addSortBy(Menu *menu, std::string name, PresetOrder order)
{
    menu->addChild(createCheckMenuItem(name, "", 
         [=](){ return my_module->preset_order == order; },
         [=](){
            my_module->preset_order = order;
            std::sort(my_module->system_presets.begin(), my_module->system_presets.end(), getPresetSort(my_module->preset_order));
            if (PresetTab::System == tab) {
                populatePresetWidgets();
                showCurrentPreset();
            }
         }
    ));
}

void Hc1ModuleWidget::addRecirculator(Menu *menu, EM_Recirculator kind)
{
    menu->addChild(createCheckMenuItem(RecirculatorName(kind), "", 
         [=](){ return my_module->recirculatorType() == kind; },
         [=](){
            my_module->recirculator =  kind | EM_Recirculator::Extend;
            my_module->sendControlChange(EM_SettingsChannel, EMCC_RecirculatorType, my_module->recirculator);
         }
    ));
}

void Hc1ModuleWidget::appendContextMenu(Menu *menu)
{
    if (!my_module) { return; }
    //menu->addChild(createMenuItem("", "", [this](){}));
    bool ready = my_module->ready();

    menu->addChild(new MenuSeparator);
    menu->addChild(createSubmenuItem("Knob control", "", [=](Menu* menu) {
        menu->addChild(createMenuItem("Center knobs", "", [=](){ my_module->centerKnobs(); }, !ready));
        menu->addChild(createMenuItem("Zero knobs", "", [=](){ my_module->zeroKnobs(); }, !ready));
        menu->addChild(createMenuItem("Absolute CV", "", [=](){ my_module->absoluteCV(); }, !ready));
        menu->addChild(createMenuItem("Relative CV", "", [=](){ my_module->relativeCV(); }, !ready));
    }));

    menu->addChild(createSubmenuItem("Recirculator", "", [=](Menu * menu) {
        addRecirculator(menu, EM_Recirculator::Reverb);
        addRecirculator(menu, EM_Recirculator::ModDelay);
        addRecirculator(menu, EM_Recirculator::SweptEcho);
        addRecirculator(menu, EM_Recirculator::AnalogEcho);
        addRecirculator(menu, EM_Recirculator::DigitalEchoLPF);
        addRecirculator(menu, EM_Recirculator::DigitalEchoHPF);
    }));

    menu->addChild(createSubmenuItem("Module", "", [=](Menu* menu) {
        menu->addChild(createMenuItem("Reboot HC-1", "",     [=](){ my_module->reboot(); }));
        menu->addChild(createCheckMenuItem("Suppress heartbeat handshake", "",
            [=](){ return !my_module->heart_beating; },
            [=](){ my_module->heart_beating = !my_module->heart_beating; }));
        menu->addChild(createMenuItem("One handshake", "",   [=](){ my_module->sendEditorPresent(); }));
        menu->addChild(createMenuItem("Request config", "",  [=](){ my_module->transmitRequestConfiguration(); }));
        menu->addChild(createMenuItem("Reset Midi I/O", "",  [=]() { my_module->resetMidiIO(); }));
    }));

    menu->addChild(createSubmenuItem("Favorites", "", [=](Menu* menu) {
        menu->addChild(createMenuItem("Clear favorites", "", [=](){ 
            my_module->clearFavorites();
            if (tab == PresetTab::Favorite) {
                updatePresetWidgets();
            }
        }, !ready));
        menu->addChild(createMenuItem("Open favorites...", "", [=]() {
            std::string path;
            std::string folder = asset::user(pluginInstance->slug);
            system::createDirectories(folder);
            if (openFileDialog(
                folder,
                "Favorites (.fav):fav;Json (.json):json;Any (*):*))",
                "",
                path)) {
                my_module->readFavoritesFile(path);
            }
            }, !ready));
        menu->addChild(createMenuItem("Save favorites as...", "", [=]() {
            std::string path;
            std::string folder = asset::user(pluginInstance->slug);
            system::createDirectories(folder);
            if (saveFileDialog(
                folder,
                "Favorites (.fav):fav;Json (.json):json;Any (*):*))",
                "my_favorites.fav",
                path)) {
                my_module->writeFavoritesFile(path);
            }
        }, !ready));
    }, !ready));

    menu->addChild(createSubmenuItem("Presets", "", [=](Menu* menu) {
        menu->addChild(createCheckMenuItem("Restore last preset on startup", "", 
            [=](){ return my_module->restore_saved_preset; },
            [=](){ my_module->restore_saved_preset = !my_module->restore_saved_preset; }
            ));
        menu->addChild(createCheckMenuItem("Use saved presets", "",
            [=](){ return my_module->cache_presets; },
            [=](){
                my_module->cache_presets = !my_module->cache_presets;
                if (my_module->cache_presets) {
                    my_module->savePresets();
                }
            }));
        menu->addChild(new MenuSeparator);
        menu->addChild(createMenuItem("Save presets", "", [=](){ my_module->savePresets(); }, !ready));
        menu->addChild(createMenuItem("Refresh User presets", "", [=](){ my_module->transmitRequestUserPresets(); }));
    }));

    menu->addChild(createSubmenuItem("Sort System presets", "", [=](Menu* menu) {
        addSortBy(menu, "Alphabetically", PresetOrder::Alpha);
        addSortBy(menu, "by Category", PresetOrder::Category);
        addSortBy(menu, "in System order", PresetOrder::System);
    }));

    // menu->addChild(createSubmenuItem("Filter by Category", "", [=](Menu* menu) {
    //     AddCategoryFilterItem(menu, my_module, "Strings", ST);
    //     AddCategoryFilterItem(menu, my_module, "Winds", WI);
    //     AddCategoryFilterItem(menu, my_module, "Vocal", VO);
    //     AddCategoryFilterItem(menu, my_module, "Keyboard", KY);
    //     AddCategoryFilterItem(menu, my_module, "Classic", CL);
    //     AddCategoryFilterItem(menu, my_module, "Other", OT);
    //     AddCategoryFilterItem(menu, my_module, "Percussion", PE);
    //     AddCategoryFilterItem(menu, my_module, "Tuned Percussion",PT);
    //     AddCategoryFilterItem(menu, my_module, "Processor", PR);
    //     AddCategoryFilterItem(menu, my_module, "Drone", DO);
    //     AddCategoryFilterItem(menu, my_module, "Midi", MD);
    //     AddCategoryFilterItem(menu, my_module, "Control Voltage",CV);
    //     AddCategoryFilterItem(menu, my_module, "Utility", UT);
    //     }));

}

} //pachde