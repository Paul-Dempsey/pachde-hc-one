#include "Round.hpp"
#include "../widgets/cc_param.hpp"
#include "tuning_ui.hpp"

namespace pachde {

RoundModule::RoundModule()
{
    std::vector<std::string> offon = {"off", "on"};
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);

    configInput(Inputs::IN_ROUND_RATE, "Round rate");

    auto p = configCCParam(EMCC_RoundRate, false, this, Params::P_ROUND_RATE, Inputs::IN_ROUND_RATE, Params::P_ROUND_RATE_REL, Lights::L_ROUND_RATE_REL, 0.f, 127.f, 0.f, "Round rate");
    p->snapEnabled = true;
    configSwitch(P_ROUND_RATE_REL, 0.f, 1.f, 0.f, "Round rate CV-relative", offon);
    configSwitch(P_ROUND_INITIAL, 0.f, 1.f, 0.f, "Round initial", offon);
    configInput(Inputs::IN_ROUND_INITIAL, "Round initial trigger");
    configSwitch(P_ROUND_KIND, 0.f, 3.f, 0.f, "Round type", {
        "Normal",
        "Release",
        "Y (Full to none)",
        "Inverse Y (None to full)"
        });
    configTuningParam(this, P_ROUND_TUNING);
    configLight(Lights::ROUND_Y_LIGHT, "Round on Y");
    configLight(Lights::ROUND_INITIAL_LIGHT, "Round initial");
    configLight(Lights::ROUND_LIGHT, "Rounding");
    configLight(Lights::ROUND_RELEASE_LIGHT, "Round on release");
    partner_binding.setClient(this);

}

RoundModule::~RoundModule()
{
    partner_binding.unsubscribe();
}

json_t * RoundModule::dataToJson()
{
    auto root = json_object();
    if (!partner_binding.claim.empty()) {
        json_object_set_new(root, "device", json_string(partner_binding.claim.c_str()));
    }
    return root;
}

void RoundModule::dataFromJson(json_t *root)
{
    auto j = json_object_get(root, "device");
    if (j) {
        partner_binding.setClaim(json_string_value(j));
    }
    getPartner();
}

Hc1Module* RoundModule::getPartner()
{
    return partner_binding.getPartner();
}

// ISendMidi
void RoundModule::sendControlChange(uint8_t channel, uint8_t cc, uint8_t value)
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
bool RoundModule::readyToSend()
{
    auto partner = getPartner();
    return partner && partner->ready();
}

void RoundModule::onPresetChanged(const PresetChangedEvent &e)
{
    pullRounding();
    if (ui_event_sink) {
        ui_event_sink->onPresetChanged(e);
    }
}

void RoundModule::onRoundingChanged(const RoundingChangedEvent& e)
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
        // $BUGBUG: why is this commented out?
        //getParamQuantity(Params::P_ROUND_EQUAL)->setValue(rounding.equal);
    }

    if (changed && ui_event_sink) {
        ui_event_sink->onRoundingChanged(e);
    }
}

void RoundModule::onDeviceChanged(const DeviceChangedEvent& e)
{
    partner_binding.onDeviceChanged(e);
    if (ui_event_sink) {
        ui_event_sink->onDeviceChanged(e);
    }
}

void RoundModule::onDisconnect(const DisconnectEvent& e)
{
    partner_binding.onDisconnect(e);
    if (ui_event_sink) {
        ui_event_sink->onDisconnect(e);
    }
}

void RoundModule::pullRounding(Hc1Module * partner)
{
    if (!partner) partner = getPartner();
    if (!partner) return;
    rounding = partner->em.rounding;
    getParamQuantity(Params::P_ROUND_KIND)->setValue(static_cast<uint8_t>(rounding.kind));
    getParamQuantity(Params::P_ROUND_INITIAL)->setValue(1.f * rounding.initial);
    getLight(Lights::L_ROUND_INITIAL).setBrightness(1.0f * rounding.initial);
    dynamic_cast<CCParamQuantity*>(getParamQuantity(Params::P_ROUND_RATE))->setValueSilent(rounding.rate);
    getParamQuantity(Params::P_ROUND_TUNING)->setValue(packTuning(rounding.tuning));
    if (ui_event_sink) {
        ui_event_sink->onRoundingChanged(RoundingChangedEvent{rounding});
    }
}

void RoundModule::processCV(int paramId)
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

void RoundModule::pushRounding(Hc1Module * partner)
{
    if (!partner) partner = getPartner();
    if (!partner) return;
    partner->em.rounding = rounding;
    if (ui_event_sink) {
        ui_event_sink->onRoundingChanged(RoundingChangedEvent{rounding});
    }
}

void RoundModule::processRoundingControls()
{
    {
        auto pq = dynamic_cast<CCParamQuantity*>(getParamQuantity(Params::P_ROUND_RATE));
        assert(pq);
        auto rr = pq->valueToSend();
        if (pq->last_value != rr) {
            rounding.rate = rr;
            pushRounding();
            pq->syncValue();
        }
    }
    {
        bool ri = getParamQuantity(Params::P_ROUND_INITIAL)->getValue() >= .5f;
        getLight(Lights::L_ROUND_INITIAL).setBrightness(1.0f * rounding.initial);
        if (rounding.initial != ri) {
            rounding.initial = ri;
            pushRounding();
            sendControlChange(EM_MasterChannel, EMCC_RoundInitial, ri * 127);
        }
    }

    {
        RoundKind kind = static_cast<RoundKind>(static_cast<uint8_t>(getParamQuantity(P_ROUND_KIND)->getValue()));
        if (kind != rounding.kind) {
            rounding.kind = kind;
            pushRounding();
            auto partner = getPartner();
            uint8_t rev = partner ? (partner->em.reverse_surface ? 1 : 0) : 0;
            sendControlChange(EM_SettingsChannel, EMCC_Reverse_Rounding, (static_cast<uint8_t>(kind) << 1) | rev);
        }
    }

    {
        auto tq = dynamic_cast<TuningParamQuantity*>(getParamQuantity(P_ROUND_TUNING));
        Tuning tuning = tq->getTuning();
        if (tuning != rounding.tuning) {
            rounding.tuning = tuning;
            pushRounding();
            sendControlChange(EM_SettingsChannel, EMCC_TuningGrid, tuning);
        }
    }
}

void RoundModule::processControls()
{
    if (!control_rate.process()) { return; }
    processRoundingControls();
}

void RoundModule::process(const ProcessArgs& args)
{
    auto partner = getPartner();
    if (!partner || !partner->readyToSend()) return;

    if (0 == ((args.frame + id) % CV_INTERVAL)) {
        processCV(Params::P_ROUND_RATE);
        bool round = rounding.rate > 0;
        getLight(Lights::ROUND_Y_LIGHT).setBrightness(1.0f * (round && (rounding.kind >= RoundKind::Y)));
        getLight(Lights::ROUND_INITIAL_LIGHT).setBrightness(1.0f * (rounding.initial));
        getLight(Lights::ROUND_LIGHT).setBrightness(1.0f * round);
        getLight(Lights::ROUND_RELEASE_LIGHT).setBrightness(1.0f * (round && (rounding.kind <= RoundKind::Release)));
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
    processControls();
}

}

Model *modelRound = createModel<pachde::RoundModule, pachde::RoundModuleWidget>("pachde-hc-round");