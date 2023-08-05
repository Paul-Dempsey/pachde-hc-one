#pragma once
#ifndef COMPONENTS_HPP_INCLUDED
#define COMPONENTS_HPP_INCLUDED
#include "plugin.hpp"
#include "colors.hpp"

namespace pachde {
constexpr const float ONE_HP = 15.0f;
constexpr const float TWO_HP = 30.0f;
constexpr const float HALF_HP = 7.5f;

void DrawLogo(NVGcontext * vg, float x, float y, NVGcolor fill, float scale = 1.0f);
void FillHeart(NVGcontext* vg, float x, float y, float square, NVGcolor fill);
void StrokeHeart(NVGcontext* vg, float x, float y, float square, NVGcolor stroke, float stroke_width = 1.0f);

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

}
#endif