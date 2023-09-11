#include "HC-2.hpp"
#include "../cc_param.hpp"
#include "../em_types_ui.hpp"
#include "../colors.hpp"
#include "../misc.hpp"
#include "../text.hpp"

namespace pachde {

Hc2Module::Hc2Module()
{
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);

    configInput(Inputs::ROUND_RATE_INPUT, "Round rate");

    auto p = configCCParam(EMCC_RoundRate, false, this, Params::ROUND_RATE_PARAM, Inputs::ROUND_RATE_INPUT, Params::ROUND_RATE_REL_PARAM, Lights::ROUND_RATE_REL_LIGHT, 0.f, 127.f, 0.f, "Round rate");
    p->snapEnabled = true;
    configSwitch(ROUND_RATE_REL_PARAM, 0.f, 1.f, 0.f, "Round rate CV-relative", {"off", "on"});
    configSwitch(ROUND_INITIAL_PARAM, 0.f, 1.f, 0.f, "Round initial", {"off", "on"});
    configSwitch(ROUND_KIND_PARAM, 0.f, 3.f, 0.f, "Rounding", {
        "Normal",
        "Release",
        "Y (Full to none)",
        "Inverse Y (None to full)"
        });
    configTuningParam(ROUND_TUNING_PARAM, this, "Tuning");

    configLight(ROUND_RATE_REL_LIGHT, "Rate CV-relative");
    configLight(ROUND_INITIAL_LIGHT, "Round initial");
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

const char * Hc2Module::getDeviceName()
{
    if (auto partner = getPartner()) {
        return partner->device_name.c_str();
    }
    return "<Eagan Matrix Device>";
}

void Hc2Module::onExpanderChange(const ExpanderChangeEvent& e)
{
    partner_side.clear();
    auto left = dynamic_cast<Hc1Module*>(getLeftExpander().module);
    auto right = dynamic_cast<Hc1Module*>(getRightExpander().module);
    if (left) {
        partner_side.addLeft();
        left->expanderAdded(Expansion::Right);
    }
    if (right) {
        partner_side.addRight();
        right->expanderAdded(Expansion::Left);
    }
}

void Hc2Module::processCV(int paramId)
{
    CCParamQuantity* pq = static_cast<CCParamQuantity*>(getParamQuantity(paramId));
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

void Hc2Module::process(const ProcessArgs& args)
{
    if (++check_cv > CV_INTERVAL) {
        check_cv = 0;

        processCV(Params::ROUND_RATE_PARAM);
    }
    // sync params
    //if (auto partner = getPartner()) {
    //}
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