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
    NVGcolor ring = COLOR_BRAND_MD;
    NVGcolor collar1, collar2, bezel, bevel1, bevel2, face1, face2;

    SmallPush()
    { 
        box.size.x = box.size.y = 20.f;
        collar1 = RampGray(G_80);
        collar2 = RampGray(G_60);
        bezel   = RampGray(G_20);
        bevel1  = RampGray(G_75);
        bevel2  = RampGray(G_25);
        face1   = RampGray(G_80); 
        face2   = RampGray(G_10);
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