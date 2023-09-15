#pragma once
#ifndef PRESET_WIDGET_HPP_INCLUDED
#define PRESET_WIDGET_HPP_INCLUDED
#include "em_midi.hpp"
#include "presets.hpp"
#include "tip_widget.hpp"
#include "symbol_widget.hpp"

namespace pachde {
using namespace em_midi;

struct PresetWidget : TipWidget
{
    bool pressed = false;
    bool hovered = false;
    std::shared_ptr<Preset> preset;
    IPresetHolder* holder = nullptr;
    SymbolWidget * symbol = nullptr;

    PresetWidget() {
        box.size.x = 320.f / 3.f;
        box.size.y = 27.f;
        symbol = createWidget<SymbolWidget>(Vec(4.f, box.size.y - 10.f));
        symbol->setSymbol(0);
        addChild(symbol);
    }

    void setPresetHolder(IPresetHolder* h) {
        holder = h;
    }

    void center(Vec pos) {
        box.pos = pos.minus(box.size.div(2));
    }

    void setPreset(std::shared_ptr<Preset> patch) {
        preset = patch;
        tip_text = preset ? preset->describe() : "(no preset)";
        symbol->setSymbol(preset && (0 == preset->bank_hi) ? 1 : 0);
        symbol->box.pos.x = preset && preset->favorite ? 12.f : 4.f;
    }

    void onHover(const HoverEvent& e) override {
        TipWidget::onHover(e);
        auto inset = box.shrink(Vec(2.f, 1.5f));
        hovered = inset.contains(e.pos.plus(box.pos));
        if (hovered) {
            e.consume(this);
        }
    }

    void onLeave(const LeaveEvent& e) override {
        TipWidget::onLeave(e);
        hovered = false;
    }

    void onDragStart(const DragStartEvent& e) override {
        pressed = true;
        TipWidget::onDragStart(e);
    }

    void onDragLeave(const DragLeaveEvent& e) override {
        TipWidget::onDragLeave(e);
    }

    void onDragEnd(const DragEndEvent& e) override
    {
        TipWidget::onDragEnd(e);
        if (e.button != GLFW_MOUSE_BUTTON_LEFT)
            return;

        if (holder && preset) {
            holder->setPreset(preset);
        }
        pressed = false;
    }

    void draw(const DrawArgs& args) override;
};

}
#endif