#pragma once
#ifndef EM_TYPES_UI_HPP_INCLUDED
#define EM_TYPES_UI_HPP_INCLUDED
#include <rack.hpp>
#include "em_types.hpp"
#include "cc_param.hpp"

namespace pachde {

struct PDSmallButton : app::SvgSwitch {
    PDSmallButton() {
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/TinyPush_up.svg")));
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/TinyPush_down.svg")));
    }
};

template <typename TLight>
struct PDLightButton : app::SvgSwitch {
	app::ModuleLightWidget* light;

	PDLightButton() {
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/TinyPush_up.svg")));
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/TinyPush_down.svg")));
		light = new TLight;
		// Move center of light to center of box
		light->box.pos = box.size.div(2).minus(light->box.size.div(2));
		addChild(light);
	}

	app::ModuleLightWidget* getLight() {
		return light;
	}
};
template <typename TLight>
using SmallLightButton = PDLightButton<TLight>;

template <typename TLight>
struct PDLightLatch : PDLightButton<TLight> {
	PDLightLatch() {
		this->momentary = false;
		this->latch = true;
	}
};

struct TuningParamQuantity: CCParamQuantity
{
    TuningParamQuantity()
    {
        minValue = float(PackedTuning::ptEqual);
        maxValue= float(PackedTuning::ptUserLast);
        defaultValue = float(PackedTuning::ptEqual);
        snapEnabled = true;
        smoothEnabled = false;
        cc = EMCC_TuningGrid;
        high_resolution = false;
    }

    std::string getDisplayValueString() override
    {
        PackedTuning tuning = static_cast<PackedTuning>(static_cast<uint8_t>(clamp(std::round(getValue()), minValue, maxValue)));
        return describeTuning(unpackTuning(tuning));
    }
};

template <typename TTPQ = TuningParamQuantity>
TTPQ* configTuningParam(int paramId, Module* module, std::string name)
{
    if (module->paramQuantities[paramId]) {
        delete module->paramQuantities[paramId];
    }
    TTPQ* q = new TTPQ();
    q->module = module;
    q->name = name;
    q->paramId = paramId;
    module->paramQuantities[paramId] = q;

    Param* p = &module->params[paramId];
    p->value = q->getDefaultValue();

    return q;
}
// TODO: Tuning UI

}
#endif