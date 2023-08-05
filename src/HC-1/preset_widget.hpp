#pragma once
#ifndef PRESET_WIDGET_HPP_INCLUDED
#define PRESET_WIDGET_HPP_INCLUDED
#include "HC-1.hpp"
#include "../colors.hpp"
#include "../presets.hpp"
#include "../text.hpp"

namespace pachde {

struct PresetWidget : OpaqueWidget
{
    bool pressed = false;
    std::shared_ptr<MinPreset> preset;
    rack::ui::Tooltip* tip = nullptr;
    Hc1Module* module = nullptr;

    virtual ~PresetWidget() {
        if (tip) delete tip;
        tip = nullptr;
    }

    PresetWidget() {
        box.size.x = 40.f;
        box.size.y = 27.f;
    }

    void setModule(Hc1Module* m) {
        module = m;
    }

    void center(Vec pos) {
        box.pos = pos.minus(box.size.div(2));
    }

    void createTip() {
        if (!rack::settings::tooltips) return;
        if (tip) return;
        tip = new Tooltip;
        tip->text = preset ? preset->describe() : "(no preset)";
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

    void setPreset(std::shared_ptr<MinPreset> patch) {
        preset = patch;
    }

    void onDragStart(const DragStartEvent& e) override {
        pressed = true;
        rack::OpaqueWidget::onDragStart(e);
    }

    void onDragLeave(const DragLeaveEvent& e) override {
        destroyTip();
        rack::OpaqueWidget::onDragLeave(e);
    }

    void onDragEnd(const DragEndEvent& e) override
    {
        rack::OpaqueWidget::onDragEnd(e);
        if (e.button != GLFW_MOUSE_BUTTON_LEFT)
            return;

        destroyTip();
        if (pressed && module && preset) {
            module->sendCC(EM_MasterChannel, MidiCC_BankSelect, preset->bank_hi);
            module->sendCC(EM_MasterChannel, EMCC_Category, preset->bank_lo);
            module->sendProgramChange(EM_MasterChannel, preset->number);
        }
        pressed = false;
    }

    void draw(const DrawArgs& args) override;
};

}
#endif