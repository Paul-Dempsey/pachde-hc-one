#pragma once
#ifndef CC_PARAM_HPP_INCLUDED
#define CC_PARAM_HPP_INCLUDED
#include "HC-1.hpp"
namespace pachde {

using namespace em_midi;
struct CCParamQuantity : rack::engine::ParamQuantity
{
    uint8_t cc = 0; // never a valid cc for param
    uint16_t last_value = 0;
    bool high_resolution = true;

    uint16_t convertValue(float value)  {
        return high_resolution
            ? static_cast<uint16_t>(clamp(value, 0.f, EM_Max14f))
            : static_cast<uint16_t>(clamp(value, 0.f, 127.f));
    }

    uint16_t valueToSend() {
        auto p = getParam();
        if (!p) return 0;
        return convertValue(p->getValue());
    }

    void syncValue() {
        assert(cc);
        auto to_send = valueToSend();
        if (last_value != to_send) {
            last_value = to_send;
            auto my_module = dynamic_cast<Hc1Module*>(module);
            if (my_module) {
                if (high_resolution) {
                    uint8_t lo = to_send & 0x7f;
                    if (lo) {
                        my_module->sendControlChange(EM_SettingsChannel, EMCC_PedalFraction, lo);
                    }
                    uint8_t hi = to_send >> 7;
                    my_module->sendControlChange(EM_SettingsChannel, cc, hi);
                } else {
                    my_module->sendControlChange(EM_SettingsChannel, cc, to_send & 0x7f);
                }
            }
        }
    }

    void updateLastValue() {
        last_value = valueToSend();
    }

    void setValueSilent(float newValue) {
        auto p = getParam();
        if (!p) return;
        p->setValue(newValue);
        updateLastValue();
    }
};

template <class TCCPQ = CCParamQuantity>
TCCPQ* configCCParam(uint8_t cc, bool hiRes, Module* module, int paramId, float minValue, float maxValue, float defaultValue, std::string name = "", std::string unit = "", float displayBase = 0.f, float displayMultiplier = 1.f, float displayOffset = 0.f)
{
    assert(paramId < (int) module->params.size() && paramId < (int) module->paramQuantities.size());
    if (module->paramQuantities[paramId])
        delete module->paramQuantities[paramId];

    TCCPQ* q = new TCCPQ;
    q->module = module;
    q->paramId = paramId;
    q->minValue = minValue;
    q->maxValue = maxValue;
    q->defaultValue = defaultValue;
    q->name = name;
    q->unit = unit;
    q->displayBase = displayBase;
    q->displayMultiplier = displayMultiplier;
    q->displayOffset = displayOffset;
    q->cc = cc;
    q->high_resolution = hiRes;
    module->paramQuantities[paramId] = q;

    Param* p = &module->params[paramId];
    p->value = q->getDefaultValue();

    return q;
}

}
#endif