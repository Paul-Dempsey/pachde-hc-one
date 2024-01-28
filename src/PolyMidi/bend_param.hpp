#pragma once
#ifndef BEND_PARAM_HPP_INCLUDED
#define BEND_PARAM_HPP_INCLUDED
#include <rack.hpp>
#include "../em.hpp"
using namespace ::rack;
namespace pachde {

inline std::string BendDisplayValue(uint8_t value, bool label = false)
{
    if (value <= 96) {
        return format_string(label ? "Bend %d" : "%d", value);
    } else {
        int ch1Bend = std::max(1, value - 96);
        return format_string(label ? "Bend 96:%d": "96:%d", ch1Bend);
    }
}

struct BendParamQuantity : engine::ParamQuantity
{
    bool is_mpe;

    uint8_t getBendValue() {
        return U8(std::round(getValue()));
    }

    std::string getDisplayValueString() override {
        return BendDisplayValue(getBendValue());
    }

    void setValue(float v) override {
        if (is_mpe && v < 12.f) { v = 12.f; }
        ParamQuantity::setValue(v);
    }

};

template <typename TPQ = BendParamQuantity>
TPQ * configBendParam(Module * module, int paramId)
{
    assert(paramId >= 0 && static_cast<size_t>(paramId) < module->params.size() && static_cast<size_t>(paramId) < module->paramQuantities.size());
    if (module->paramQuantities[paramId]) {
        delete module->paramQuantities[paramId];
    }  
    TPQ* q = new TPQ;
    q->module = module;
    q->paramId = paramId;
    q->minValue = 1.f;
    q->maxValue = 120.f;
    q->name = "Pitch Bend Range";
    q->is_mpe = true;
    q->snapEnabled = true;

    module->paramQuantities[paramId] = q;

    Param* p = &module->params[paramId];
    p->value = q->getDefaultValue();

    return q;
}

}
#endif