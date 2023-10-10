#pragma once
#ifndef PRESET_WIDGET_HPP_INCLUDED
#define PRESET_WIDGET_HPP_INCLUDED
#include "../em_midi.hpp"
#include "../presets.hpp"
//#include "../text.hpp"
#include "tip_widget.hpp"
#include "label_widget.hpp"
#include "symbol_widget.hpp"

namespace pachde {
using namespace em_midi;

using Symbol = SymbolWidget::Symbol;

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
        symbol->setSymbol(Symbol::Blank);
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
            describe(preset->describe());
            symbol->setSymbol((0 == preset->bank_hi) ? Symbol::Person : Symbol::Blank);
            symbol->box.pos.x = preset->favorite ? 12.f : 4.f;
            text_label->text(preset->name);
            text_code->text(preset->categoryName());
        } else {
            describe("(no preset)");
            symbol->setSymbol(Symbol::Blank);
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

    void setPressed(bool is_pressed) {
        pressed = is_pressed;
        if (pressed) {
            text_label->color(RampGray(G_15));
            text_code->color(GetStockColor(StockColor::pachde_blue_dark));
        } else {
            text_label->color(RampGray(G_90));
            text_code->color(GetStockColor(StockColor::pachde_blue_light));
        }
        text_label->dirty();
        text_code->dirty();
    }

    void onDragStart(const DragStartEvent& e) override {
        TipWidget::onDragStart(e);
    }

    void onDragLeave(const DragLeaveEvent& e) override {
        setPressed(false);
        TipWidget::onDragLeave(e);
    }

    void onDragEnd(const DragEndEvent& e) override
    {
        setPressed(false);
        TipWidget::onDragEnd(e);
        if (e.button != GLFW_MOUSE_BUTTON_LEFT)
            return;

       // e.consume(this);
        if (holder && preset) {
            holder->setPreset(preset);
        }
    }
    void onButton(const ButtonEvent& e) override
    {
        bool was_pressed = pressed;
        setPressed(false);
        TipWidget::onButton(e);
        if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT && (e.mods & RACK_MOD_MASK) == 0)
        {
            return;
        }
        if (was_pressed && holder && preset) {
            holder->setPreset(preset);
        }
        //e.consume(this);
    }

    void draw(const DrawArgs& args) override;
    void appendContextMenu(ui::Menu* menu) override;
};

}
#endif