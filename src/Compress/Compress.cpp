#include "Compress.hpp"
#include "../colors.hpp"
#include "../misc.hpp"
#include "../module_broker.hpp"
#include "../text.hpp"
#include "../widgets/cc_param.hpp"
#include "../widgets/components.hpp"

namespace pachde {

CompressModule::CompressModule()
{
    std::vector<std::string> offon = {"off", "on"};

    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);

    configCCParam0(EMCC_CompressorThreshold, false, this, P_COMP_THRESHOLD,  IN_COMP_THRESHOLD,  P_COMP_THRESHOLD_REL,  L_COMP_THRESHOLD_REL, 0.f, 127.f, 127.f, "Threshold", "", 0.f, 1.f)->snapEnabled = true;
    configCCParam0(EMCC_CompressorAttack,    false, this, P_COMP_ATTACK,     IN_COMP_ATTACK,     P_COMP_ATTACK_REL,     L_COMP_ATTACK_REL,    0.f, 127.f,  64.f, "Attack", "",    0.f, 1.f)->snapEnabled = true;
    configCCParam0(EMCC_CompressorRatio,     false, this, P_COMP_RATIO,      IN_COMP_RATIO,      P_COMP_RATIO_REL,      L_COMP_RATIO_REL,     0.f, 127.f,  64.f, "Ratio", "",     0.f, 1.f)->snapEnabled = true;
    configCCParam0(EMCC_CompressorMix,       false, this, P_COMP_MIX,        IN_COMP_MIX,        P_COMP_MIX_REL,        L_COMP_MIX_REL,       0.f, 127.f,   0.f, "Mix", "%",       0.f, 100.f/127.f);

    configInput(IN_COMP_THRESHOLD, "Compression threshold");
    configInput(IN_COMP_ATTACK, "Compression attack");
    configInput(IN_COMP_RATIO, "Compression ratio");
    configInput(IN_COMP_MIX, "Compression mix");

    configSwitch(P_COMP_THRESHOLD_REL, 0.f, 1.f, 0.f, "Threshold relative CV", offon);
    configSwitch(P_COMP_ATTACK_REL,    0.f, 1.f, 0.f, "Attack relative CV", offon);
    configSwitch(P_COMP_RATIO_REL,     0.f, 1.f, 0.f, "Ratio relative CV", offon);
    configSwitch(P_COMP_MIX_REL,       0.f, 1.f, 0.f, "Mix relative CV", offon);

    configLight(L_COMPRESSOR, "Compressor");
    partner_binding.setClient(this);
}

CompressModule::~CompressModule()
{
    partner_binding.unsubscribe();
}

json_t * CompressModule::dataToJson()
{
    auto root = json_object();
    if (!partner_binding.claim.empty()){
        json_object_set_new(root, "device", json_string(partner_binding.claim.c_str()));
    }
    return root;
}

void CompressModule::dataFromJson(json_t *root)
{
    auto j = json_object_get(root, "device");
    if (j) {
        partner_binding.setClaim(json_string_value(j));
    }
    getPartner();
}

Hc1Module* CompressModule::getPartner()
{
    return partner_binding.getPartner();
}

void CompressModule::onCompressorChanged(const CompressorChangedEvent &e)
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

void CompressModule::onDeviceChanged(const DeviceChangedEvent& e)
{
    partner_binding.onDeviceChanged(e);
    if (ui_event_sink) {
        ui_event_sink->onDeviceChanged(e);
    }
}

void CompressModule::onDisconnect(const DisconnectEvent& e)
{
    partner_binding.onDisconnect(e);
    if (ui_event_sink) {
        ui_event_sink->onDisconnect(e);
    }
}

void CompressModule::pullCompressor(Hc1Module *partner)
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

void CompressModule::pushCompressor(Hc1Module *partner)
{
    if (!partner) partner = getPartner();
    if (!partner) return;
    partner->em.compressor = compressor;
    if (ui_event_sink) {
        ui_event_sink->onCompressorChanged(CompressorChangedEvent{compressor});
    }
}

void CompressModule::absoluteCV()
{
    getParam(P_COMP_THRESHOLD_REL).setValue(0.f);
    getParam(P_COMP_ATTACK_REL).setValue(0.f);
    getParam(P_COMP_RATIO_REL).setValue(0.f);
    getParam(P_COMP_MIX_REL).setValue(0.f);
}

void CompressModule::relativeCV()
{
    getParam(P_COMP_THRESHOLD_REL).setValue(1.f);
    getParam(P_COMP_ATTACK_REL).setValue(1.f);
    getParam(P_COMP_RATIO_REL).setValue(1.f);
    getParam(P_COMP_MIX_REL).setValue(1.f);
}

void CompressModule::processCV(int paramId)
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

void CompressModule::processCompressorControls()
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

void CompressModule::processControls()
{
    if (!control_rate.process()) { return; }
    processCompressorControls();
}

void CompressModule::process(const ProcessArgs& args)
{
    auto partner = getPartner();
    if (!partner || !partner->readyToSend()) return;

    if ((0 == ((args.frame + id) % CV_INTERVAL))) {
        processCV(Params::P_COMP_THRESHOLD);
        processCV(Params::P_COMP_ATTACK);
        processCV(Params::P_COMP_RATIO);
        processCV(Params::P_COMP_MIX);
    }

    processControls();
    getLight(Lights::L_COMPRESSOR).setBrightness(static_cast<float>(compressor.mix)/127.f);
}

// ISendMidi
void CompressModule::sendControlChange(uint8_t channel, uint8_t cc, uint8_t value)
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
bool CompressModule::readyToSend()
{
    auto partner = getPartner();
    return partner && partner->ready();
}

}

Model *modelCompress = createModel<pachde::CompressModule, pachde::CompressModuleWidget>("pachde-hc-compressor");