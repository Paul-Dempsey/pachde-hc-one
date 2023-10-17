#pragma once
#ifndef VERT_SLIDER_HPP_INCLUDED
#define VERT_SLIDER_HPP_INCLUDED
#include <rack.hpp>
#include "../plugin.hpp"
#include "../colors.hpp"
#include "tip_widget.hpp"
namespace pachde {


struct VerticalSlider : SvgSlider
{
    struct SliderValueFill : TransparentWidget
    {
        Widget * reference;

        SliderValueFill(Widget * reference) : reference(reference) {}
        void draw(const DrawArgs& args) override;
    };

	VerticalSlider();

    void onButton(const ButtonEvent& e) override;
    void onHover(const HoverEvent& e) override;
    void onEnter(const EnterEvent& e) override;
    void onLeave(const LeaveEvent& e) override ;
    void onSelectKey(const SelectKeyEvent& e) override;
    void onHoverScroll(const HoverScrollEvent & e) override;
};


}
#endif