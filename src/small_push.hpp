#pragma once
#include "plugin.hpp"
#include "colors.hpp"
#include "components.hpp"

namespace pachde {

struct SmallPush : OpaqueWidget
{
    bool pressed = false;
    bool ctrl = false;
    bool shift = false;
    rack::ui::Tooltip* tip = nullptr;
    std::string text;
    NVGcolor ring = COLOR_BRAND_MD;
    NVGcolor collar1, collar2, bezel, bevel1, bevel2, face1, face2;

    virtual ~SmallPush() {
        if (tip) delete tip;
        tip = nullptr;
    }

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
        text = description;
    }

    void createTip() {
        if (!rack::settings::tooltips) return;
        if (tip) return;
        tip = new Tooltip;
        tip->text = text;
        APP->scene->addChild(tip);
    }

    void destroyTip() {
        if (!tip) return;
        auto t = tip;
        tip = nullptr;
	    APP->scene->removeChild(t);
        delete t;
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
        rack::OpaqueWidget::onHoverKey(e);
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
        rack::OpaqueWidget::onDragStart(e);
    }

    void onDragEnd(const DragEndEvent& e) override
    {
        pressed = false;
        rack::OpaqueWidget::onDragEnd(e);
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