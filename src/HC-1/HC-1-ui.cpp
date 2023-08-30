#include "HC-1.hpp"
#include "cc_param.hpp"
#include "../components.hpp"
#include "../misc.hpp"
#include "../open_file.hpp"
#include "../port.hpp"
#include "../small_push.hpp"

namespace pachde {

//#define SHOW_PRESET0

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
constexpr const float PRESET_LEFT = 12.5f;
constexpr const float PRESET_WIDTH = 320.f;
constexpr const float PRESET_RIGHT = PRESET_LEFT + PRESET_WIDTH;
constexpr const float PRESET_BOTTOM = PRESET_TOP + 8.f * 27.f;

constexpr const float KNOB_LEFT   = 45.f;
constexpr const float KNOB_SPREAD = 54.f;
constexpr const float KNOB_ROW_1  = 288.f;
constexpr const float KNOB_ROW_2  = 346.f;
constexpr const float RKNOB_LEFT  = KNOB_LEFT; //- KNOB_SPREAD *.5f;

constexpr const float LABEL_OFFSET = 20.f;

constexpr const float CV_COLUMN_OFFSET = 24.f;
constexpr const float RB_OFFSET = 20.f;
constexpr const float RB_VOFFSET = 15.f;
constexpr const float CV_ROW_1 = KNOB_ROW_1 + 6.f;
constexpr const float CV_ROW_2 = KNOB_ROW_2 + 6.f;

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

    page_up = createWidget<SquareButton>(Vec(box.size.x -23.f, PRESET_TOP));
    page_up->setSymbol(SquareButtonSymbol::Up);
    page_up->onClick([this](){ pageUp(); });
    addChild(page_up);

    page_down = createWidget<SquareButton>(Vec(box.size.x -23.f, PRESET_TOP + 17.5f));
    page_up->setSymbol(SquareButtonSymbol::Down);
    page_down->onClick([this](){ pageDown(); });
    addChild(page_down);

    // Filter
    {
        auto light = createLightCentered<SmallLight<BlueLight>>(Vec(box.size.x -15.f, PRESET_BOTTOM - 24.f), my_module, Hc1Module::Lights::FILTER_LIGHT);
        light->baseColors[0] = PORT_VIOLET;
        if (my_module && my_module->filter_presets) {
            my_module->getLight(Hc1Module::Lights::FILTER_LIGHT).setBrightness(1.f);
        }
        addChild(light);
        auto filter = createWidget<SquareButton>(Vec(box.size.x - 23.f, PRESET_BOTTOM - 18.f));
        filter->setSymbol(SquareButtonSymbol::Funnel);
        filter->onClick([this]() {
            if (!my_module) return;
            my_module->filter_presets = !my_module->filter_presets;
        });
        addChild(filter);
    }

    tab_bar = createWidget<TabBarWidget>(Vec(PRESET_LEFT, PRESET_TOP - 13.f));
    tab_bar->setSize(Vec(PRESET_WIDTH, 13.f));
    tab_bar->addTab("User", PresetTab::User);
    tab_bar->addTab("Favorite", PresetTab::Favorite);
    tab_bar->addTab("System", PresetTab::System);
    tab_bar->layout();
    tab_bar->selectTab(tab);
    addChild(tab_bar);

    auto pm = createWidget<PickMidi>(Vec(7.5f, box.size.y - 16.f));
    pm->describe("Choose Midi input");
    if (my_module) {
        pm->setMidiPort(my_module);
    }
    addChild(pm);

