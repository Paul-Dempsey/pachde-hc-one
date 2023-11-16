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

    configCCParam(EMCC_CompressorThreshold, false, this, P_COMP_THRESHOLD, IN_COMP_THRESHOLD, P_COMP_THRESHOLD_REL, L_COMP_THRESHOLD_REL, 0.f, 127.f, 127.f, "Threshold", "%", 0.f, 100.f/127.f)->snapEnabled = true;
    configCCParam(EMCC_CompressorThreshold, false, this, P_COMP_ATTACK,     IN_COMP_ATTACK,     P_COMP_ATTACK_REL,     L_COMP_ATTACK_REL,     0.f, 127.f,  64.f, "Attack", "%", 0.f, 100.f/127.f)->snapEnabled = true;
    configCCParam(EMCC_CompressorThreshold, false, this, P_COMP_RATIO,      IN_COMP_RATIO,      P_COMP_RATIO_REL,      L_COMP_RATIO_REL,      0.f, 127.f,  64.f, "Ratio", "%", 0.f, 100.f/127.f)->snapEnabled = true;
    configCCParam(EMCC_CompressorThreshold, false, this, P_COMP_MIX,        IN_COMP_MIX,        P_COMP_MIX_REL,        L_COMP_MIX_REL,        0.f, 127.f,   0.f, "Mix", "%", 0.f, 100.f/127.f)->snapEnabled = true;

    configInput(IN_COMP_THRESHOLD, "Compression threshold");
    configInput(IN_COMP_ATTACK, "Compression attack");
    configInput(IN_COMP_RATIO, "Compression ratio");
    configInput(IN_COMP_MIX, "Compression mix");

    configSwitch(P_COMP_THRESHOLD_REL, 0.f, 1.f, 0.f, "Threshold relative CV", offon);
    configSwitch(P_COMP_ATTACK_REL,    0.f, 1.f, 0.f, "Attack relative CV", offon);
    configSwitch(P_COMP_RATIO_REL,     0.f, 1.f, 0.f, "Ratio relative CV", offon);
    configSwitch(P_COMP_MIX_REL,       0.f, 1.f, 0.f, "Mix relative CV", offon);

    configLight(L_COMPRESSOR, "Compressor");

    configCCParam(EMCC_TiltEq,          false, this, P_TEQ_TILT, IN_TEQ_TILT, P_TEQ_TILT_REL, L_TEQ_TILT_REL, 0.f, 127.f,  64.f, "Tilt", "", 0.f, 1.f)->snapEnabled = true;
    configCCParam(EMCC_TiltEqFrequency, false, this, P_TEQ_FREQ, IN_TEQ_FREQ, P_TEQ_FREQ_REL, L_TEQ_FREQ_REL, 0.f, 127.f,  64.f, "Frequency", "%", 0.f, 100.f/127.f)->snapEnabled = true;
    configCCParam(EMCC_TiltEqMix,       false, this, P_TEQ_MIX,  IN_TEQ_MIX,  P_TEQ_MIX_REL,  L_TEQ_MIX_REL,  0.f, 127.f,   0.f, "Mix", "%", 0.f, 100.f/127.f)->snapEnabled = true;

    configInput(IN_TEQ_TILT, "Tilt EQ tilt");
    configInput(IN_TEQ_FREQ, "Tilt EQ frequency");
    configInput(IN_TEQ_MIX, "Tilt EQ mix");

    configSwitch(P_TEQ_TILT_REL, 0.f, 1.f, 0.f, "Tilt relative CV", offon);
    configSwitch(P_TEQ_FREQ_REL, 0.f, 1.f, 0.f, "Frequency relative CV", offon);
    configSwitch(P_TEQ_MIX_REL,  0.f, 1.f, 0.f, "Mix relative CV", offon);

    configLight(L_TEQ, "Tilt EQ");
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

    if (old.threshold != compressor.threshold) {
        changed = true;
        auto pq = dynamic_cast<CCParamQuantity*>(getParamQuantity(Params::P_COMP_THRESHOLD));
        pq->setValueSilent(compressor.threshold);
    }

    if (old.attack != compressor.attack) {
        changed = true;
        auto pq = dynamic_cast<CCParamQuantity*>(getParamQuantity(Params::P_COMP_ATTACK));
        pq->setValueSilent(compressor.attack);
    }

    if (old.ratio != compressor.ratio) {
        changed = true;
        auto pq = dynamic_cast<CCParamQuantity*>(getParamQuantity(Params::P_COMP_RATIO));
        pq->setValueSilent(compressor.ratio);
    }

    if (old.mix != compressor.mix) {
        changed = true;
        auto pq = dynamic_cast<CCParamQuantity*>(getParamQuantity(Params::P_COMP_MIX));
        pq->setValueSilent(compressor.mix);
    }

    if (changed && ui_event_sink) {
        ui_event_sink->onCompressorChanged(e);
    }
}

void Hc2Module::onTiltEqChanged(const TiltEqChangedEvent& e)
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
    getParamQuantity(Params::P_COMP_THRESHOLD)->setValue(compressor.threshold);
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

void Hc2Module::pullTiltEq(Hc1Module *partner)
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

void Hc2Module::pushTiltEq(Hc1Module *partner)
{
    if (!partner) partner = getPartner();
    if (!partner) return;
    partner->em.tilt_eq = tilt_eq;
    if (ui_event_sink) {
        ui_event_sink->onTiltEqChanged(TiltEqChangedEvent{tilt_eq});
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
    auto pq = dynamic_cast<CCParamQuantity*>(getParamQuantity(Params::P_COMP_THRESHOLD));
    auto v = pq->valueToSend();
    if (pq->last_value != v) {
        compressor.threshold = v;
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
    if (changed) {
        pushCompressor();
    }
}

void Hc2Module::processTiltEqControls()
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

void Hc2Module::processControls()
{
    if (!control_rate.process()) { return; }
    processRoundingControls();
    processCompressorControls();
    processTiltEqControls();
}

void Hc2Module::process(const ProcessArgs& args)
{
    if (++check_cv > CV_INTERVAL) {
        check_cv = 0;
        processCV(Params::P_ROUND_RATE);
        processCV(Params::P_COMP_THRESHOLD);
        processCV(Params::P_COMP_ATTACK);
        processCV(Params::P_COMP_RATIO);
        processCV(Params::P_COMP_MIX);
        processCV(Params::P_TEQ_TILT);
        processCV(Params::P_TEQ_FREQ);
        processCV(Params::P_TEQ_MIX);
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
    getLight(Lights::L_COMPRESSOR).setBrightness(static_cast<float>(compressor.mix)/127.f);
    getLight(Lights::L_TEQ).setBrightness(static_cast<float>(tilt_eq.mix)/127.f);
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