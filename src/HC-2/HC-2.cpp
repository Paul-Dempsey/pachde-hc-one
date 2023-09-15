#include "HC-2.hpp"
#include "tuning_ui.hpp"
#include "../cc_param.hpp"
#include "../em_types_ui.hpp"
#include "../colors.hpp"
#include "../misc.hpp"
#include "../text.hpp"

namespace pachde {

Hc2Module::Hc2Module()
{
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);

    configInput(Inputs::IN_ROUND_RATE, "Round rate");

    auto p = configCCParam(EMCC_RoundRate, false, this, Params::P_ROUND_RATE, Inputs::IN_ROUND_RATE, Params::P_ROUND_RATE_REL, Lights::L_ROUND_RATE_REL, 0.f, 127.f, 0.f, "Round rate");
    p->snapEnabled = true;
    configSwitch(P_ROUND_RATE_REL, 0.f, 1.f, 0.f, "Round rate CV-relative", {"off", "on"});
    configLight(L_ROUND_RATE_REL, "Rate CV-relative");

    configSwitch(P_ROUND_INITIAL, 0.f, 1.f, 0.f, "Round initial", {"off", "on"});
    configLight(L_ROUND_INITIAL, "Round initial");
    configInput(Inputs::IN_ROUND_INITIAL, "Round initial");
    configSwitch(P_ROUND_KIND, 0.f, 3.f, 0.f, "Round type", {
        "Normal",
        "Release",
        "Y (Full to none)",
        "Inverse Y (None to full)"
        });
    configTuningParam(this, P_ROUND_TUNING);

    configParam(P_TEST, 0.f, 1.f, .5f, "Test");
}

Hc1Module* Hc2Module::getPartner()
{
    if (partner_side.left()) {
        return dynamic_cast<Hc1Module*>(getLeftExpander().module);
    }
    if (partner_side.right()) {
        return dynamic_cast<Hc1Module*>(getRightExpander().module);
    }
    return nullptr;
}

std::string Hc2Module::getDeviceName()
{
    if (auto partner = getPartner()) {
        return partner->device_name;
    }
    return "";
}

void Hc2Module::onExpanderChange(const ExpanderChangeEvent& e)
{
    partner_side.clear();
    auto left = dynamic_cast<Hc1Module*>(getLeftExpander().module);
    auto right = dynamic_cast<Hc1Module*>(getRightExpander().module);
    Hc1Module* partner = nullptr;
    if (left) {
        partner_side.addLeft();
        left->expanderAdded(Expansion::Right);
        partner = left;
    }
    if (right) {
        partner_side.addRight();
        right->expanderAdded(Expansion::Left);
        partner = right;
    }
    if (partner) {
        pullRounding(partner);
    } else {
        rounding.clear();
    }
}

void Hc2Module::onPresetChanged(const PresetChangedEvent& e)
{
    pullRounding();
    if (ui_event_sink) {
        ui_event_sink->onPresetChanged(e);
    }
}

void Hc2Module::onRoundingChanged(const RoundingChangedEvent& e)
{
    bool changed = false;
    auto old = rounding;
    rounding = e.rounding;

    if (old.rate != rounding.rate) {
        changed = true;
        auto pq = dynamic_cast<CCParamQuantity*>(getParamQuantity(Params::P_ROUND_RATE));
        pq->setValueSilent(rounding.rate);
    }

    if (old.initial != rounding.initial) {
        changed = true;
        getParamQuantity(Params::P_ROUND_INITIAL)->setValue(1.f * rounding.initial);
        getLight(Lights::L_ROUND_INITIAL).setBrightness(1.0f * rounding.initial);
    }

    if (old.kind != rounding.kind) {
        changed = true;
        getParamQuantity(Params::P_ROUND_KIND)->setValue(static_cast<uint8_t>(rounding.kind));
    }
    if (old.tuning != rounding.tuning) {
        changed = true;
        getParamQuantity(Params::P_ROUND_TUNING)->setValue(packTuning(rounding.tuning));
    }
    if (old.equal != rounding.equal) {
        changed = true;
        //getParamQuantity(Params::P_ROUND_EQUAL)->setValue(rounding.equal);
    }

    if (changed && ui_event_sink) {
        ui_event_sink->onRoundingChanged(e);
    }
}

void Hc2Module::pullRounding(Hc1Module * partner)
{
    if (!partner) partner = getPartner();
    if (!partner) return;
    rounding = partner->rounding;
    getParamQuantity(Params::P_ROUND_KIND)->setValue(static_cast<uint8_t>(rounding.kind));
    getParamQuantity(Params::P_ROUND_INITIAL)->setValue(1.f * rounding.initial);
    getLight(Lights::L_ROUND_INITIAL).setBrightness(1.0f * rounding.initial);
    dynamic_cast<CCParamQuantity*>(getParamQuantity(Params::P_ROUND_RATE))->setValueSilent(rounding.rate);
    getParamQuantity(Params::P_ROUND_TUNING)->setValue(packTuning(rounding.tuning));
    if (ui_event_sink) {
        ui_event_sink->onRoundingChanged(RoundingChangedEvent{rounding});
    }
}

