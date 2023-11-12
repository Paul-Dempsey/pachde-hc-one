#include "HC-2.hpp"
#include "../colors.hpp"
#include "../misc.hpp"
#include "../module_broker.hpp"
#include "../text.hpp"
#include "../widgets/cc_param.hpp"
#include "../widgets/components.hpp"
#include "../widgets/pedal_param.hpp"
#include "tuning_ui.hpp"

namespace pachde {

Hc2Module::Hc2Module()
{
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);

    configInput(Inputs::IN_ROUND_RATE, "Round rate");

    auto p = configCCParam(EMCC_RoundRate, false, this, Params::P_ROUND_RATE, Inputs::IN_ROUND_RATE, Params::P_ROUND_RATE_REL, Lights::L_ROUND_RATE_REL, 0.f, 127.f, 0.f, "Round rate");
    p->snapEnabled = true;
    configSwitch(P_ROUND_RATE_REL, 0.f, 1.f, 0.f, "Round rate CV-relative", {"off", "on"});
    configSwitch(P_ROUND_INITIAL, 0.f, 1.f, 0.f, "Round initial", {"off", "on"});
    configInput(Inputs::IN_ROUND_INITIAL, "Round initial");
    configSwitch(P_ROUND_KIND, 0.f, 3.f, 0.f, "Round type", {
        "Normal",
        "Release",
        "Y (Full to none)",
        "Inverse Y (None to full)"
        });
    configTuningParam(this, P_ROUND_TUNING);

    configCCParam(EMCC_CompressorThreshhold, false, this, P_COMP_THRESHHOLD, IN_COMP_THRESHHOLD, P_COMP_THRESHHOLD_REL, L_COMP_THRESHHOLD_REL, 0.f, 127.f, 127.f, "Compressor threshhold")->snapEnabled = true;
    configCCParam(EMCC_CompressorThreshhold, false, this, P_COMP_ATTACK,     IN_COMP_ATTACK,     P_COMP_ATTACK_REL,     L_COMP_ATTACK_REL,     0.f, 127.f,  64.f, "Compressor attack")->snapEnabled = true;
    configCCParam(EMCC_CompressorThreshhold, false, this, P_COMP_RATIO,      IN_COMP_RATIO,      P_COMP_RATIO_REL,      L_COMP_RATIO_REL,      0.f, 127.f,  64.f, "Compressor ratio")->snapEnabled = true;
    configCCParam(EMCC_CompressorThreshhold, false, this, P_COMP_MIX,        IN_COMP_MIX,        P_COMP_MIX_REL,        L_COMP_MIX_REL,        0.f, 127.f,   0.f, "Compressor mix")->snapEnabled = true;
    configLight(L_COMPRESSOR, "Compressor");
}

Hc2Module::~Hc2Module()
{
    if (!partner_subscribed) return;
    auto partner = partner_binding.getPartner();
    if (partner){
        partner->unsubscribeHcEvents(this);
        partner_subscribed = false;
    }
}
json_t * Hc2Module::dataToJson()
{
    auto root = json_object();
    json_object_set_new(root, "device", json_string(partner_binding.claim.c_str()));
    return root;
}

void Hc2Module::dataFromJson(json_t *root)
{
    auto j = json_object_get(root, "device");
    if (j) {
        partner_binding.setClaim(json_string_value(j));
    }
    getPartner();
}

Hc1Module* Hc2Module::getPartner()
{
    return getPartnerImpl<Hc2Module>(this);
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
        // $BUGBUG: why is this commented out?
        //getParamQuantity(Params::P_ROUND_EQUAL)->setValue(rounding.equal);
    }

    if (changed && ui_event_sink) {
        ui_event_sink->onRoundingChanged(e);
    }
}

void Hc2Module::onCompressorChanged(const CompressorChangedEvent &e)
{
    bool changed = false;
    auto old = compressor;
    compressor = e.compressor;

    if (old.threshhold != compressor.threshhold) {
        changed = true;
        getParamQuantity(Params::P_COMP_THRESHHOLD)->setValue(static_cast<uint8_t>(compressor.threshhold));
    }
    if (old.attack != compressor.attack) {
        changed = true;
        getParamQuantity(Params::P_COMP_ATTACK)->setValue(static_cast<uint8_t>(compressor.attack));
    }
    if (old.ratio != compressor.ratio) {
        changed = true;
        getParamQuantity(Params::P_COMP_RATIO)->setValue(static_cast<uint8_t>(compressor.ratio));
    }
    if (old.mix != compressor.mix) {
        changed = true;
        getParamQuantity(Params::P_COMP_MIX)->setValue(static_cast<uint8_t>(compressor.mix));
    }

    if (changed && ui_event_sink) {
        ui_event_sink->onCompressorChanged(e);
    }
}

