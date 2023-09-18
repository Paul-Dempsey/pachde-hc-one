#pragma once
#ifndef PRESET_WIDGET_HPP_INCLUDED
#define PRESET_WIDGET_HPP_INCLUDED
#include "em_midi.hpp"
#include "presets.hpp"
#include "text.hpp"
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
    SymbolWidget* symbol = nullptr;
    StaticTextLabel* text_label = nullptr;
    StaticTextLabel* text_code = nullptr;

    PresetWidget() {
        box.size.x = 320.f / 3.f;
        box.size.y = 27.f;
        symbol = createWidget<SymbolWidget>(Vec(4.f, box.size.y - 10.f));
        symbol->setSymbol(0);
        addChild(symbol);
        text_label = createStaticTextLabel(Vec(2.5f, 1.5f), box.size.x - 4.f, "", TextAlignment::Left, 12.f, false);
        addChild(text_label);
        text_code = createStaticTextLabel(Vec(box.size.x - 20, box.size.y - 12.f), 16.f, "", TextAlignment::Right, 9.f, false, GetStockColor(StockColor::pachde_blue_light));
        addChild(text_code);
    }

    void setPresetHolder(IPresetHolder* h) {
        holder = h;
    }

    void center(Vec pos) {
        box.pos = pos.minus(box.size.div(2));
    }

    void setPreset(std::shared_ptr<Preset> patch) {
        preset = patch;
        if (preset) {
            tip_text = preset->describe();
            symbol->setSymbol((0 == preset->bank_hi) ? 1 : 0);
            symbol->box.pos.x = preset->favorite ? 12.f : 4.f;
            text_label->text(preset->name);
            text_code->text(preset->categoryName());
        } else {
            tip_text = "(no preset)";
            symbol->setSymbol(0);
            symbol->box.pos.x = 4.f;
            text_label->text("");
            text_code->text("");
        }
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
        text_label->color(RampGray(G_15));
        text_label->dirty();
        text_code->color(GetStockColor(StockColor::pachde_blue_dark));
        text_code->dirty();
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
        text_label->color(RampGray(G_90));
        text_label->dirty();
        text_code->color(GetStockColor(StockColor::pachde_blue_light));
        text_code->dirty();
    }

    void draw(const DrawArgs& args) override;
};

}
#endif