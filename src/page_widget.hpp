#pragma once
#ifndef PAGE_WIDGET_HPP_INCLUDED
#define PAGE_WIDGET_HPP_INCLUDED
#include "plugin.hpp"
#include "colors.hpp"
#include "tip_widget.hpp"

namespace pachde {

struct UpDown : TipWidget {
    bool pressed = false;
    bool enabled = true;
    bool up = true;
    std::function<void()> handler;

    UpDown() {
        box.size.x = box.size.y = 7.5f;
    }

    void setUp(bool is_up) {
        up = is_up;
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
        pressed = true;
        TipWidget::onDragStart(e);
    }

    void onDragEnd(const DragEndEvent& e) override
    {
        TipWidget::onDragEnd(e);
        pressed = false;
        if (e.button != GLFW_MOUSE_BUTTON_LEFT)
            return;

        if (handler) {
            handler();
        }
    }

    void draw(const DrawArgs& args) override {

    }
}

}
#endif