void Hc2Module::onDeviceChanged(const DeviceChangedEvent& e)
{
    partner_binding.onDeviceChanged(e);
    if (ui_event_sink) {
        ui_event_sink->onDeviceChanged(e);
    }
}

void Hc2Module::onDisconnect(const DisconnectEvent& e)
{
    partner_subscribed = false;
    if (ui_event_sink) {
        ui_event_sink->onDisconnect(e);
    }
}

void Hc2Module::pullRounding(Hc1Module * partner)
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

void Hc2Module::pushRounding(Hc1Module * partner)
{
    if (!partner) partner = getPartner();
    if (!partner) return;
    partner->em.rounding = rounding;
    if (ui_event_sink) {
        ui_event_sink->onRoundingChanged(RoundingChangedEvent{rounding});
    }
}

void Hc2Module::pullCompressor(Hc1Module *partner)
{
    if (!partner) partner = getPartner();
    if (!partner) return;
    compressor = partner->em.compressor;
    getParamQuantity(Params::P_COMP_THRESHHOLD)->setValue(compressor.threshhold);
    getParamQuantity(Params::P_COMP_ATTACK)->setValue(compressor.attack);
    getParamQuantity(Params::P_COMP_RATIO)->setValue(compressor.ratio);
    getParamQuantity(Params::P_COMP_MIX)->setValue(compressor.mix);
    if (ui_event_sink) {
        ui_event_sink->onCompressorChanged(CompressorChangedEvent{compressor});
    }
}

void Hc2Module::pushCompressor(Hc1Module *partner)
{
    if (!partner) partner = getPartner();
    if (!partner) return;
    partner->em.compressor = compressor;
    if (ui_event_sink) {
        ui_event_sink->onCompressorChanged(CompressorChangedEvent{compressor});
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

void Hc2Module::processRoundingControls()
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
            uint8_t rev = partner ? partner->em.reverse_surface : 0;
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

void Hc2Module::processCompressorControls()
{
    bool changed = false;
    auto pq = dynamic_cast<CCParamQuantity*>(getParamQuantity(Params::P_COMP_THRESHHOLD));
    auto v = pq->valueToSend();
    if (pq->last_value != v) {
        compressor.threshhold = v;
        changed = true;
        pq->syncValue();
    }
    pq = dynamic_cast<CCParamQuantity*>(getParamQuantity(Params::P_COMP_ATTACK));
    v = pq->valueToSend();
    if (pq->last_value != v) {
        compressor.attack = v;
        changed = true;
        pq->syncValue();
    }
    pq = dynamic_cast<CCParamQuantity*>(getParamQuantity(Params::P_COMP_RATIO));
    v = pq->valueToSend();
    if (pq->last_value != v) {
        compressor.ratio = v;
        changed = true;
        pq->syncValue();
    }
    pq = dynamic_cast<CCParamQuantity*>(getParamQuantity(Params::P_COMP_MIX));
    v = pq->valueToSend();
    if (pq->last_value != v) {
        compressor.mix = v;
        changed = true;
        pq->syncValue();
    }
    getLight(Lights::L_COMPRESSOR).setBrightness(static_cast<float>(compressor.mix)/127.f);

    if (changed) {
        pushCompressor();
    }
}

void Hc2Module::processControls()
{
    if (!control_rate.process()) { return; }
    processRoundingControls();
    processCompressorControls();
}

void Hc2Module::process(const ProcessArgs& args)
{
    if (++check_cv > CV_INTERVAL) {
        check_cv = 0;
        processCV(Params::P_ROUND_RATE);
        processCV(Params::P_COMP_THRESHHOLD);
        processCV(Params::P_COMP_ATTACK);
        processCV(Params::P_COMP_RATIO);
        processCV(Params::P_COMP_MIX);
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