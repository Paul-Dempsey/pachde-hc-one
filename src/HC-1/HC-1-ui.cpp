#include "HC-1.hpp"
#include "cc_param.hpp"
#include "../components.hpp"
#include "../misc.hpp"
#include "../port.hpp"
#include "../small_push.hpp"

namespace pachde {
#define MIDI_ANIMATION

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

constexpr const float PRESET_TOP = 42.f;
constexpr const float PRESET_LEFT = 12.5f;
constexpr const float PRESET_WIDTH = 320.f;

constexpr const float KNOB_LEFT   = 45.f;
constexpr const float KNOB_SPREAD = 54.f;
constexpr const float KNOB_ROW_1  = 280.f;
constexpr const float KNOB_ROW_2  = 334.f;
constexpr const float RKNOB_LEFT  = KNOB_LEFT - KNOB_SPREAD *.5f;

constexpr const float CV_COLUMN_OFFSET = 21.f;
constexpr const float RB_OFFSET = 17.75f;
constexpr const float RB_VOFFSET = 14.5f;
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

    fave = createWidget<FavoriteWidget>(Vec(box.size.x - (12.f + 6.f), 6.f));
    fave->setPresetHolder(this);
    addChild(fave);

    addChild(createInputCentered<ColorPort>(Vec(KNOB_LEFT                     - CV_COLUMN_OFFSET, CV_ROW_1), module, Hc1Module::M1_INPUT));
    addChild(createInputCentered<ColorPort>(Vec(KNOB_LEFT +       KNOB_SPREAD - CV_COLUMN_OFFSET, CV_ROW_1), module, Hc1Module::M2_INPUT));
    addChild(createInputCentered<ColorPort>(Vec(KNOB_LEFT + 2.f * KNOB_SPREAD - CV_COLUMN_OFFSET, CV_ROW_1), module, Hc1Module::M3_INPUT));
    addChild(createInputCentered<ColorPort>(Vec(KNOB_LEFT + 3.f * KNOB_SPREAD - CV_COLUMN_OFFSET, CV_ROW_1), module, Hc1Module::M4_INPUT));
    addChild(createInputCentered<ColorPort>(Vec(KNOB_LEFT + 4.f * KNOB_SPREAD - CV_COLUMN_OFFSET, CV_ROW_1), module, Hc1Module::M5_INPUT));
    addChild(createInputCentered<ColorPort>(Vec(KNOB_LEFT + 5.f * KNOB_SPREAD - CV_COLUMN_OFFSET, CV_ROW_1), module, Hc1Module::M6_INPUT));

    addChild(createInputCentered<ColorPort>(Vec(RKNOB_LEFT +       KNOB_SPREAD - CV_COLUMN_OFFSET, CV_ROW_2), module, Hc1Module::R1_INPUT));
    addChild(createInputCentered<ColorPort>(Vec(RKNOB_LEFT + 2.f * KNOB_SPREAD - CV_COLUMN_OFFSET, CV_ROW_2), module, Hc1Module::R2_INPUT));
    addChild(createInputCentered<ColorPort>(Vec(RKNOB_LEFT + 3.f * KNOB_SPREAD - CV_COLUMN_OFFSET, CV_ROW_2), module, Hc1Module::R3_INPUT));
    addChild(createInputCentered<ColorPort>(Vec(RKNOB_LEFT + 4.f * KNOB_SPREAD - CV_COLUMN_OFFSET, CV_ROW_2), module, Hc1Module::R4_INPUT));
    addChild(createInputCentered<ColorPort>(Vec(RKNOB_LEFT + 5.f * KNOB_SPREAD - CV_COLUMN_OFFSET, CV_ROW_2), module, Hc1Module::RMIX_INPUT));

