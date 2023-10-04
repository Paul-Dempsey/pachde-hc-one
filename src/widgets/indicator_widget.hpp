#pragma once
#ifndef INDICATOR_WIDGET_INCLUDED
#define INDICATOR_WIDGET_INCLUDED
#include "../colors.hpp"
#include "tip_widget.hpp"

namespace pachde {
// indicator with callbacks for color and optional fill
struct IndicatorWidget: TipWidget
{
    std::function<const NVGcolor& ()> get_color;
    std::function<bool ()> get_fill;
    IndicatorWidget()
    : get_color(nullptr), get_fill(nullptr)
    {
        box.size.x = box.size.y = 5.f;
    }

    void draw(const DrawArgs& args) override
    {
        Dot(args.vg, box.size.x*.5, box.size.y*.5, get_color(), get_fill ? get_fill() : true);
    }
};

IndicatorWidget * createIndicatorCentered(float x, float y, std::string name, std::function<const NVGcolor& ()> getColor)
{
    IndicatorWidget* w = createWidgetCentered<IndicatorWidget>(Vec(x,y));
    w->describe(name);
    w->get_color = getColor;
    return w;
}

IndicatorWidget * createIndicatorCentered(float x, float y, std::string name, std::function<const NVGcolor&()> getColor, std::function<bool()> getFill)
{
    IndicatorWidget* w = createWidgetCentered<IndicatorWidget>(Vec(x,y));
    w->describe(name);
    w->get_color = getColor;
    w->get_fill = getFill;
    return w;
}

}
#endif