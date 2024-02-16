#include "Tilt.hpp"
#include "../colors.hpp"
#include "../misc.hpp"
#include "../module_broker.hpp"
#include "../text.hpp"
#include "../widgets/cc_param.hpp"
#include "../widgets/components.hpp"
#include "../widgets/pedal_param.hpp"

namespace pachde {

TiltModule::TiltModule()
{
    std::vector<std::string> offon = {"off", "on"};
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);

    configCCParam0(EMCC_TiltEq,          false, this, P_TEQ_TILT, IN_TEQ_TILT, P_TEQ_TILT_REL, L_TEQ_TILT_REL, 0.f, 127.f,  64.f, "Tilt", "", 0.f, 1.f)->snapEnabled = true;
    configCCParam0(EMCC_TiltEqFrequency, false, this, P_TEQ_FREQ, IN_TEQ_FREQ, P_TEQ_FREQ_REL, L_TEQ_FREQ_REL, 0.f, 127.f,  64.f, "Frequency", "", 0.f, 1.f)->snapEnabled = true;
    configCCParam0(EMCC_TiltEqMix,       false, this, P_TEQ_MIX,  IN_TEQ_MIX,  P_TEQ_MIX_REL,  L_TEQ_MIX_REL,  0.f, 127.f,   0.f, "Mix", "%", 0.f, 100.f/127.f)->snapEnabled = true;

    configInput(IN_TEQ_TILT, "Tilt EQ Tilt");
    configInput(IN_TEQ_FREQ, "Tilt EQ Frequency");
    configInput(IN_TEQ_MIX, "Tilt EQ Mix");

    configSwitch(P_TEQ_TILT_REL, 0.f, 1.f, 0.f, "Tilt relative CV", offon);
    configSwitch(P_TEQ_FREQ_REL, 0.f, 1.f, 0.f, "Frequency relative CV", offon);
    configSwitch(P_TEQ_MIX_REL,  0.f, 1.f, 0.f, "Mix relative CV", offon);

    configLight(L_TEQ, "Tilt EQ");
    partner_binding.setClient(this);
}

TiltModule::~TiltModule()
{
    partner_binding.unsubscribe();
}

json_t * TiltModule::dataToJson()
{
    auto root = json_object();
    json_object_set_new(root, "device", json_string(partner_binding.claim.c_str()));
    return root;
}

void TiltModule::dataFromJson(json_t *root)
{
    auto j = json_object_get(root, "device");
    if (j) {
        partner_binding.setClaim(json_string_value(j));
    }
    getPartner();
}

Hc1Module* TiltModule::getPartner()
{
    return partner_binding.getPartner();
}

// ISendMidi
void TiltModule::sendControlChange(uint8_t channel, uint8_t cc, uint8_t value)
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
bool TiltModule::readyToSend()
{
    auto partner = getPartner();
    return partner && partner->ready();
}

void TiltModule::onTiltEqChanged(const TiltEqChangedEvent& e)
{
    bool changed = false;
    auto old = tilt_eq;
    tilt_eq = e.tilt_eq;

    if (old.tilt != tilt_eq.tilt) {
        changed = true;
        auto pq = dynamic_cast<CCParamQuantity*>(getParamQuantity(Params::P_TEQ_TILT));
        pq->setValueSilent(tilt_eq.tilt);
    }
    if (old.frequency != tilt_eq.frequency) {
        changed = true;
        auto pq = dynamic_cast<CCParamQuantity*>(getParamQuantity(Params::P_TEQ_FREQ));
        pq->setValueSilent(tilt_eq.frequency);
    }
    if (old.mix != tilt_eq.mix) {
        changed = true;
        auto pq = dynamic_cast<CCParamQuantity*>(getParamQuantity(Params::P_TEQ_MIX));
        pq->setValueSilent(tilt_eq.mix);
    }
    if (changed && ui_event_sink) {
        ui_event_sink->onTiltEqChanged(e);
    }
}

void TiltModule::onDeviceChanged(const DeviceChangedEvent& e)
{
    partner_binding.onDeviceChanged(e);
    if (ui_event_sink) {
        ui_event_sink->onDeviceChanged(e);
    }
}

void TiltModule::onDisconnect(const DisconnectEvent& e)
{
    partner_binding.onDisconnect(e);
    if (ui_event_sink) {
        ui_event_sink->onDisconnect(e);
    }
}

void TiltModule::pullTiltEq(Hc1Module *partner)
{
    if (!partner) partner = getPartner();
    if (!partner) return;
    tilt_eq = partner->em.tilt_eq;
    getParamQuantity(Params::P_TEQ_TILT)->setValue(tilt_eq.tilt);
    getParamQuantity(Params::P_TEQ_FREQ)->setValue(tilt_eq.frequency);
    getParamQuantity(Params::P_TEQ_MIX)->setValue(tilt_eq.mix);
    if (ui_event_sink) {
        ui_event_sink->onTiltEqChanged(TiltEqChangedEvent{tilt_eq});
    }
}

void TiltModule::pushTiltEq(Hc1Module *partner)
{
    if (!partner) partner = getPartner();
    if (!partner) return;
    partner->em.tilt_eq = tilt_eq;
    if (ui_event_sink) {
        ui_event_sink->onTiltEqChanged(TiltEqChangedEvent{tilt_eq});
    }
}

void TiltModule::processTiltEqControls()
{
    bool changed = false;

    auto pq = dynamic_cast<CCParamQuantity*>(getParamQuantity(Params::P_TEQ_TILT));
    auto v = pq->valueToSend();
    if (pq->last_value != v) {
        tilt_eq.tilt = v;
        changed = true;
        pq->syncValue();
    }
    pq = dynamic_cast<CCParamQuantity*>(getParamQuantity(Params::P_TEQ_FREQ));
    v = pq->valueToSend();
    if (pq->last_value != v) {
        tilt_eq.frequency = v;
        changed = true;
        pq->syncValue();
    }
    pq = dynamic_cast<CCParamQuantity*>(getParamQuantity(Params::P_TEQ_MIX));
    v = pq->valueToSend();
    if (pq->last_value != v) {
        tilt_eq.mix = v;
        changed = true;
        pq->syncValue();
    }
    if (changed) {
        pushTiltEq();
    }
}

void TiltModule::processControls()
{
    if (!control_rate.process()) { return; }
    processTiltEqControls();
}

void TiltModule::processCV(int paramId)
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

void TiltModule::process(const ProcessArgs& args)
{
    if ((0 == ((args.frame + id) % CV_INTERVAL))) {
        processCV(Params::P_TEQ_TILT);
        processCV(Params::P_TEQ_FREQ);
        processCV(Params::P_TEQ_MIX);
    }

    processControls();
    getLight(Lights::L_TEQ).setBrightness(static_cast<float>(tilt_eq.mix)/127.f);
}

}

Model *modelTilt = createModel<pachde::TiltModule, pachde::TiltModuleWidget>("pachde-hc-tilteq");