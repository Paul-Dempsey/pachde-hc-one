#pragma once
#ifndef COMPONENTS_HPP_INCLUDED
#define COMPONENTS_HPP_INCLUDED
#include "plugin.hpp"
#include "colors.hpp"

//#define IMPLEMENT_DRAW_PERSON
namespace pachde {
constexpr const float ONE_HP = 15.0f;
constexpr const float TWO_HP = 30.0f;
constexpr const float HALF_HP = 7.5f;

void DrawLogo(NVGcontext * vg, float x, float y, NVGcolor fill, float scale = 1.0f);
void FillHeart(NVGcontext* vg, float x, float y, float square, NVGcolor fill);
void StrokeHeart(NVGcontext* vg, float x, float y, float square, NVGcolor stroke, float stroke_width = 1.0f);
#if defined IMPLEMENT_DRAW_PERSON
void StrokePerson(NVGcontext* vg, float x, float y, float square, NVGcolor stroke, float stroke_width = 1.0f);
void FillPerson(NVGcontext* vg, float x, float y, float square, NVGcolor fill);
#endif
void DrawKnobTrack(NVGcontext* vg, const Knob * w, float track_radius, float track_width, const NVGcolor& color);

// struct USB_A_Port : app::SvgPort {

// 	USB_A_Port() {
// 		SvgPort::setSvg(Svg::load(asset::plugin(pluginInstance, "res/USB-A-blue.svg")));
// 	}

//     void guideColor(const NVGcolor& co) {
//         if (nullptr == sw || nullptr == sw->svg || nullptr == sw->svg->handle || nullptr == sw->svg->handle->shapes) {
//             return;
//         }
//         PackedColor color = toPacked(co);
//         auto filled = 0;
//         for (auto shape = this->sw->svg->handle->shapes; shape && (filled < 2); shape = shape->next) {
//             auto d = shape->id[0];
//             if ((0 == shape->id[1]) && (d == 'b' || d == 't')) {
//                 shape->fill.color = color;
//                 ++filled;
//             }
//         }
//         this->fb->dirty = true;
//     }
// };

struct PickMidi : app::MidiButton {
    rack::ui::Tooltip* tip = nullptr;
    std::string text;

    virtual ~PickMidi() {
        if (tip) delete tip;
        tip = nullptr;       
    }
    void describe(std::string description) {
        text = description;
    }
    void createTip() {
        if (!rack::settings::tooltips) return;
        if (tip) return;
        tip = new Tooltip;
        tip->text = text;
        APP->scene->addChild(tip);
    }
    void destroyTip() {
        if (!tip) return;
        auto t = tip;
        tip = nullptr;
	    APP->scene->removeChild(t);
        delete t;
    }
    void onEnter(const EnterEvent& e) override {
        createTip();
    }
    void onLeave(const LeaveEvent& e) override {
        destroyTip();
    }

    PickMidi() {
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/MIDI.svg")));
		shadow->opacity = 0.0;
    }
};


template<typename TSvg>
struct TKnob: rack::RoundKnob
{
    bool clickStepValue = true;
    float stepIncrementBy = 1.f;
    bool key_modified = false;
    bool key_modified2 = false;

    TKnob()
    {
        setImage();
    }

    std::function<void(void)> clickHandler;
    void onClick(std::function<void(void)> callback)
    {
        clickHandler = callback;
    }

    void onHoverKey(const HoverKeyEvent& e) override {
        rack::RoundKnob::onHoverKey(e);
        key_modified = (e.mods & RACK_MOD_MASK) & RACK_MOD_CTRL;
        key_modified2 = (e.mods & RACK_MOD_MASK) & GLFW_MOD_SHIFT;
    }

    void onAction(const ActionEvent& e) override {
        rack::RoundKnob::onAction(e);
        if (clickHandler) {
            clickHandler();
        } else if (clickStepValue) {
            auto pq = getParamQuantity();
            if (pq) {
                float value = pq->getValue();
                if (!key_modified) {
                    auto lim = pq->getMaxValue();
                    if (value == lim) {
                        value = pq->getMinValue();
                    } else {
                        value = value + (key_modified2 ? stepIncrementBy * 10 : stepIncrementBy);
                        if (value > lim) {
                            value = lim;
                        }
                    }
                } else {
                    auto lim = pq->getMinValue();
                    if (value == lim) {
                        value = pq->getMaxValue();
                    } else {
                        value = value - (key_modified2 ? stepIncrementBy * 10 : stepIncrementBy);
                        if (value < pq->getMinValue()) {
                            value = lim;
                        }
                    }
                }
                pq->setValue(value);
            }
        }
        if (fb) {
            fb->setDirty(true);
        }
    }

    void setImage() {
        if (bg && bg->svg) return;
        if (TSvg::isSystemSvg()) {
            setSvg(Svg::load(asset::system(TSvg::knob())));
            bg->setSvg(Svg::load(asset::system(TSvg::background())));
        } else {
            setSvg(Svg::load(asset::plugin(pluginInstance, TSvg::knob())));
            bg->setSvg(Svg::load(asset::plugin(pluginInstance, TSvg::background())));
        }
        if (fb) {
            fb->setDirty(true);
        }
    }
};

struct SmallBlackKnobSvg {
    static bool isSystemSvg() { return true; }
    static std::string knob() {
        return "res/ComponentLibrary/RoundSmallBlackKnob.svg";
    }
    static std::string background() {
        return "res/ComponentLibrary/RoundSmallBlackKnob_bg.svg";
    }
};
using SmallBlackKnob = TKnob<SmallBlackKnobSvg>;


}
#endif