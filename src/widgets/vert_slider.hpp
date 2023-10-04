#pragma once
#ifndef VERT_SLIDER_HPP_INCLUDED
#define VERT_SLIDER_HPP_INCLUDED
#include <rack.hpp>
#include "../plugin.hpp"
#include "../colors.hpp"
#include "tip_widget.hpp"
namespace pachde {

//static const NVGcolor slider_bg = nvgRGB(0x2d, 0x5d, 0x86); //#2d5d86
static const NVGcolor value_bg = nvgRGB(0x8f, 0x7a, 0x25);

struct VerticalSlider : SvgSlider
{
    struct SliderValueFill : TransparentWidget
    {
        Widget * reference;
        SliderValueFill(Widget * reference) : reference(reference) {}
        void draw(const DrawArgs& args) override {
            auto break_pos = reference->box.pos.y + 4;
            FillRect(args.vg, 1.5f, break_pos, box.size.x - 3.f, box.size.y - break_pos, value_bg);
        }
    };
	VerticalSlider()
    {
		setBackgroundSvg(Svg::load(asset::plugin(pluginInstance, "res/VertSlider_bg.svg")));
		setHandleSvg(Svg::load(asset::plugin(pluginInstance, "res/VertSliderThumb.svg")));
        setHandlePosCentered(Vec(10.f, 198.f), Vec(10.f, 5.f));
        auto fill = new SliderValueFill(handle);
        fill->box.size = box.size;
        fb->addChildBelow(fill, handle);
        handle->box.pos.y = 198.f;
    }
};


}
#endif