    addChild(createParamCentered<MidiKnob>(Vec(KNOB_LEFT                    , KNOB_ROW_1), module, Hc1Module::M1_PARAM));
    addChild(createParamCentered<MidiKnob>(Vec(KNOB_LEFT +       KNOB_SPREAD, KNOB_ROW_1), module, Hc1Module::M2_PARAM));
    addChild(createParamCentered<MidiKnob>(Vec(KNOB_LEFT + 2.f * KNOB_SPREAD, KNOB_ROW_1), module, Hc1Module::M3_PARAM));
    addChild(createParamCentered<MidiKnob>(Vec(KNOB_LEFT + 3.f * KNOB_SPREAD, KNOB_ROW_1), module, Hc1Module::M4_PARAM));
    addChild(createParamCentered<MidiKnob>(Vec(KNOB_LEFT + 4.f * KNOB_SPREAD, KNOB_ROW_1), module, Hc1Module::M5_PARAM));
    addChild(createParamCentered<MidiKnob>(Vec(KNOB_LEFT + 5.f * KNOB_SPREAD, KNOB_ROW_1), module, Hc1Module::M6_PARAM));

    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<WhiteLight>>>(Vec(KNOB_LEFT                     - RB_OFFSET, CV_ROW_1 - RB_VOFFSET), module, Hc1Module::M1_REL_PARAM, Hc1Module::M1_REL_LIGHT));
    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<WhiteLight>>>(Vec(KNOB_LEFT +       KNOB_SPREAD - RB_OFFSET, CV_ROW_1 - RB_VOFFSET), module, Hc1Module::M2_REL_PARAM, Hc1Module::M2_REL_LIGHT));
    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<WhiteLight>>>(Vec(KNOB_LEFT + 2.f * KNOB_SPREAD - RB_OFFSET, CV_ROW_1 - RB_VOFFSET), module, Hc1Module::M3_REL_PARAM, Hc1Module::M3_REL_LIGHT));
    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<WhiteLight>>>(Vec(KNOB_LEFT + 3.f * KNOB_SPREAD - RB_OFFSET, CV_ROW_1 - RB_VOFFSET), module, Hc1Module::M4_REL_PARAM, Hc1Module::M4_REL_LIGHT));
    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<WhiteLight>>>(Vec(KNOB_LEFT + 4.f * KNOB_SPREAD - RB_OFFSET, CV_ROW_1 - RB_VOFFSET), module, Hc1Module::M5_REL_PARAM, Hc1Module::M5_REL_LIGHT));
    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<WhiteLight>>>(Vec(KNOB_LEFT + 5.f * KNOB_SPREAD - RB_OFFSET, CV_ROW_1 - RB_VOFFSET), module, Hc1Module::M6_REL_PARAM, Hc1Module::M6_REL_LIGHT));

    addChild(createParamCentered<MidiKnob>(Vec(RKNOB_LEFT +       KNOB_SPREAD, KNOB_ROW_2), module, Hc1Module::R1_PARAM));
    addChild(createParamCentered<MidiKnob>(Vec(RKNOB_LEFT + 2.f * KNOB_SPREAD, KNOB_ROW_2), module, Hc1Module::R2_PARAM));
    addChild(createParamCentered<MidiKnob>(Vec(RKNOB_LEFT + 3.f * KNOB_SPREAD, KNOB_ROW_2), module, Hc1Module::R3_PARAM));
    addChild(createParamCentered<MidiKnob>(Vec(RKNOB_LEFT + 4.f * KNOB_SPREAD, KNOB_ROW_2), module, Hc1Module::R4_PARAM));
    addChild(createParamCentered<MidiKnob>(Vec(RKNOB_LEFT + 5.f * KNOB_SPREAD, KNOB_ROW_2), module, Hc1Module::RMIX_PARAM));

    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<WhiteLight>>>(Vec(RKNOB_LEFT +       KNOB_SPREAD - RB_OFFSET, CV_ROW_2 - RB_VOFFSET), module, Hc1Module::R1_REL_PARAM, Hc1Module::R1_REL_LIGHT));
    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<WhiteLight>>>(Vec(RKNOB_LEFT + 2.f * KNOB_SPREAD - RB_OFFSET, CV_ROW_2 - RB_VOFFSET), module, Hc1Module::R2_REL_PARAM, Hc1Module::R2_REL_LIGHT));
    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<WhiteLight>>>(Vec(RKNOB_LEFT + 3.f * KNOB_SPREAD - RB_OFFSET, CV_ROW_2 - RB_VOFFSET), module, Hc1Module::R3_REL_PARAM, Hc1Module::R3_REL_LIGHT));
    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<WhiteLight>>>(Vec(RKNOB_LEFT + 4.f * KNOB_SPREAD - RB_OFFSET, CV_ROW_2 - RB_VOFFSET), module, Hc1Module::R4_REL_PARAM, Hc1Module::R4_REL_LIGHT));
    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<WhiteLight>>>(Vec(RKNOB_LEFT + 5.f * KNOB_SPREAD - RB_OFFSET, CV_ROW_2 - RB_VOFFSET), module, Hc1Module::RMIX_REL_PARAM, Hc1Module::RMIX_REL_LIGHT));

    tab_bar = createWidget<TabBarWidget>(Vec(PRESET_LEFT, PRESET_TOP - 13.f));
    tab_bar->setSize(Vec(PRESET_WIDTH, 13.f));
    tab_bar->addTab("User", PresetTab::User);
    tab_bar->addTab("Favorite", PresetTab::Favorite);
    tab_bar->addTab("System", PresetTab::System);
    tab_bar->layout();
    tab_bar->selectTab(tab);
    addChild(tab_bar);

    page_up = createWidget<UpDown>(Vec(box.size.x -23.f, PRESET_TOP));
    page_up->setUp(true);
    page_up->onClick([this](){ pageUp(); });
    addChild(page_up);

    page_down = createWidget<UpDown>(Vec(box.size.x -23.f, PRESET_TOP + 17.5f));
    page_down->setUp(false);
    page_down->onClick([this](){ pageDown(); });
    addChild(page_down);

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

    auto pm = createWidget<PickMidi>(Vec(7.5f, box.size.y - 16.f));
    pm->describe("Choose Midi input");
    if (my_module) {
        pm->setMidiPort(my_module);
    }
    addChild(pm);

    pm = createWidget<PickMidi>(Vec(20.f, box.size.y - 16.f));
    pm->describe("Choose Midi output");
    if (my_module) {
        pm->setMidiPort(&my_module->midiOutput);
    }
    addChild(pm);

    auto pb = createWidgetCentered<SmallPush>(Vec(45.f, box.size.y -10.f));
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
    auto start = mp.cbegin() + 24*page;
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
        page_down->enable(start <  mp.cend()-(24*page));
    }

}

