#pragma once
#ifndef EM_PICKER_HPP_INCLUDED
#define EM_PICKER_HPP_INCLUDED
#include <rack.hpp>
#include "misc.hpp"
#include "plugin.hpp"
#include "tip_widget.hpp"
using namespace ::rack;
namespace pachde {

struct ISetDevice {
    virtual void setMidiDevice(int id) = 0;
};

struct EMPicker : TipWidget
{
    midi::Port* port;
	widget::FramebufferWidget* fb;
	widget::SvgWidget* sw;
    ISetDevice* setter;

    EMPicker() {
        fb = new widget::FramebufferWidget;
        addChild(fb);
        sw = new widget::SvgWidget;
        fb->addChild(sw);
        sw->setSvg(Svg::load(asset::plugin(pluginInstance, "res/MIDI.svg")));
        box.size = sw->box.size;
        fb->box.size = sw->box.size;
        fb->setDirty(true);
    }

    void setExternals(midi::Port* the_port, ISetDevice * callback) {
        assert(the_port);
        assert(callback);
        port = the_port;
        setter = callback;
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
        if (!port || ! setter) return;
        assert(port);
        assert(setter);
        menu->addChild(createMenuLabel("Eagan Matrix device"));
        menu->addChild(new MenuSeparator);
        menu->addChild(createMenuItem("Reset (auto)", "", [=](){ setter->setMidiDevice(-1); }));
        for (auto id : port->getDeviceIds()) {
            std::string name = FilterDeviceName(port->getDeviceName(id));
            if (is_EMDevice(name)) {
                menu->addChild(createCheckMenuItem(name, "",
                [=](){ return port->getDeviceId() == id; },
                [=](){ setter->setMidiDevice(id); }));
            }
        }
    }
};

}
#endif