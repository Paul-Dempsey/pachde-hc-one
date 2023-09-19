#pragma once
#ifndef TIP_WIDGET_HPP_INCLUDED
#define TIP_WIDGET_HPP_INCLUDED
#include <rack.hpp>
using namespace ::rack;
namespace pachde {

struct TipWidget : OpaqueWidget {
    rack::ui::Tooltip* tip = nullptr;
    std::string tip_text;

    virtual ~TipWidget() {
        if (tip) delete tip;
        tip = nullptr;
    }
    TipWidget() {}

    void createTip() {
        if (!rack::settings::tooltips) return;
        if (tip_text.empty()) return;
        if (tip) return;
        tip = new Tooltip;
        tip->text = tip_text;
        APP->scene->addChild(tip);
    }

    void destroyTip() {
        if (!tip) return;
        auto t = tip;
        tip = nullptr;
	    APP->scene->removeChild(t);
        delete t;
    }

    void onEnter(const EnterEvent& e) override {
        createTip();
    }

    void onLeave(const LeaveEvent& e) override {
        destroyTip();
    }

    void onDragLeave(const DragLeaveEvent& e) override {
        rack::OpaqueWidget::onDragLeave(e);
        destroyTip();
    }

    void onDragEnd(const DragEndEvent& e) override
    {
        rack::OpaqueWidget::onDragEnd(e);
        destroyTip();
    }

    void onButton(const ButtonEvent& e) override
    {
        OpaqueWidget::onButton(e);
        if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT && (e.mods & RACK_MOD_MASK) == 0) {
            destroyTip();
            createContextMenu();
            e.consume(this);
        }
    }

    virtual void appendContextMenu(ui::Menu* menu) {}

    void createContextMenu() {
        ui::Menu* menu = createMenu();
    	appendContextMenu(menu);
    }

};

}
#endif