void Hc2Module::pushRounding(Hc1Module * partner)
{
    if (!partner) partner = getPartner();
    if (!partner) return;
    partner->rounding = rounding;
    if (ui_event_sink) {
        ui_event_sink->onRoundingChanged(RoundingChangedEvent{rounding});
    }
}

void Hc2Module::processCV(int paramId)
{
    auto pq = dynamic_cast<CCParamQuantity*>(getParamQuantity(paramId));
    if (!pq) return;
    if (pq->inputId < 0) return;

    auto in = getInput(pq->inputId);

    bool relative =  pq->relativeId >= 0 ? getParam(pq->relativeId).getValue() > .5f : false;
    if (pq->lightId >= 0) {
        getLight(pq->lightId).setBrightness((relative *.20f) + ((in.isConnected() && relative) *.80f));
    }

    if (in.isConnected()) {
        auto v = in.getVoltage();
        if (relative) {
            pq->setRelativeVoltage(v);
        } else {
            pq->offset = 0.f;
            pq->setKnobVoltage(v);
        }
    } else {
        pq->offset = 0.f;
    }
}

void Hc2Module::syncParam(int paramId)
{
    auto pq = dynamic_cast<CCParamQuantity*>(getParamQuantity(paramId));
    if (pq) {
        pq->syncValue();
    }
}

void Hc2Module::process(const ProcessArgs& args)
{
    if (++check_cv > CV_INTERVAL) {
        check_cv = 0;
        processCV(Params::P_ROUND_RATE);
    }

    if (getInput(Inputs::IN_ROUND_INITIAL).isConnected()) {
        auto v = getInput(Inputs::IN_ROUND_INITIAL).getVoltage();
        if (round_initial_trigger.process(v, 0.1f, 5.f)) {
            round_initial_trigger.reset();
            auto pq = getParamQuantity(Params::P_ROUND_INITIAL);
            bool ri = !(pq->getValue() >= .5f); // toggle
            pq->setValue(1.0f * ri);
        }
    }

    if (control_rate.process()) {

        auto pq = dynamic_cast<CCParamQuantity*>(getParamQuantity(Params::P_ROUND_RATE));
        assert(pq);
        auto rr = pq->valueToSend();
        if (pq->last_value != rr) {
            rounding.rate = rr;
            pushRounding();
            pq->syncValue();
        }

        bool ri = getParamQuantity(Params::P_ROUND_INITIAL)->getValue() >= .5f;
        getLight(Lights::L_ROUND_INITIAL).setBrightness(1.0f * rounding.initial);
        if (rounding.initial != ri) {
            rounding.initial = ri;
            pushRounding();
            sendControlChange(EM_MasterChannel, EMCC_RountInitial, ri * 127);
        }

        RoundKind kind = static_cast<RoundKind>(static_cast<uint8_t>(getParamQuantity(P_ROUND_KIND)->getValue()));
        if (kind != rounding.kind) {
            rounding.kind = kind;
            pushRounding();
            auto partner = getPartner();
            uint8_t rev = partner ? partner->reverse_surface : 0;
            sendControlChange(EM_SettingsChannel, EMCC_Reverse_Rounding, (static_cast<uint8_t>(kind) << 1) | rev);
        }

        auto tq = dynamic_cast<TuningParamQuantity*>(getParamQuantity(P_ROUND_TUNING));
        Tuning tuning = tq->getTuning();
        if (tuning != rounding.tuning) {
            rounding.tuning = tuning;
            pushRounding();
            sendControlChange(EM_SettingsChannel, EMCC_TuningGrid, tuning);
        }
    }
}

// ISendMidi
void Hc2Module::sendControlChange(uint8_t channel, uint8_t cc, uint8_t value)
{
    auto partner = getPartner();
    if (partner) {
        partner->sendControlChange(channel, cc, value);
    }
}
//void sendProgramChange(uint8_t channel, uint8_t program) {}
//void sendKeyPressure(uint8_t channel, uint8_t note, uint8_t pressure) {}
//void sendChannelPressure(uint8_t channel, uint8_t pressure) {}
//void sendPitchBend(uint8_t channel, uint8_t bend_lo, uint8_t bend_hi) {}
bool Hc2Module::readyToSend()
{
    auto partner = getPartner();
    return partner && partner->ready();
}

}

Model *modelHc2 = createModel<pachde::Hc2Module, pachde::Hc2ModuleWidget>("pachde-hc-2");