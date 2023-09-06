#pragma once
#ifndef SQUARE_BUTTON_HPP_INCLUDED
#define SQUARE_BUTTON_HPP_INCLUDED
#include "plugin.hpp"
#include "colors.hpp"
#include "tip_widget.hpp"

namespace pachde {

enum SquareButtonSymbol {
    Up,
    Down,
    Funnel
};

struct SquareButton : TipWidget {
    bool pressed = false;
    bool enabled = true;
    SquareButtonSymbol symbol = SquareButtonSymbol::Up;
    std::function<void()> handler;

    SquareButton() {
        box.size.x = 15.f;
        box.size.y = 16.f;
    }

    void setSymbol(SquareButtonSymbol sym) {
        symbol = sym;
    }

    void enable(bool enable = true) {
        enabled = enable;
    }

    void onClick(std::function<void()> callback) {
        handler = callback;
    }

    void describe(std::string description) {
        tip_text = description;
    }

    void onDragStart(const DragStartEvent& e) override {
        if (enabled) pressed = true;
        TipWidget::onDragStart(e);
    }

    void onDragEnd(const DragEndEvent& e) override
    {
        TipWidget::onDragEnd(e);
        pressed = false;
        if (e.button != GLFW_MOUSE_BUTTON_LEFT)
            return;

        if (handler && enabled) {
            handler();
        }
    }


    void drawUpFace(NVGcontext* vg) {
        FillRect(vg, 0.5f, 1.f, 14.f, 15.f, nvgRGBAf(0.f,0.f,0.f,.75f));
        GradientRect(vg, 0.5f, 0.75f, 14.f, 14.f, RampGray(G_75), RampGray(G_25), 0.f, 8.f);
        GradientRect(vg, .85f, 1.125f, 13.25f, 13.5f, RampGray(G_35), RampGray(G_20), 0.f, 15.f);
    }

    void drawFunnel(NVGcontext* vg) {
        nvgBeginPath(vg);
        nvgMoveTo(vg, 4.f, 4.f);
        nvgLineTo(vg, 11.f, 4.f );
        nvgLineTo(vg, 8.2f, 8.f );
        nvgLineTo(vg, 7.9f, 11.f );
        nvgLineTo(vg, 7.1f, 11.f );
        nvgLineTo(vg, 6.8f, 8.f );
        //nvgFillColor(vg, RampGray(G_85));
        auto fill = nvgLinearGradient(vg, 7.5f, 4.f, 7.5f, 11.f,
            pressed ? RampGray(G_65) : RampGray(G_45),
            pressed ? RampGray(G_45) : RampGray(G_85));
        nvgFillPaint(vg, fill);
        nvgClosePath(vg);
        nvgFill(vg);
    }

    void drawDownSymbol(NVGcontext* vg) {
        nvgBeginPath(vg);
        nvgFillColor(vg, RampGray(G_85));
        nvgMoveTo(vg, 7.5f, 4.f);
        nvgLineTo(vg, 5.f, 8.f );
        nvgLineTo(vg, 10.f, 8.f);
        nvgClosePath(vg);
        nvgFill(vg);
    }

    void drawUpSymbol(NVGcontext* vg) {
        nvgBeginPath(vg);
        nvgFillColor(vg, RampGray(G_85));
        nvgMoveTo(vg, 5.f, 8.f);
        nvgLineTo(vg, 10.f, 8.f);
        nvgLineTo(vg, 7.5f, 12.f);
        nvgFill(vg);
    }

    void draw(const DrawArgs& args) override {
        auto vg = args.vg;
        FillRect(vg, 0.f, 0.f, 15.f, 15.f, RampGray(G_0));
        FillRect(vg, 0.f, 0.f, 15.f, 0.5f, nvgRGBAf(.4f,.4f,.4f,.65f));
        //FillRect(vg, 0.f, 14.5f, 15.f, 0.5f, RampGray(G_65));
        if (enabled) {
            if (pressed) {
                FillRect(vg, 0.5f, 1.f, 14.f, 14.25f, nvgRGBAf(0.f,0.f,0.f,.75f));
                GradientRect(vg, 0.5f, 0.75f, 14.f, 14.f, RampGray(G_25), RampGray(G_75), 8.f, 15.f);
                GradientRect(vg, .85f, 1.125f, 13.25f, 13.5f, RampGray(G_20), RampGray(G_35), 0.f, 15.f);
            } else {
                drawUpFace(vg);
            }
            switch (symbol) {
            case SquareButtonSymbol::Up: drawUpSymbol(vg); break;
            case SquareButtonSymbol::Down: drawDownSymbol(vg); break;
            case SquareButtonSymbol::Funnel: drawFunnel(vg); break;
            }
        } else {
            drawUpFace(vg);
            OpenCircle(vg, 7.5f, 8.f, 3.f, RampGray(G_45), 0.5f);
            Line(vg, 7.f, 8.f, 8.f, 8.f, RampGray(G_50), 0.5f);
        }
    }
};

}
#endif