// Copyright (C) Paul Chase Dempsey
#pragma once
#ifndef FAVORITE_WIDGET_HPP_INCLUDED
#define FAVORITE_WIDGET_HPP_INCLUDED
#include "../colors.hpp"
#include "../presets.hpp"
#include "components.hpp"
#include "tip_widget.hpp"

namespace pachde {

struct FavoriteWidget : TipWidget
{
    bool pressed = false;
    std::shared_ptr<Preset> preset = nullptr;
    IPresetHolder* holder = nullptr;

    FavoriteWidget()
    {
        box.size.x = 12.f;
        box.size.y = 12.f;
    }

    void setPresetHolder(IPresetHolder* h) {
        holder = h;
    }

    void setPreset(std::shared_ptr<Preset> patch) {
        if (patch == preset) { return; }
        preset = patch;
        if (!preset) {
            describe("(no preset)");
        } else {
            describe(format_string("%s %s", preset->favorite ? "un-favorite" : "favorite", preset->name.c_str()));
        }
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

        if (preset) {
            preset->favorite = !preset->favorite;
            if (holder) {
                if (preset->favorite) {
                    holder->addFavorite(preset);
                } else {
                    holder->unFavorite(preset);
                }
            }
        }
        pressed = false;
    }

    void draw(const DrawArgs& args) override
    {
        TipWidget::draw(args);
        auto vg = args.vg;
        if (preset && preset->favorite && !pressed) {
            FillHeart(vg, 0, 0, box.size.x, PORT_PINK);
        } else {
            StrokeHeart(vg, 0, 0, box.size.x, PORT_PINK, .75f);
        }
    }
};

}
#endif