#include "HC-1.hpp"
#include "cc_param.hpp"
#include "../components.hpp"
#include "../small_push.hpp"
#include "../port.hpp"

namespace pachde {

std::string FilterDeviceName(std::string text) {
    #ifdef ARCH_WIN
    if (!text.empty()) {
        text.erase(text.find_last_not_of("0123456789"));
    }
    #endif
    return text;
}

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

const NVGcolor blue_light         = nvgHSL(220.f/360.f, 0.85f, 0.5f);
const NVGcolor green_light        = nvgHSL(120.f/360.f, 0.85f, 0.5f);
const NVGcolor bright_green_light = nvgHSL(120.f/360.f, 0.85f, 0.9f);
const NVGcolor orange_light       = nvgHSL(30.f/360.f, 0.85f, 0.5f);
const NVGcolor yellow_light       = nvgHSL(60.f/360.f, 0.85f, 0.5f);
const NVGcolor red_light          = nvgHSL(0.f, 0.85f, 0.5f);
const NVGcolor white_light        = nvgRGB(0xef, 0xef, 0xef);
const NVGcolor purple_light       = nvgHSL(270.f/360.f, 0.85f, 0.5f);
const NVGcolor preset_name_color  = nvgRGB(0xe6, 0xa2, 0x1a);

constexpr const float KNOB_LEFT   = 45.f;
constexpr const float KNOB_SPREAD = 54.f;
constexpr const float KNOB_ROW_1  = 42.f;
constexpr const float KNOB_ROW_2  = 85.f;

constexpr const float CV_COLUMN_OFFSET = 21.f;
constexpr const float CV_ROW_1 = KNOB_ROW_1 + 6.f;
constexpr const float CV_ROW_2 = KNOB_ROW_2 + 6.f;

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
                module->sendNoteOff(0, 60);
            } else {
                module->sendNote(0, 60, 64);
            }
        });
    }
    addChild(pb);
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
    }

}

void Hc1ModuleWidget::drawLayer(const DrawArgs& args, int layer)
{
    ModuleWidget::drawLayer(args, layer);
    if (1 == layer) {
        auto vg = args.vg;
        // auto color = my_module ? nvgHSL(360.f/210.f, .5f, my_module->note/128.f) : COLOR_BRAND;
        // Circle(args.vg, 12.f, 17.f, 3.f, color); // todo: halo, device state
        auto font = GetPluginFontSemiBold();
        if (FontOk(font)) {
            SetTextStyle(vg, font, preset_name_color, 16.f);
            if (my_module) {
                CenterText(vg, box.size.x/2.f, 20.f, my_module->preset0.name(), nullptr);
                //nvgText(vg, 130.f, 32.f, my_module->preset0.text(), nullptr);
            } else {
                CenterText(vg, box.size.x/2.f, 20.f, "(preset name)", nullptr);
                //nvgText(vg, 130.f, 32.f, "(preset text)", nullptr);
            }
        }
        if (my_module) {
            auto h = 10.f;
            auto w = 2.5f;
            auto x = box.size.x - 7.5f - 3.f*w - 2;
            auto y = box.size.y - 20.f - h;
            for (auto n = 0; n < 3; n++) {
                auto pct = my_module->dsp[n];
                auto co = pct < 85 ? green_light : red_light;
                auto bh = h * (pct / 100.f);
                FillRect(vg, x, y + h - bh, w, bh, co);
                x += w + 1;
            }
        } else {

        }
    }
}

void Hc1ModuleWidget::draw(const DrawArgs& args)
{
    ModuleWidget::draw(args);
    auto vg = args.vg;

    Circle(vg, 12.f, 20.f, 3.f, my_module && my_module->waiting_for_handshake ? orange_light : blue_light );
        if (my_module) {
        Circle(vg, 12.f + (my_module->tick_tock ? 1.5f : -1.5f), 26.f, 1.25f, COLOR_MAGENTA);
        }

    auto font = GetPluginFontRegular();
    if (FontOk(font)) {
        std::string device_name;
        if (my_module) {
            device_name = FilterDeviceName(my_module->deviceName());
        } else {
            device_name = "Continuum #######";
        }
        if (device_name.empty()) {
            device_name = "(suitable device not found)";
        }
        SetTextStyle(vg, font, RampGray(G_90), 12.f);
        nvgText(vg, box.size.x/2.f + 25.f, box.size.y - 7.5f, device_name.c_str(), nullptr);

        if (my_module) {
            RightAlignText(vg, box.size.x - 7.5, box.size.y - 7.5f, format_string("v %.2f", my_module->firmware_version/100.f).c_str(), nullptr);
            if (my_module->download_message_id >= 0)
            {
                auto message = DownloadMessage(my_module->download_message_id);
                if (nullptr != message) {
                    nvgText(vg, 22.f, 34.f, message, nullptr);
                }
            }
        }
    }
    font = GetPluginFontSemiBold();
    if (FontOk(font)) {
        SetTextStyle(vg, font, RampGray(G_90), 12.f);
        float y = KNOB_ROW_1 + 22.f;
        if (my_module) {
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
    DrawLogo(vg, box.size.x /2.f - 12.f, RACK_GRID_HEIGHT - ONE_HP, RampGray(G_90));
}

// Add options to your module's menu here
// void appendContextMenu(Menu *menu) override
//{
//}

} //pachde