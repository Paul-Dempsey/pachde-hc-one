#pragma once
#ifndef SYMBOL_WIDGET_HPP_INCLUDE
#define SYMBOL_WIDGET_HPP_INCLUDE
#include <rack.hpp>
#include "plugin.hpp"
#include "preset_meta.hpp"

using namespace ::rack;
namespace pachde {

struct SymbolWidget : widget::Widget
{
	widget::FramebufferWidget* fb;
	widget::SvgWidget* sw;

	SymbolWidget() {
        fb = new widget::FramebufferWidget;
        addChild(fb);
        sw = new widget::SvgWidget;
        fb->addChild(sw);
    }

    void setSymbol(uint16_t category) {
        const char * res_path = nullptr;
        switch (category) {
        case 0: res_path = "res/blank.svg"; break;
        case ST: res_path = "res/viol.svg"; break;
        case WI: res_path = "res/wind.svg"; break;
        case VO: res_path = "res/vocal.svg"; break;
        case KY: res_path = "res/keys.svg"; break;
        case CL: res_path = "res/classic.svg"; break;
        case PE: res_path = "res/drum.svg"; break;
        case PT: res_path = "res/marimba.svg"; break;
        case PR: res_path = "res/gear.svg"; break;
        case DO: res_path = "res/drone.svg"; break;
        case MD: res_path = "res/midi-din.svg"; break;
        case CV: res_path = "res/cv-jack.svg"; break;
        case UT: res_path = "res/tools.svg"; break;
        case OT:
        default: res_path = "res/other.svg"; break;
        }
        setSvg(Svg::load(asset::plugin(pluginInstance, res_path)));
    }

	void setSvg(std::shared_ptr<window::Svg> svg) {
        if (sw->svg == svg)
            return;
        sw->setSvg(svg);
        fb->box.size = sw->box.size;
        box.size = sw->box.size;
        fb->setDirty();
    }
};

}
#endif