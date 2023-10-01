#pragma once
#ifndef CC_PARAM_HPP_INCLUDED
#define CC_PARAM_HPP_INCLUDED
#include <rack.hpp>
#include "components.hpp"
#include "../em_midi.hpp"

namespace pachde {
using namespace em_midi;

struct CCParamQuantity : rack::engine::ParamQuantity
{
    uint8_t cc = 0; // never a valid cc for param
    uint16_t last_value = 0;
    bool high_resolution = true;
    bool enabled = true;
    float offset = 0.f;
    int inputId = -1;
    int relativeId = -1;
    int lightId = -1;

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
        if (!enabled) return;

        auto iSend = dynamic_cast<ISendMidi*>(module);
        if (iSend && iSend->readyToSend()) {
            if (high_resolution) {
                uint8_t lo = to_send & 0x7f;
                if (lo) {
                    iSend->sendControlChange(EM_SettingsChannel, EMCC_PedalFraction, lo);
                }
                uint8_t hi = to_send >> 7;
                iSend->sendControlChange(EM_SettingsChannel, cc, hi);
            } else {
                iSend->sendControlChange(EM_SettingsChannel, cc, to_send & 0x7f);
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
TCCPQ* configCCParam(uint8_t cc, bool hiRes, Module* module, int paramId, int inputId, int relativeParamId, int lightId, float minValue, float maxValue, float defaultValue, std::string name = "", std::string unit = "", float displayBase = 0.f, float displayMultiplier = 1.f, float displayOffset = 0.f)
{
    assert(paramId >= 0 && static_cast<size_t>(paramId) < module->params.size() && static_cast<size_t>(paramId) < module->paramQuantities.size());
    if (module->paramQuantities[paramId]) {
        delete module->paramQuantities[paramId];
    }

    TCCPQ* q = new TCCPQ;
    q->module = module;
    q->paramId = paramId;
    q->inputId = inputId;
    q->relativeId = relativeParamId;
    q->lightId = lightId;
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

const NVGcolor connected_track_color = nvgRGB(0x73, 0x5d, 0x26);

struct ModKnob : SmallBlackKnob
{
    int inputId = -1;
    int relativeParamId = -1;

    void drawLayer(const DrawArgs& args, int layer) override
    {
        SmallBlackKnob::drawLayer(args, layer);
        if (1 != layer) return;

        if (module
            && inputId >= 0
            && module->getInput(inputId).isConnected()
            ) {
            // track
            DrawKnobTrack(args.vg, this, 13.25, 1.2f, connected_track_color);

            // LED
            if (relativeParamId >= 0 && module->getParam(relativeParamId).getValue() >= .5f)
            {
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
    }

    void draw(const DrawArgs& args) override {
        SmallBlackKnob::draw(args);
        if ((inputId >= 0) && (!module || !module->getInput(inputId).isConnected())) {
            DrawKnobTrack(args.vg, this, 13.25, 1.2f, RampGray(G_30));
        }
    }
};

template <typename T = ModKnob>
T* createModKnob(Vec pos, Module * module, int paramId, int inputId, int relativeId)
{
    auto mk = createParamCentered<T>(pos, module, paramId);
    mk->setImage();
    mk->inputId = inputId;
    mk->relativeParamId = relativeId;
    return mk;
}

}
#endif