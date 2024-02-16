// Copyright (C) Paul Chase Dempsey
#pragma once
#ifndef SQUARE_BUTTON_HPP_INCLUDED
#define SQUARE_BUTTON_HPP_INCLUDED
#include "../plugin.hpp"
#include "../colors.hpp"
#include "tip_widget.hpp"

namespace pachde {

enum class SquareButtonSymbol : uint8_t {
    Nil,
    Up,
    Down,
    Left, 
    Right,
    Funnel
};

struct ButtonBehavior : TipWidget {
    bool pressed = false;
    bool enabled = true;
    std::function<void()> handler;

    void enable(bool enable = true) {
        enabled = enable;
    }

    void onClick(std::function<void()> callback) {
        handler = callback;
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
};

struct DrawSquareButton
{
    NVGcolor base_screen = nvgRGBAf(0.f,0.f,0.f,.75f);
    NVGcolor med_screen = nvgRGBAf(.4f,.4f,.4f,.65f);
    NVGcolor hi1 =  RampGray(G_75);
    NVGcolor hi2 =  RampGray(G_35);
    NVGcolor face1 = RampGray(G_35);
    NVGcolor face2 = RampGray(G_20);

    void drawUpFace(NVGcontext* vg) {
        FillRect(vg, 0.5f, 1.f, 14.f, 15.f, base_screen);
        GradientRect(vg, 0.5f, 0.75f, 14.f, 14.f, hi1, hi2, 0.f, 8.f);
        GradientRect(vg, .85f, 1.125f, 13.25f, 13.5f, face1, face2, 0.f, 15.f);
    }

    void drawDownFace(NVGcontext* vg) {
        FillRect(vg, 0.5f, 1.f, 14.f, 14.25f, base_screen);
        GradientRect(vg, 0.5f, 0.75f, 14.f, 14.f, hi2, hi1, 8.f, 15.f);
        GradientRect(vg, .85f, 1.125f, 13.25f, 13.5f, face2, face1, 0.f, 15.f);
    }

    void drawFunnel(NVGcontext* vg, bool pressed) {
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

    void drawUpSymbol(NVGcontext* vg) {
        nvgBeginPath(vg);
        nvgFillColor(vg, RampGray(G_85));
        nvgMoveTo(vg, 7.5f, 4.f);
        nvgLineTo(vg, 5.f, 8.f );
        nvgLineTo(vg, 10.f, 8.f);
        nvgClosePath(vg);
        nvgFill(vg);
    }

    void drawDownSymbol(NVGcontext* vg) {
        nvgBeginPath(vg);
        nvgFillColor(vg, RampGray(G_85));
        nvgMoveTo(vg, 5.f, 8.f);
        nvgLineTo(vg, 10.f, 8.f);
        nvgLineTo(vg, 7.5f, 12.f);
        nvgFill(vg);
    }

    void drawLeftSymbol(NVGcontext * vg) {
        nvgBeginPath(vg);
        nvgFillColor(vg, RampGray(G_85));
        nvgMoveTo(vg, 4.f, 8.5f);
        nvgLineTo(vg, 9.f, 6.f);
        nvgLineTo(vg, 9.f, 11.f);
        nvgFill(vg);
    }

    void drawRightSymbol(NVGcontext * vg) {
        nvgBeginPath(vg);
        nvgFillColor(vg, RampGray(G_85));
        nvgMoveTo(vg, 11.f, 8.5f);
        nvgLineTo(vg, 6.f, 6.f);
        nvgLineTo(vg, 6.f, 11.f);
        nvgFill(vg);
    }

    void drawNilSymbol(NVGcontext * vg) {
        OpenCircle(vg, 7.5f, 8.f, 3.f, RampGray(G_45), 0.5f);
        Line(vg, 7.f, 8.f, 8.f, 8.f, RampGray(G_50), 0.5f);
    }

    void drawSymbol(NVGcontext * vg, SquareButtonSymbol symbol, bool pressed) {
        switch (symbol) {
        case SquareButtonSymbol::Nil:    drawNilSymbol(vg); break;
        case SquareButtonSymbol::Up:     drawUpSymbol(vg); break;
        case SquareButtonSymbol::Down:   drawDownSymbol(vg); break;
        case SquareButtonSymbol::Left:   drawLeftSymbol(vg); break;
        case SquareButtonSymbol::Right:  drawRightSymbol(vg); break;
        case SquareButtonSymbol::Funnel: drawFunnel(vg, pressed); break;
        default:
            Line(vg, 0, 0, 15, 15, GetStockColor(StockColor::Red));
            Line(vg, 15, 0, 0, 15, GetStockColor(StockColor::Red));
            break;
        }
    }

    void drawBase(NVGcontext * vg) {
        FillRect(vg, 0.f, 0.f, 15.f, 15.f, RampGray(G_0));
        FillRect(vg, 0.f, 0.f, 15.f, 0.5f, med_screen);
        //FillRect(vg, 0.f, 14.5f, 15.f, 0.5f, RampGray(G_65));
    }
};

struct SquareButton : ButtonBehavior {
    SquareButtonSymbol symbol = SquareButtonSymbol::Funnel;
    DrawSquareButton image;

    SquareButton() {
        box.size.x = 15.f;
        box.size.y = 16.f;
    }

    void setSymbol(SquareButtonSymbol sym) {
        symbol = sym;
    }

    void draw(const DrawArgs& args) override {
        auto vg = args.vg;
        image.drawBase(vg);
        if (enabled) {
            if (pressed) {
                image.drawDownFace(vg);
            } else {
                image.drawUpFace(vg);
            }
            image.drawSymbol(vg, symbol, pressed);
        } else {
            image.drawUpFace(vg);
            image.drawNilSymbol(vg);
        }
    }
};

}
#endif