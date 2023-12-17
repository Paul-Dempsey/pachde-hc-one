#pragma once
#ifndef HAMBURGER_HPP_INCLUDED
#define HAMBURGER_HPP_INCLUDED
#include <rack.hpp>
#include "../colors.hpp"
#include "tip_widget.hpp"
using namespace ::rack;
namespace pachde {

struct Hamburger : TipWidget
{
    uint8_t patties;

    Hamburger() : patties(3)
    {
        box.size.x = 10.f;
        box.size.y = 10.f;
    }

    void onButton(const ButtonEvent& e) override
    {
        TipWidget::onButton(e);
        if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT && (e.mods & RACK_MOD_MASK) == 0) {
            createContextMenu();
            e.consume(this);
        }
    }

    void draw(const DrawArgs& args) override
    {
        TipWidget::draw(args);

        auto vg = args.vg;
        float y = 1.5f;
        const float step = 2.5f;
        const float line_width = 1.5f;
        auto color = RampGray(G_90);
        for (auto n = 0; n < patties; ++n) {
            Line(vg, 1.5f, y, box.size.x - 1.5f, y, color, line_width); y += step;
        }
    }
};

}
#endif
