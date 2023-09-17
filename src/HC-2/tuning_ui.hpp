#pragma once
#ifndef TUNING_UI_HPP_INCLUDED
#define TUNING_UI_HPP_INCLUDED
#include "rack.hpp"
#include "HC-2.hpp"
#include "../em_types.hpp"
using namespace ::rack;
namespace pachde {
//using namespace em_midi;

struct TuningParamQuantity : ::rack::engine::ParamQuantity
{
    void setTuning(Tuning tuning) {
        setValue(packTuning(tuning));
    }

    Tuning getTuning() {
        uint8_t packed = getValue();
        return unpackTuning(packed);
    }
	std::string getDisplayValueString() override {
        return describeTuning(getTuning());
    }
};

template <typename TPQ = TuningParamQuantity>
TPQ* configTuningParam(Module* module, int paramId)
{
    assert(paramId >= 0 && static_cast<size_t>(paramId) < module->params.size() && static_cast<size_t>(paramId) < module->paramQuantities.size());
    if (module->paramQuantities[paramId]) {
        delete module->paramQuantities[paramId];
    }

    TPQ* q = new TPQ;
    q->module = module;
    q->paramId = paramId;
    q->minValue = 0.f;
    q->maxValue = static_cast<float>(PackedTuning::ptUserLast);
    q->name = "Tuning";

    module->paramQuantities[paramId] = q;

    Param* p = &module->params[paramId];
    p->value = q->getDefaultValue();

    return q;    
}

struct TuningKnob : SmallBlackKnob
{
    void draw(const DrawArgs& args) override {
        //DrawKnobTrack(args.vg, this, 13.f, 1.2f, RampGray(G_20));
        Circle(args.vg, box.size.x*.5f, box.size.x*.5f, 13.f, GetStockColor(StockColor::Gray_25p)); // bezel
        SmallBlackKnob::draw(args);
    }

    Tuning getParamValue() {
        uint8_t packed = getParamQuantity()->getValue();
        return unpackTuning(packed);
    }
    void setParamValue(Tuning tuning) {
        getParamQuantity()->setValue(packTuning(tuning));
    }

    MenuItem * createTuningMenuItem(Tuning tuningItem)
    {
        return createCheckMenuItem(describeTuning(tuningItem), "", 
            [=](){ return getParamValue() == tuningItem; },
            [=](){ setParamValue(tuningItem); });
    }

    void appendContextMenu(Menu* menu) override
    {
        if (!module) return;
        menu->addChild(new MenuSeparator);
        menu->addChild(createTuningMenuItem(Tuning::EqualTuning));
        menu->addChild(createSubmenuItem("n-Tone Equal", "", [=](Menu * menu) {
            menu->addChild(createTuningMenuItem(Tuning::OneTone));
            menu->addChild(createTuningMenuItem(static_cast<Tuning>(2)));
            menu->addChild(createTuningMenuItem(static_cast<Tuning>(3)));
            menu->addChild(createTuningMenuItem(static_cast<Tuning>(4)));
            menu->addChild(createTuningMenuItem(static_cast<Tuning>(5)));
            menu->addChild(createTuningMenuItem(static_cast<Tuning>(6)));
            menu->addChild(createTuningMenuItem(static_cast<Tuning>(7)));
            menu->addChild(createTuningMenuItem(static_cast<Tuning>(8)));
            menu->addChild(createTuningMenuItem(static_cast<Tuning>(9)));
            menu->addChild(createTuningMenuItem(static_cast<Tuning>(10)));
            menu->addChild(createTuningMenuItem(static_cast<Tuning>(11)));
            menu->addChild(createTuningMenuItem(static_cast<Tuning>(17)));
            menu->addChild(createTuningMenuItem(static_cast<Tuning>(19)));
            menu->addChild(createTuningMenuItem(static_cast<Tuning>(22)));
            menu->addChild(createTuningMenuItem(static_cast<Tuning>(24)));
            menu->addChild(createTuningMenuItem(static_cast<Tuning>(26)));
            menu->addChild(createTuningMenuItem(static_cast<Tuning>(31)));
            menu->addChild(createTuningMenuItem(static_cast<Tuning>(43)));
            menu->addChild(createTuningMenuItem(Tuning::FiftyTone));
        }));
        menu->addChild(createSubmenuItem("Just", "", [=](Menu * menu) {
            menu->addChild(createTuningMenuItem(Tuning::JustC));
            menu->addChild(createTuningMenuItem(Tuning::JustCs));
            menu->addChild(createTuningMenuItem(Tuning::JustD));
            menu->addChild(createTuningMenuItem(Tuning::JustEb));
            menu->addChild(createTuningMenuItem(Tuning::JustF));
            menu->addChild(createTuningMenuItem(Tuning::JustFs));
            menu->addChild(createTuningMenuItem(Tuning::JustG));
            menu->addChild(createTuningMenuItem(Tuning::JustAb));
            menu->addChild(createTuningMenuItem(Tuning::JustA));
            menu->addChild(createTuningMenuItem(Tuning::JustBb));
            menu->addChild(createTuningMenuItem(Tuning::JustB));
        }));
        menu->addChild(createSubmenuItem("User-defined", "", [=](Menu * menu) {
            menu->addChild(createTuningMenuItem(Tuning::UserTuning1));
            menu->addChild(createTuningMenuItem(static_cast<Tuning>(Tuning::UserTuning1 + 1)));
            menu->addChild(createTuningMenuItem(static_cast<Tuning>(Tuning::UserTuning1 + 2)));
            menu->addChild(createTuningMenuItem(static_cast<Tuning>(Tuning::UserTuning1 + 3)));
            menu->addChild(createTuningMenuItem(static_cast<Tuning>(Tuning::UserTuning1 + 4)));
            menu->addChild(createTuningMenuItem(static_cast<Tuning>(Tuning::UserTuning1 + 5)));
            menu->addChild(createTuningMenuItem(static_cast<Tuning>(Tuning::UserTuning1 + 6)));
            menu->addChild(createTuningMenuItem(static_cast<Tuning>(Tuning::UserTuning1 + 7)));
        }));
    }
};

}
#endif