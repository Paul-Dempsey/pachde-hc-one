#pragma once
#ifndef INDEX_PARAM_HPP_INCLUDED
#define INDEX_PARAM_HPP_INCLUDED
#include <rack.hpp>
#include "../misc.hpp"
using namespace ::rack;
namespace pachde {

template<typename TEnum>
struct EnumQuantity : SwitchQuantity
{
    std::vector<TEnum> map;
    bool clip_max; // clip to max value (else default value)

    EnumQuantity(
        std::vector<TEnum> items, 
        std::vector<std::string> labels = {})
    : map(items), clip_max(false)
    {
        SwitchQuantity::labels = labels;
    }

    TEnum getItemValue() {
        return getItemValue(getValue());
    }
    TEnum getItemValue(float index_value) {
        size_t index = static_cast<size_t>(std::round(clamp(index_value, minValue, maxValue)));
        return map[index];
    }
    void setEnumValue(TEnum evalue) {
        auto it = std::find(map.cbegin(), map.cend(), evalue);
        if (it == map.cend()) {
            setValue(clip_max ? maxValue : defaultValue);
        } else {
            setValue(static_cast<float>(std::distance(map.cbegin(), it)));
        }
    }
};

template<typename TEnum>
EnumQuantity<TEnum>* configEnumParam(
    int paramId,
    std::string name,
    Module* module,
    TEnum defaultValue,
    std::vector<TEnum> items,
    std::vector<std::string> labels,
    bool clipToMax = false /* otherwise out-of range values are set to default */
    )
{
    assert(paramId >= 0 && static_cast<size_t>(paramId) < module->params.size() && static_cast<size_t>(paramId) < module->paramQuantities.size());
    if (module->paramQuantities[paramId]) {
        delete module->paramQuantities[paramId];
    }
    auto q = new EnumQuantity<TEnum>(items, labels);
    q->clip_max = clipToMax;
    q->module = module;
    q->paramId = paramId;
    q->name = name;
    q->minValue = 0.f;
    q->maxValue = items.size() - 1;
    q->setEnumValue(defaultValue);
    q->defaultValue = q->getValue();
    q->snapEnabled = true;
    q->smoothEnabled = false;

    module->paramQuantities[paramId] = q;
    return q;
}

}
#endif