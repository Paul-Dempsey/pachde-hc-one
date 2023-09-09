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
    bool enabled = true;
    float offset = 0.f;

    void enable(bool enable) { enabled = enable; }

    uint16_t clipValue(float value)
    {
        return high_resolution
            ? static_cast<uint16_t>(clamp(value, 0.f, EM_Max14f))
            : static_cast<uint16_t>(clamp(value, 0.f, 127.f));
    }

    uint16_t valueToSend()
    {
        auto p = getParam();
        return p ? clipValue(p->getValue() + offset) : 0;
    }

    void setRelativeVoltage(float v)
    {
        offset = v * .2f * getMaxValue() * .5f;
    }

    void setKnobVoltage(float v)
    {
        // knob is unipolar 0-10v
        v = v * .1f * getMaxValue();
        setValue(v);
    }

    void sendValue()
    {
        auto to_send = valueToSend();
        last_value = to_send;
        if (enabled) {
            auto my_module = dynamic_cast<Hc1Module*>(module);
            if (my_module && my_module->ready()) {
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

    void syncValue()
    {
        assert(cc);
        auto to_send = valueToSend();
        if (last_value != to_send) {
            sendValue();
        }
    }

    void updateLastValue() {
        last_value = valueToSend();
    }

    void setValueSilent(float newValue)
    {
        auto p = getParam();
        if (!p) return;
        p->setValue(newValue);
        updateLastValue();
    }
};

template <class TCCPQ = CCParamQuantity>
TCCPQ* configCCParam(uint8_t cc, bool hiRes, Module* module, int paramId, float minValue, float maxValue, float defaultValue, std::string name = "", std::string unit = "", float displayBase = 0.f, float displayMultiplier = 1.f, float displayOffset = 0.f)
{
    assert(paramId >= 0 && static_cast<size_t>(paramId) < module->params.size() && static_cast<size_t>(paramId) < module->paramQuantities.size());
    if (module->paramQuantities[paramId]) {
        delete module->paramQuantities[paramId];
    }

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

struct MidiKnob : RoundSmallBlackKnob
{
    void drawLayer(const DrawArgs& args, int layer) override
    {
        RoundSmallBlackKnob::drawLayer(args, layer);
        if (1 != layer) return;
        if (module
            && module->getInput(paramId).isConnected()
            && module->getParam(13 + paramId).getValue() > .5f
            ) {
            auto pq = dynamic_cast<CCParamQuantity*>(getParamQuantity());
            if (!pq) return;
            float value = pq->clipValue(pq->offset + pq->getValue()) / pq->getMaxValue();
            float angle = math::rescale(value, 0.f, 1.f, minAngle, maxAngle);
            angle = std::fmod(angle, 2 * M_PI);

        	float transform[6];
            nvgTransformIdentity(transform);
            float t[6];
            math::Vec center = sw->box.getCenter();
            nvgTransformTranslate(t, VEC_ARGS(center));
            nvgTransformPremultiply(transform, t);
            nvgTransformRotate(t, angle);
            nvgTransformPremultiply(transform, t);
            nvgTransformTranslate(t, VEC_ARGS(center.neg()));
    		nvgTransformPremultiply(transform, t);

            auto vg = args.vg;
            nvgSave(vg);
    		nvgTransform(vg, transform[0], transform[1], transform[2], transform[3], transform[4], transform[5]);
            //Line(vg, center.x, center.y, center.x, center.y - 14.f, preset_name_color, .75f);
            CircularHalo(vg, center.x, center.y - 13.f, 2.75f, 9.5f, preset_name_color);
            Circle(vg, center.x, center.y - 13.f, 1.75f, preset_name_color);
            nvgRestore(vg);
        }
    }
    void draw(const DrawArgs& args) override {
        RoundSmallBlackKnob::draw(args);

        auto vg = args.vg;
        nvgBeginPath(vg);
        nvgArc(vg, box.size.x *.5f, box.size.y *.5f, 13.25f, minAngle - M_PI/2.f, maxAngle - M_PI/2.f, NVG_CW);
        nvgStrokeColor(vg, RampGray(G_40));
        nvgStrokeWidth(vg, 1.2f);
        nvgLineCap(vg, NVG_ROUND);
        nvgStroke(vg);
    }
};

}
#endif