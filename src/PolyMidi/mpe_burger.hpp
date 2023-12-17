#pragma once
#ifndef MPE_BURGERM_HPP_INCLUDED
#define MPE_BURGER_HPP_INCLUDED
#include <rack.hpp>
#include "../em_types/em_mpe.hpp"
#include "../text.hpp"
#include "../widgets/hamburger.hpp"
#include "../widgets/label_widget.hpp"
using namespace ::rack;
using namespace ::eagan_matrix;

namespace pachde {

struct MpeBurger : Hamburger
{
    StaticTextLabel* tw;
    MidiMode mode;
    ParamQuantity* pq;

    MpeBurger() : tw(nullptr), pq(nullptr)
    {
        setMode(MidiMode::MpePlus);
    }

    void setLabel(StaticTextLabel* label) {
        tw = label;
    }
    void setParam(ParamQuantity* param) {
        pq = param;
    }

    void setMode(MidiMode new_mode) {
        mode = new_mode; 
        switch (mode) {
        default:
        case MidiMode::Midi:
            describe("Midi");
            if (tw) tw->text("Midi");
            if (pq) pq->setValue(U8(MidiMode::Midi));
            break;
        case MidiMode::Mpe:
            describe("MPE");
            if (tw) tw->text("MPE");
            if (pq) pq->setValue(U8(MidiMode::Mpe));
            break;
        case MidiMode::MpePlus: 
            describe("MPE+");
            if (tw) tw->text("MPE+");
            if (pq) pq->setValue(U8(MidiMode::MpePlus));
            break;
        }
    }

    void step() override {
        if (!pq) return;
        mode = static_cast<MidiMode>(GetByteParamValue(pq));
    }

    void appendContextMenu(Menu * menu) override
    {
        menu->addChild(new MenuSeparator());
        menu->addChild(createCheckMenuItem("MIDI", "", [=](){ return MidiMode::Midi == mode; }, [=](){ setMode(MidiMode::Midi); }));
        menu->addChild(createCheckMenuItem("MPE", "", [=](){ return MidiMode::Mpe == mode; }, [=](){ setMode(MidiMode::Mpe); }));
        menu->addChild(createCheckMenuItem("MPE+", "", [=](){ return MidiMode::MpePlus == mode; }, [=](){ setMode(MidiMode::MpePlus); }));
    }
};

}
#endif