    pm = createWidget<PickMidi>(Vec(20.f, box.size.y - 16.f));
    pm->describe("Choose Midi output");
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
    page_down->enable(true);
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

void Hc1ModuleWidget::clearPresetWidgets() {
    for (auto p: presets) {
        p->setPreset(nullptr);
    }
    have_preset_widgets = false;
}

void Hc1ModuleWidget::updatePresetWidgets() {
    if (!my_module) return;

    auto p = presets.begin();
    auto mp = my_module->getPresets(tab);
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

}

void Hc1ModuleWidget::populatePresetWidgets()
{
    if (!my_module) return;
    clearPresetWidgets();
    updatePresetWidgets();
}

void  Hc1ModuleWidget::setTab(PresetTab new_tab, bool force) {
    if (!force && (new_tab == this->tab)) return;
    if (my_module) my_module->tab = new_tab;
    tab = new_tab;
    tab_bar->selectTab(tab);
    page = my_module ? my_module->getTabPage(tab) : 0;
    updatePresetWidgets();
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
    auto font = GetPluginFontSemiBold();
    if (FontOk(font)) {
        SetTextStyle(vg, font, preset_name_color, 16.f);
        std::string text;
        if (my_module) {
            if (my_module->broken) {
                text = "[MIDI error - please wait]";
            } else
            if (InitState::Uninitialized == my_module->device_output_state) {
                text = "[looking for EM out]";
            } else
            if (InitState::Uninitialized == my_module->device_input_state) {
                text = "[looking for EM in]";
            } else
            if (my_module->is_gathering_presets()) {
                text = format_string("[gathering %s preset %d]", my_module->in_user_names ? "User" : "System", my_module->in_user_names ? my_module->user_presets.size() : my_module->system_presets.size());
            } else
            if (InitState::Uninitialized == my_module->system_preset_state) {
                text = "[preparing system presets]";
            } else
            if (InitState::Uninitialized == my_module->user_preset_state) {
                text = "[preparing user presets]";
            } else
            if (InitState::Uninitialized == my_module->config_state) {
                text = "[preparing for current configuration]";
            } else
            if (InitState::Pending == my_module->config_state) {
                text = "[processing current configuration]";
            } else
            if (my_module->current_preset) {
                text = my_module->current_preset->name;
            } else {
                text = "";
            }
        } else {
            text = "< current preset >";
        }
        CenterText(vg, box.size.x/2.f, 15.f, text.c_str(), nullptr);
    }

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
    //if (!my_module || my_module->heartbeat)
    {
        const float h = 10.f;
        const float w = 2.5f;
        const float y = KNOB_ROW_1 - 5.f;
        float x = box.size.x - 10.f - 3.f * w - 2;
        FillRect(vg, x - 1.25f, y - 1.25f, w * 3.f + 5.f, h + 5.f, RampGray(G_30));
        const uint8_t tdsp[] = {65, 30, 75};
        const uint8_t* pdsp = (my_module && my_module->ready()) ? &my_module->dsp[0] : &tdsp[0];

        if (pdsp == &tdsp[0]) {
            BoxRect(vg, x - 1.5f, y - 1.5f, w * 3.f + 4.f, h + 4.f, green_light, .5f);
            //Line(vg, x, y + h + 1, x + w*3 + 3, y + h + 1, green_light, 0.5f);
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

void Hc1ModuleWidget::draw(const DrawArgs& args)
{
    ModuleWidget::draw(args);
    auto vg = args.vg;
    auto rt = my_module ? my_module->recirculatorType() : EM_Recirculator::Reverb;

    // extender connector
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
            auto pg = format_string("%d", 1 + page);
            CenterText(vg, box.size.x - 15.25f, PRESET_TOP -2.5f, pg.c_str(), nullptr);
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
            auto y = KNOB_ROW_2 - LABEL_OFFSET - 13.f;
            auto left = RKNOB_LEFT - 35.f;
            auto w = 53.f + KNOB_SPREAD * 4.f;
            BoxRect(vg, left, y-2.f, w, 52.5f, RampGray(G_35), .5f);
            auto right = left + w;
            float bounds[4] = { 0, 0, 0, 0 };
            nvgTextBounds(vg, 0, 0, RecirculatorName(rt).c_str(), nullptr, bounds);
            bounds[2] += 15.f; // pad
            FillRect(vg, right - (w * .5f) - (bounds[2] * .5f), y - 10.f, bounds[2], 14.f, RampGray(G_08));
            SetTextStyle(vg, font, RampGray(G_90), 12.f);
            CenterText(vg, right - (w * .5f), y, RecirculatorName(rt).c_str(), nullptr);
        }
    }

    // labels
    font = GetPluginFontSemiBold();
    if (FontOk(font)) {
        SetTextStyle(vg, font, RampGray(G_90), 12.f);
        float y = KNOB_ROW_1 - LABEL_OFFSET;
        if (my_module && !my_module->anyPending()) {
            CenterText(vg, KNOB_LEFT,                     y, my_module->preset0.macro[0].c_str(), nullptr);
            CenterText(vg, KNOB_LEFT +       KNOB_SPREAD, y, my_module->preset0.macro[1].c_str(), nullptr);
            CenterText(vg, KNOB_LEFT + 2.f * KNOB_SPREAD, y, my_module->preset0.macro[2].c_str(), nullptr);
            CenterText(vg, KNOB_LEFT + 3.f * KNOB_SPREAD, y, my_module->preset0.macro[3].c_str(), nullptr);
            CenterText(vg, KNOB_LEFT + 4.f * KNOB_SPREAD, y, my_module->preset0.macro[4].c_str(), nullptr);
            CenterText(vg, KNOB_LEFT + 5.f * KNOB_SPREAD, y, my_module->preset0.macro[5].c_str(), nullptr);
        } else {
            CenterText(vg, KNOB_LEFT,                     y, "i", nullptr);
            CenterText(vg, KNOB_LEFT +       KNOB_SPREAD, y, "ii", nullptr);
            CenterText(vg, KNOB_LEFT + 2.f * KNOB_SPREAD, y, "iii", nullptr);
            CenterText(vg, KNOB_LEFT + 3.f * KNOB_SPREAD, y, "iv", nullptr);
            CenterText(vg, KNOB_LEFT + 4.f * KNOB_SPREAD, y, "v", nullptr);
            CenterText(vg, KNOB_LEFT + 5.f * KNOB_SPREAD, y, "vi", nullptr);
        }

        y = KNOB_ROW_2 - LABEL_OFFSET;
        CenterText(vg, RKNOB_LEFT,                     y, RecirculatorParameterName(rt, 1).c_str(), nullptr);
        CenterText(vg, RKNOB_LEFT + KNOB_SPREAD,       y, RecirculatorParameterName(rt, 2).c_str(), nullptr);
        CenterText(vg, RKNOB_LEFT + KNOB_SPREAD * 2.f, y, RecirculatorParameterName(rt, 3).c_str(), nullptr);
        CenterText(vg, RKNOB_LEFT + KNOB_SPREAD * 3.f, y, RecirculatorParameterName(rt, 4).c_str(), nullptr);
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
        if (my_module->heartbeat) {
            Dot(vg, left, y, InitStateColor(my_module->handshake));
        }
        left += spacing;
    }

    if (!my_module) {
        DrawLogo(args.vg, box.size.x*.5f - 80, box.size.y *.3f, Overlay(COLOR_BRAND), 8.0);
    }
    DrawLogo(vg, box.size.x /2.f - 12.f, RACK_GRID_HEIGHT - ONE_HP, RampGray(G_90));
}

void Hc1ModuleWidget::appendContextMenu(Menu *menu)
{
    if (my_module) {
        menu->addChild(new MenuSeparator);
        bool ready = my_module->ready();
        //menu->addChild(createMenuItem("", "", [this](){}));
        menu->addChild(createSubmenuItem("Advanced", "", [=](Menu* menu) {
            menu->addChild(createMenuItem("Reboot HC-1", "",     [=](){ my_module->reboot(); }));
            menu->addChild(createCheckMenuItem("Suppress heartbeat handshake", "",
                [=](){ return !my_module->heartbeat; },
                [=](){ my_module->heartbeat = !my_module->heartbeat; }));
            menu->addChild(createMenuItem("One handshake", "",   [=](){ my_module->sendEditorPresent(); }));
            menu->addChild(createMenuItem("Request config", "",  [=](){ my_module->transmitRequestConfiguration(); }));
            menu->addChild(createMenuItem("Reset Midi", "",
                [=]() { 
                    // TODO: module method
                    my_module->is_eagan_matrix = false;
                    my_module->device_name = "";
                    my_module->midi_output.reset(); 
                    my_module->midi::Input::reset();
                    my_module->input_device_id = -1;
                    my_module->device_input_state = InitState::Uninitialized;
                    my_module->device_output_state = InitState::Uninitialized;
                }));
        }));

        menu->addChild(createSubmenuItem("Favorites", "", [=](Menu* menu) {
            menu->addChild(createMenuItem("Clear favorites", "", [=](){ my_module->clearFavorites();}, !ready));
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

        menu->addChild(createCheckMenuItem("Restore last preset on startup", "", 
            [=](){ return my_module->restore_saved_preset; },
            [=](){ my_module->restore_saved_preset = !my_module->restore_saved_preset; }
            ));
        menu->addChild(createMenuItem("Save presets", "", [=](){ my_module->savePresets(); }, !ready));
        menu->addChild(createCheckMenuItem("Use saved presets", "",
            [=](){ return my_module->cache_presets; },
            [=](){
                my_module->cache_presets = !my_module->cache_presets;
                if (my_module->cache_presets) {
                    my_module->savePresets();
                }
            }));
    }
}

} //pachde