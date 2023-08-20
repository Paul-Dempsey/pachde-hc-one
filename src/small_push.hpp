#pragma once
#include "plugin.hpp"
#include "colors.hpp"
#include "components.hpp"
#include "tip_widget.hpp"

namespace pachde {

struct SmallPush : TipWidget
{
    bool pressed = false;
    bool ctrl = false;
    bool shift = false;

    NVGcolor bezel, bevel1, bevel2, face, face1, face2;

    SmallPush()
    :   bezel(RampGray(G_BLACK)), 
        bevel1(RampGray(G_50)),
        bevel2(nvgRGB(0x0b, 0x0b, 0x0b)),
        face(nvgRGB(0x2e,0x2e,0x2e)),
        face1(nvgTransRGBAf(RampGray(G_50), .29f)),
        face2(nvgTransRGBAf(nvgRGB(0x0b, 0x0b, 0x0b), .29f))
    {
        box.size.x = box.size.y = 12.f;
    }

    void describe(std::string description) {
        tip_text = description;
    }

    void center(Vec pos) {
        box.pos = pos.minus(box.size.div(2));
    }

    std::function<void(bool, bool)> clickHandler;
    void onClick(std::function<void(bool, bool)> callback)
    {
        clickHandler = callback;
    }

    void onHoverKey(const HoverKeyEvent& e) override {
        TipWidget::onHoverKey(e);
        ctrl = (e.mods & RACK_MOD_MASK) & RACK_MOD_CTRL;
        shift = (e.mods & RACK_MOD_MASK) & GLFW_MOD_SHIFT;
    }

    void onEnter(const EnterEvent& e) override {
        createTip();
    }

    void onLeave(const LeaveEvent& e) override {
        destroyTip();
    }

    void onDragStart(const DragStartEvent& e) override {
        pressed = true;
        TipWidget::onDragStart(e);
    }

    void onDragEnd(const DragEndEvent& e) override
    {
        pressed = false;
        TipWidget::onDragEnd(e);
        if (e.button != GLFW_MOUSE_BUTTON_LEFT)
            return;

        if (clickHandler) {
            destroyTip();
            clickHandler(ctrl, shift);
        }
    }

    void draw(const DrawArgs& args) override;
};

}