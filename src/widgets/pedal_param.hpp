#pragma once
#ifndef PEDAL_PARAM_HPP_INCLUDED
#define PEDAL_PARAM_HPP_INCLUDED
#include <rack.hpp>
#include "components.hpp"
#include "../em_types.hpp"

namespace pachde {
using namespace em_midi;

struct PedalParamQuantity : rack::engine::ParamQuantity
{
    uint8_t pedal_id = 0; // 0,1 = pedal1,pedal2
    bool enabled = false;
    uint8_t last_cc;

    void setEnabled(bool on)
    {
        enabled = on;
    }

    static float voltageFromPedalAssign(PedalAssign assign)
    {
        return static_cast<float>(static_cast<uint8_t>(assign));
    }

    static PedalAssign voltageToPedalAssign(float v)
    {
        uint8_t raw = static_cast<uint8_t>(std::round(v));
        raw = clamp(raw, static_cast<uint8_t>(PedalAssign::First), static_cast<uint8_t>(PedalAssign::Last));
        return static_cast<PedalAssign>(raw);
    }

    void setParamValue(PedalAssign assign)
    {
        ParamQuantity::setImmediateValue(voltageFromPedalAssign(assign));
    }

    PedalAssign getParamValue()
    {
        return voltageToPedalAssign(ParamQuantity::getImmediateValue());
    }

    uint8_t paramCC()
    {
        return PedalCC(getParamValue());
    }

    void sendValue(uint8_t explicit_value = 0)
    {
        auto to_send = explicit_value > 0 ? explicit_value : paramCC();
        last_cc = to_send;
        if (!enabled) return;
        auto iSend = dynamic_cast<ISendMidi*>(module);
        if (!(iSend && iSend->readyToSend())) return;
        iSend->sendControlChange(EM_SettingsChannel, pedal_id ? EMCC_Pedal2CC : EMCC_Pedal1CC, to_send);
    }

    bool syncValue()
    {
        auto to_send = paramCC();
        if (last_cc != to_send) {
            sendValue(to_send);
            return true;
        }
        return false;
    }

    void updateLastValue() {
        last_cc = paramCC();
    }

    void setAssignSilent(PedalAssign newValue)
    {
        ParamQuantity::setImmediateValue(voltageFromPedalAssign(newValue));
        updateLastValue();
    }

    void setVoltageSilent(float newValue)
    {
        ParamQuantity::setImmediateValue(newValue);
        updateLastValue();
    }

    std::string getDisplayValueString() override {
        return LongPedalAssignment(paramCC());
    }
};

template <class PPQ = PedalParamQuantity>
PPQ * configPedalParam(uint8_t pedal, PedalAssign role, Module* module, int paramId)
{
    PPQ* q = new PPQ;
    q->module = module;
    q->paramId = paramId;
    q->pedal_id = pedal;
    q->snapEnabled = true;
    q->smoothEnabled = false;
    q->minValue = PPQ::voltageFromPedalAssign(PedalAssign::First);
    q->maxValue = PPQ::voltageFromPedalAssign(PedalAssign::Last);
    q->defaultValue = PPQ::voltageFromPedalAssign(pedal ? PedalAssign::Sostenuto : PedalAssign::Sustain);
    q->name = pedal ? "Pedal 2" : "Pedal 1";
    module->paramQuantities[paramId] = q;

    Param * p = &module->params[paramId];
    p->value = q->getDefaultValue();

    return q;
}

struct PedalKnob : SmallBlackKnob
{
    void draw(const DrawArgs& args) override {
        Circle(args.vg, box.size.x*.5f, box.size.x*.5f, 13.f, GetStockColor(StockColor::Gray_25p)); // bezel
        SmallBlackKnob::draw(args);
    }

    PedalAssign getParamValue()
    {
        return static_cast<PedalParamQuantity*>(getParamQuantity())->getParamValue();
    }
    void setParamValue(PedalAssign assign)
    {
        static_cast<PedalParamQuantity*>(getParamQuantity())->setParamValue(assign);
    }

    MenuItem* createPedalMenuItem(PedalAssign assign)
    {
        return createCheckMenuItem(LongPedalAssignment(PedalCC(assign)), "",
            [=]() { return getParamValue() == assign; },
            [=]() { setParamValue(assign); }
        );
    }

    void appendContextMenu(Menu * menu) override
    {
        if (!module) return;
        menu->addChild(new MenuSeparator);
        menu->addChild(createSubmenuItem("Switches", "",  [=](Menu * menu) {
            menu->addChild(createPedalMenuItem(PedalAssign::OctaveShift));
            menu->addChild(createPedalMenuItem(PedalAssign::MonoSwitch));
            menu->addChild(createPedalMenuItem(PedalAssign::PresetAdvance));
            }));
        menu->addChild(createSubmenuItem("Levels", "",  [=](Menu * menu) {
            menu->addChild(createPedalMenuItem(PedalAssign::PreLevel));
            menu->addChild(createPedalMenuItem(PedalAssign::PostLevel));
            menu->addChild(createPedalMenuItem(PedalAssign::AudioInputLevel));
            //menu->addChild(createPedalMenuItem(PedalAssign::Attenuation)); // "works" but doesn't behave
            }));

        menu->addChild(createSubmenuItem("Sus/Sos", "",  [=](Menu * menu) {        
            menu->addChild(createPedalMenuItem(PedalAssign::Sustain));
            menu->addChild(createPedalMenuItem(PedalAssign::Sostenuto));
            menu->addChild(createPedalMenuItem(PedalAssign::Sostenuto2));
            }));
        menu->addChild(createSubmenuItem("Macros", "",  [=](Menu * menu) {        
            menu->addChild(createPedalMenuItem(PedalAssign::Macro_i));
            menu->addChild(createPedalMenuItem(PedalAssign::Macro_ii));
            menu->addChild(createPedalMenuItem(PedalAssign::Macro_iii));
            menu->addChild(createPedalMenuItem(PedalAssign::Macro_iv));
            menu->addChild(createPedalMenuItem(PedalAssign::Macro_v));
            menu->addChild(createPedalMenuItem(PedalAssign::Macro_vi));
            }));
        menu->addChild(createSubmenuItem("Recirculator", "",  [=](Menu * menu) {
            menu->addChild(createPedalMenuItem(PedalAssign::R1));
            menu->addChild(createPedalMenuItem(PedalAssign::R2));
            menu->addChild(createPedalMenuItem(PedalAssign::R3));
            menu->addChild(createPedalMenuItem(PedalAssign::R4));
            menu->addChild(createPedalMenuItem(PedalAssign::RecirculatorMix));
            }));
        menu->addChild(createSubmenuItem("Rounding", "",  [=](Menu * menu) {
            menu->addChild(createPedalMenuItem(PedalAssign::RoundRate));
            menu->addChild(createPedalMenuItem(PedalAssign::RoundInitial));
            menu->addChild(createPedalMenuItem(PedalAssign::RoundEqual));
            }));
        menu->addChild(createPedalMenuItem(PedalAssign::FineTune));

        menu->addChild(createSubmenuItem("Extensions", "",  [=](Menu * menu) {
            menu->addChild(createPedalMenuItem(PedalAssign::Expression));
            menu->addChild(createPedalMenuItem(PedalAssign::Volume));
            menu->addChild(createMenuLabel("Extensions send Pedal output on channel 16"));
            menu->addChild(createMenuLabel("but do not affect EM sound generation"));
            }));
    }
};

}
#endif