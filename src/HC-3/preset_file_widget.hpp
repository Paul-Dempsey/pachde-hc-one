#pragma once
#ifndef PRESET_FILE_WIDGET_HPP_INCLUDED
#define PRESET_FILE_WIDGET_HPP_INCLUDED
#include <rack.hpp>
#include "../tip_widget.hpp"
#include "../open_file.hpp"
#include "../colors.hpp"
#include "HC-3.hpp"

using namespace ::rack;
namespace pachde {

struct PresetFileWidget : TipWidget
{
    Hc3Module * my_module;
    int id;
    DrawSquareButton * drawButton;
    bool pressed = false;

    PresetFileWidget() 
    : my_module(nullptr), id(-1), drawButton(nullptr)
    {
        box.size.x = 75.f;
        box.size.y = 15;
    }
    void setDrawButton(DrawSquareButton * draw) {
        drawButton = draw;
    }
    void setModule(Hc3Module * module) { my_module = module; }
    void setId(int the_id) {
        assert(id == -1);
        assert(the_id >= 0);
        id = the_id;
    }
    int getId() { return id; }
    bool haveFile() { 
        if (my_module) { return !my_module->files[id].empty(); }
        return !hc3_sample_data[id].empty();
    }
    bool isCurrent() { 
        if (my_module) { return my_module->loaded_id == id; }
        return id == CHOSEN_SAMPLE;
    }
    std::string getLabel() {
        return my_module ? system::getStem(my_module->files[id]) : hc3_sample_data[id];
    }

    void onDragStart(const DragStartEvent& e) override {
        if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
            pressed = true;
        }
        TipWidget::onDragStart(e);
    }

    void onDragEnd(const DragEndEvent& e) override
    {
        TipWidget::onDragEnd(e);
        pressed = false;
    }

    void onButton(const ButtonEvent& e) override {
        TipWidget::onButton(e);
        if (!my_module) return;
        if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT && (e.mods & RACK_MOD_MASK) == 0) {
            if (isCurrent()) {
                my_module->openFile(-1);
            } else if (haveFile()) {
                my_module->openFile(id);
            }
        }
    }

    void drawLayer(const DrawArgs& args, int layer) override {
        TipWidget::drawLayer(args, layer);
        if (layer != 1) return;
        if (isCurrent() && haveFile()) {
            auto vg = args.vg;
            auto font = GetPluginFontSemiBold();
            SetTextStyle(vg, font, preset_name_color, 9.f);
            nvgTextAlign(vg, NVG_ALIGN_LEFT);
            nvgText(vg, 18.f, 10.f, getLabel().c_str(), nullptr);
        }
    }

    void draw(const DrawArgs& args) override {
        TipWidget::draw(args);
        auto vg = args.vg;

        drawButton->drawBase(vg);
        if (pressed) {
            drawButton->drawDownFace(vg);
        } else {
            drawButton->drawUpFace(vg);
        }
        if (haveFile())
        {
            if (isCurrent()) {
                FillRect(vg, 16.5f, 0.f, box.size.x, box.size.y, GetStockColor(StockColor::pachde_blue_dark));
            } else {
                auto font = GetPluginFontRegular();
                SetTextStyle(vg, font, RampGray(G_85), 9.f);
                nvgTextAlign(vg, NVG_ALIGN_LEFT);
                nvgText(vg, 18.f, 10.f, getLabel().c_str(), nullptr);
            }
        }
    }

    void step() override {
        TipWidget::step();
        if (!my_module) return;
        if (!TipWidget::hasText() && haveFile()) {
            describe(format_string("Open %s", my_module->files[id].c_str()));
        }
    }

    void appendContextMenu(ui::Menu* menu) override {
        if (!my_module) return;
        assert(id >= 0);

        std::string friendly_file = my_module->files[id].empty() ? "(none)" : system::getStem(my_module->files[id]);
        menu->addChild(createMenuLabel(friendly_file));
        menu->addChild(new MenuSeparator);

        menu->addChild(createMenuItem("Favorite file...", "", [=](){
            std::string path;
            std::string folder = asset::user(pluginInstance->slug);
            system::createDirectories(folder);
            if (openFileDialog(
                folder,
                "Favorites (.fav):fav;Json (.json):json;Any (*):*))",
                "",
                path)) {
                my_module->files[id] = path;
                describe(format_string("Open %s", path.c_str()));
            }
        }));

        menu->addChild(createMenuItem(haveFile() ? format_string("Forget %s", friendly_file.c_str()) : "Forget", "", [=](){
            my_module->files[id] = "";
            my_module->loaded_id = -1;
            describe("");
        }, !haveFile()));

        auto partner = my_module->getPartner();
        if (partner && !partner->favoritesFile.empty()) {
            auto partnerFile = partner->favoritesFile;
            if (partnerFile != my_module->files[id]) {
                auto stem =  system::getStem(partnerFile);

                menu->addChild(createMenuItem(format_string("Use %s", stem.c_str()), "", [&](){
                    my_module->useCurrentFavoriteFile(id);
                    describe(format_string("Open %s", my_module->files[id].c_str()));
                    }));
            }
        }
    }
};

template <typename TW = PresetFileWidget>
TW* createPFWidget(Vec pos, Hc3Module* module, int id, DrawSquareButton* draw) {
    auto w = new TW;
    w->box.pos = pos;
    w->setModule(module);
    w->setId(id);
    w->setDrawButton(draw);
    return w;
}

}
#endif