void Hc1ModuleWidget::populatePresetWidgets()
{
    if (!my_module) return;
    clearPresetWidgets();
    updatePresetWidgets();
}

void  Hc1ModuleWidget::setTab(PresetTab new_tab) {
    if (new_tab == this->tab) return;
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
        if (my_module->savedPreset) {
            fave->setPreset(my_module->savedPreset);
        }

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
        } else {
            auto selected = static_cast<PresetTab>(tab_bar->getSelectedTab());
            if (selected != tab) {
                setTab(selected);
            }
        }
    }
}

void Hc1ModuleWidget::drawLayer(const DrawArgs& args, int layer)
{
    ModuleWidget::drawLayer(args, layer);
    if (1 == layer) {
        auto vg = args.vg;
        auto font = GetPluginFontSemiBold();
        if (FontOk(font)) {
            SetTextStyle(vg, font, preset_name_color, 16.f);
            std::string text;
            if (my_module) {
                if (my_module->broken) {
                    text = "[MIDI error - Recovering...]";
                } else if (my_module->is_gathering_presets()) {
                    text = format_string("[Gathering Presets... %d]", my_module->system_presets.size());
                } else if (my_module->savedPreset) {
                    text = my_module->savedPreset->name;
                }
            } 
            CenterText(vg, box.size.x/2.f, 15.f, text.empty() ? "(Preset Name)": text.c_str(), nullptr);
        }

        // DSP status
        const float h = 10.f;
        const float w = 2.5f;
        const float y = box.size.y - 20.f - h;
        float x = box.size.x - 7.5f - 3.f * w - 2;
        FillRect(vg, x - 1.25f, y - 1.25f, w * 3.f + 5.f, h + 5.f, RampGray(G_30));
        const uint8_t tdsp[] = {65, 30, 75};
        const uint8_t* pdsp = (my_module && my_module->hasPresets()) ? &my_module->dsp[0] : &tdsp[0];
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

#ifdef MIDI_ANIMATION
    if (my_module && my_module->anyPending()) {
        // raw MIDI animation
        Circle(vg, PRESET_LEFT + ((my_module->midi_receive_count / 50) % 320), 2.25f, 2.f, blue_green_light);
    }
#endif
    auto font = GetPluginFontRegular();
    if (FontOk(font)) {
        SetTextStyle(vg, font, RampGray(G_90), 12.f);

        { // page
            auto pg = format_string("%d", 1 + page);
            CenterText(vg, box.size.x - 15.25f, PRESET_TOP -2.5f, pg.c_str(), nullptr);
        }
        
        { // device
            std::string device_name;
            if (my_module) {
                device_name = FilterDeviceName(my_module->deviceName());
            } else {
                device_name = "<Eagan Matrix Device>";
            }
            if (device_name.empty()) {
                device_name = "(no Eagan Matrix available)";
            }
            nvgTextAlign(vg, NVGalign::NVG_ALIGN_LEFT);
            nvgText(vg, box.size.x*.5f + 25.f, box.size.y - 3.f, device_name.c_str(), nullptr);
        }

        // firmware
        if (my_module && my_module->is_eagan_matrix && (my_module->firmware_version > 0)) {
            RightAlignText(vg, box.size.x - 7.5, box.size.y - 3.f, format_string("v %.2f", my_module->firmware_version/100.f).c_str(), nullptr);
        } else {
            RightAlignText(vg, box.size.x - 7.5, box.size.y - 3.f, "v 00.00", nullptr);
        }

        { // recirculator
            auto y = KNOB_ROW_2 - 18.5f;
            Line(vg, 7.5f, y-2.f, box.size.x - 7.5f, y-2.f, RampGray(G_35), .5f);
            float bounds[4] = { 0, 0, 0, 0 };
            nvgTextBounds(vg, 0, 0, RecirculatorName(rt).c_str(), nullptr, bounds);
            bounds[2] += 15.f; // pad
            FillRect(vg, (box.size.x * .5f) - (bounds[2] * .5f), y - 10.f, bounds[2], 14.f, RampGray(G_08));
            SetTextStyle(vg, font, RampGray(G_90), 12.f);
            CenterText(vg, box.size.x * .5f, y, RecirculatorName(rt).c_str(), nullptr);
        }
    }

    // labels
    font = GetPluginFontSemiBold();
    if (FontOk(font)) {
        SetTextStyle(vg, font, RampGray(G_90), 12.f);
        float y = KNOB_ROW_1 + 22.f;
        if (my_module/* && my_module->hasConfig()*/) {
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

        y = KNOB_ROW_2 + 22.f;
        CenterText(vg, RKNOB_LEFT + KNOB_SPREAD,       y, RecirculatorParameterName(rt, 1).c_str(), nullptr);
        CenterText(vg, RKNOB_LEFT + KNOB_SPREAD * 2.f, y, RecirculatorParameterName(rt, 2).c_str(), nullptr);
        CenterText(vg, RKNOB_LEFT + KNOB_SPREAD * 3.f, y, RecirculatorParameterName(rt, 3).c_str(), nullptr);
        CenterText(vg, RKNOB_LEFT + KNOB_SPREAD * 4.f, y, RecirculatorParameterName(rt, 4).c_str(), nullptr);
        CenterText(vg, RKNOB_LEFT + KNOB_SPREAD * 5.f, y, "Mix", nullptr);
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

        Dot(vg, left + 4.f * spacing, y, (my_module->broken || my_module->stateError()) ? red_light : my_module->anyPending() ? yellow_light : green_light);

        on = my_module->notesOn > 0;
        Dot(vg, left + 5.f * spacing, y, on ? purple_light : gray_light, on);

        // handshake tick/tock
        // if (my_module) {
        //     Circle(vg, left + 7.f * spacing + (my_module->tick_tock ? 1.5f : -1.5f), y, 1.25f, COLOR_MAGENTA);
        // }

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
        menu->addChild(createMenuItem("Reboot HC-1", "",     [this](){ my_module->reboot(); }));
        menu->addChild(createMenuItem("Mute", "",            [this](){ my_module->silence(true); }));
        menu->addChild(createMenuItem("Clear favorites", "", [this](){ my_module->favorite_presets.clear(); }));
        menu->addChild(createMenuItem("Save presets", "",    [this](){ my_module->savePresets(); }));
        menu->addChild(createCheckMenuItem("Use saved presets", "",
            [this](){ return my_module->cache_presets; },
            [this](){
                my_module->cache_presets = !my_module->cache_presets;
                if (my_module->cache_presets) {
                    my_module->savePresets();
                }
            }));
    }
}

} //pachde