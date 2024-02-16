// Copyright (C) Paul Chase Dempsey
#pragma once
#ifndef SYMBOL_WIDGET_HPP_INCLUDE
#define SYMBOL_WIDGET_HPP_INCLUDE
#include <rack.hpp>
#include "../plugin.hpp"
#include "../preset_meta.hpp"

using namespace ::rack;
namespace pachde {

struct SymbolWidget : widget::Widget
{
    enum class Symbol {
        Blank,
        Person,
        NoPedal, FirstPedal = NoPedal,
        SwitchPedal,
        ExpressionPedal,
        DamperPedal,
        TriValuePedal,
        CVPedal,
        PotPedal,
        OtherPedal, LastPedal = OtherPedal
    };
    widget::FramebufferWidget* fb;
    widget::SvgWidget* sw;
    bool visible;

	SymbolWidget() : visible(true) {
        fb = new widget::FramebufferWidget;
        addChild(fb);
        sw = new widget::SvgWidget;
        fb->addChild(sw);
    }

    void setVisible(bool shown)
    {
        visible = shown;
    }

    void setSymbol(Symbol category)
    {
        const char * res_path = nullptr;
        switch (category) {
        default:
        case Symbol::Blank: res_path = "res/blank.svg"; break;
        case Symbol::Person: res_path = "res/person.svg"; break;

        case Symbol::NoPedal :        res_path = "res/ped-none.svg"; break;
        case Symbol::SwitchPedal:     res_path = "res/ped-switch.svg"; break;
        case Symbol::ExpressionPedal: res_path = "res/ped-expression.svg"; break;
        case Symbol::DamperPedal:     res_path = "res/ped-damper.svg"; break;
        case Symbol::TriValuePedal:   res_path = "res/ped-tri.svg"; break;
        case Symbol::CVPedal:         res_path = "res/ped-cv.svg"; break;
        case Symbol::PotPedal:        res_path = "res/ped-pot.svg"; break;
        case Symbol::OtherPedal:      res_path = "res/ped-other.svg"; break;

        // case ST: res_path = "res/viol.svg"; break;
        // case WI: res_path = "res/wind.svg"; break;
        // case VO: res_path = "res/vocal.svg"; break;
        // case KY: res_path = "res/keys.svg"; break;
        // case CL: res_path = "res/classic.svg"; break;
        // case PE: res_path = "res/drum.svg"; break;
        // case PT: res_path = "res/marimba.svg"; break;
        // case PR: res_path = "res/gear.svg"; break;
        // case DO: res_path = "res/drone.svg"; break;
        // case MD: res_path = "res/midi-din.svg"; break;
        // case CV: res_path = "res/cv-jack.svg"; break;
        // case UT: res_path = "res/tools.svg"; break;
        // case OT:
        //default: res_path = "res/other.svg"; break;
        }
        setSvg(Svg::load(asset::plugin(pluginInstance, res_path)));
    }

	void setSvg(std::shared_ptr<window::Svg> svg)
    {
        if (sw->svg == svg)
            return;
        sw->setSvg(svg);
        fb->box.size = sw->box.size;
        box.size = sw->box.size;
        fb->setDirty();
    }
    void draw(const DrawArgs& args) override
    {
        if (visible) {
            Widget::draw(args);
        }
    }
};


struct SymbolTipWidget : TipWidget
{
    using Symbol = SymbolWidget::Symbol;
    SymbolWidget* symbol;

    SymbolTipWidget()
    {
        addChild(symbol = new SymbolWidget());
    }
    void setVisible(bool shown)
    {
        symbol->setVisible(shown);
    }
    void setSymbol(Symbol category)
    {
        switch (category) {
        default:
        case Symbol::Blank: 
        case Symbol::Person: describe(""); break;

        case Symbol::NoPedal :        describe("No pedal"); break;
        case Symbol::SwitchPedal:     describe("Switch pedal"); break;
        case Symbol::ExpressionPedal: describe("Expression pedal"); break;
        case Symbol::DamperPedal:     describe("Damper pedal"); break;
        case Symbol::TriValuePedal:   describe("Tri-value pedal"); break;
        case Symbol::CVPedal:         describe("CV pedal"); break;
        case Symbol::PotPedal:        describe("Pot pedal"); break;
        case Symbol::OtherPedal:      describe("Other pedal"); break;
        }
        symbol->setSymbol(category);
        box.size = symbol->box.size;
    }
};

template <typename SWT>
SWT * createSymbolWidget(float x, float y, SymbolWidget::Symbol symbol = SymbolWidget::Symbol::Blank, bool visible = true)
{
    auto w = createWidget<SWT>(Vec(x,y));
    w->setSymbol(symbol);
    w->setVisible(visible);
    return w;
}

}
#endif