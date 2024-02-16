// Copyright (C) Paul Chase Dempsey
#pragma once
#ifndef INDICATOR_WIDGET_INCLUDED
#define INDICATOR_WIDGET_INCLUDED
#include "../colors.hpp"
#include "tip_widget.hpp"

namespace pachde {
// indicator with callbacks for color, optional fill, and tip text
struct IndicatorWidget: TipWidget
{
    std::function<const NVGcolor& ()> get_color;
    std::function<bool ()> get_fill;
    std::function<std::string ()> get_tip;
    std::string title;
    IndicatorWidget()
    : get_color(nullptr), get_fill(nullptr), get_tip(nullptr)
    {
        box.size.x = box.size.y = 4.5f;
    }

    void onEnter(const EnterEvent& e) override {
        if (get_tip) {
            describe(format_string("%s: %s", title.c_str(), get_tip().c_str()));
        }
        TipWidget::onEnter(e);
    }

    void draw(const DrawArgs& args) override
    {
        Dot(args.vg, box.size.x*.5, box.size.y*.5, get_color(), get_fill ? get_fill() : true);
    }
};

IndicatorWidget * createIndicatorCentered(float x, float y, std::string name, std::function<const NVGcolor& ()> getColor)
{
    IndicatorWidget* w = createWidgetCentered<IndicatorWidget>(Vec(x,y));
    w->title = name;
    w->describe(name);
    w->get_color = getColor;
    return w;
}

IndicatorWidget * createIndicatorCentered(float x, float y, 
    std::string name, std::function<const NVGcolor&()> getColor,
    std::function<bool()> getFill)
{
    IndicatorWidget* w = createWidgetCentered<IndicatorWidget>(Vec(x,y));
    w->title = name;
    w->describe(name);
    w->get_color = getColor;
    w->get_fill = getFill;
    return w;
}
IndicatorWidget * createIndicatorCentered(float x, float y, std::string name,
    std::function<const NVGcolor&()> getColor,
    std::function<std::string()> getTip,
    std::function<bool()> getFill = nullptr
    )
{
    IndicatorWidget* w = createWidgetCentered<IndicatorWidget>(Vec(x,y));
    w->title = name;
    if (!getTip) {
        w->describe(name);
    }
    w->get_tip = getTip;
    w->get_color = getColor;
    w->get_fill = getFill;
    return w;
}

}
#endif