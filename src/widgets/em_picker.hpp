#pragma once
#ifndef EM_PICKER_HPP_INCLUDED
#define EM_PICKER_HPP_INCLUDED
#include <rack.hpp>
#include "../em_device.hpp"
#include "../plugin.hpp"
#include "tip_widget.hpp"

using namespace ::rack;
namespace pachde {

struct ISetDevice {
    virtual void setMidiDevice(const std::string& claim) = 0;
};

struct EMPicker : TipWidget
{
    widget::FramebufferWidget* fb;
    widget::SvgWidget* sw;
    ISetDevice* setter;
    std::shared_ptr<MidiDeviceConnection> connection;
    EMPicker & operator=(const EMPicker &) = delete;
    EMPicker(const EMPicker&) = delete;

    EMPicker() : setter(nullptr)
    {
        fb = new widget::FramebufferWidget;
        addChild(fb);
        sw = new widget::SvgWidget;
        fb->addChild(sw);
        sw->setSvg(Svg::load(asset::plugin(pluginInstance, "res/MIDI.svg")));
        box.size = sw->box.size;
        fb->box.size = sw->box.size;
        fb->setDirty(true);
    }

    void setCallback(ISetDevice * callback) {
        assert(callback);
        setter = callback;
    }
    void setConnection(std::shared_ptr<MidiDeviceConnection> conn) {
        connection = conn;
    }

    void onButton(const ButtonEvent& e) override
    {
        TipWidget::onButton(e);
        if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT && (e.mods & RACK_MOD_MASK) == 0) {
            createContextMenu();
            e.consume(this);
        }
    }

    void appendContextMenu(Menu* menu) override
    {
        if (!setter) return;

        menu->addChild(createMenuLabel("Eagan Matrix device"));
        menu->addChild(new MenuSeparator);
        menu->addChild(createMenuItem("Reset (auto)", "", [=](){ setter->setMidiDevice(""); }));
        auto broker = MidiDeviceBroker::get();
        //broker->sync();

        auto current_claim = connection ? connection->info.spec() : "";

        broker->scan_while(
            [=](const std::shared_ptr<MidiDeviceConnection> conn) {
                if (is_EMDevice(conn->info.input_device_name)) {
                    auto item_claim = conn->info.spec();
                    bool mine = (0 == current_claim.compare(item_claim));
                    bool unavailable = mine ? false : !broker->is_available(item_claim);

                    menu->addChild(createCheckMenuItem(conn->info.friendly(true), "",
                        [=](){ return mine; },
                        [=](){ setter->setMidiDevice(item_claim); }, unavailable));
                }
                return true;
            }
        );
        menu->addChild(new MenuSeparator);
        menu->addChild(createSubmenuItem("Any MIDI device (advanced)", "", [=](Menu * menu){
            broker->scan_while(
                [=](const std::shared_ptr<MidiDeviceConnection> conn) {
                    auto item_claim = conn->info.spec();
                    bool mine = (0 == current_claim.compare(item_claim));
                    bool unavailable = mine ? false : !broker->is_available(item_claim);

                    menu->addChild(createCheckMenuItem(conn->info.friendly(true), "",
                        [=](){ return mine; },
                        [=](){ setter->setMidiDevice(item_claim); }, unavailable));
                    return true;
                }
            );
       }));
    }
};

}
#endif