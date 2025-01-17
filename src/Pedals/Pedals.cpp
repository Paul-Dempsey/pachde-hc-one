// Copyright (C) Paul Chase Dempsey
#include "Pedals.hpp"
#include "../widgets/pedal_param.hpp"
namespace pachde {

PedalCore::PedalCore(uint8_t pedal)
: pedal_id(pedal)
{
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);

    configPedalParam(pedal_id, PedalAssign::Sustain, this, Params::P_PEDAL_ASSIGN);

    auto p = configParam(P_PEDAL_VALUE, 0.f, 127.f, 0.f, format_string("Pedal %d value", 1 + pedal_id));
    p->snapEnabled = true;

    p = configParam(P_PEDAL_MIN, 0.f, 127.f, 0.f, format_string("Pedal %d min value", 1 + pedal_id));
    p->snapEnabled = true;
    
    p = configParam(P_PEDAL_MAX, 0.f, 127.f, 127.f, format_string("Pedal %d max value", 1 + pedal_id));
    p->snapEnabled = true;

    configInput(Inputs::I_PEDAL_VALUE, "Pedal");
    configOutput(Outputs::O_PEDAL_VALUE, "Pedal");
    partner_binding.setClient(this);
}

PedalCore::~PedalCore()
{
    partner_binding.unsubscribe();
}

json_t * PedalCore::dataToJson()
{
    auto root = json_object();
    json_object_set_new(root, "device", json_string(partner_binding.claim.c_str()));
    return root;
}

void PedalCore::dataFromJson(json_t *root)
{
    auto j = json_object_get(root, "device");
    if (j) {
        partner_binding.setClaim(json_string_value(j));
    }
    getPartner();
}

Hc1Module* PedalCore::getPartner()
{
    return partner_binding.getPartner();
}

// IHandleHcEvents
void PedalCore::onPresetChanged(const PresetChangedEvent &e)
{
    static_cast<PedalParamQuantity*>(getParamQuantity(P_PEDAL_ASSIGN))->setEnabled(true);
    // UI doesn't need it
    // if (ui_event_sink) {
    //     ui_event_sink->onPresetChanged(e);
    // }
}

void PedalCore::onPedalChanged(const PedalChangedEvent& e)
{
    if (e.pedal.jack != pedal_id) return;
    static_cast<PedalParamQuantity*>(getParamQuantity(Params::P_PEDAL_ASSIGN))
        ->setAssignSilent(PedalAssignFromCC(e.pedal.cc));

    last_pedal_value = e.pedal.value;
    auto pqv = getParamQuantity(Params::P_PEDAL_VALUE);
    pqv->setValue(e.pedal.value);
    pqv->minValue = e.pedal.min;
    pqv->maxValue = e.pedal.max;
    getParamQuantity(Params::P_PEDAL_MIN)->setValue(e.pedal.min);
    getParamQuantity(Params::P_PEDAL_MAX)->setValue(e.pedal.max);

    if (ui_event_sink) {
        ui_event_sink->onPedalChanged(e);
    }
}

void PedalCore::onDeviceChanged(const DeviceChangedEvent& e)
{
    partner_binding.onDeviceChanged(e);
    if (ui_event_sink) {
        ui_event_sink->onDeviceChanged(e);
    }
}

void PedalCore::onDisconnect(const DisconnectEvent& e)
{
    partner_binding.onDisconnect(e);
    if (ui_event_sink) {
        ui_event_sink->onDisconnect(e);
    }
}

// ISendMidi
void PedalCore::sendControlChange(uint8_t channel, uint8_t cc, uint8_t value)
{
    auto partner = getPartner();
    if (partner && partner->ready()) {
        partner->sendControlChange(channel, cc, value);
    }
}

bool PedalCore::readyToSend()
{
    auto partner = getPartner();
    return partner && partner->readyToSend();
}

void PedalCore::syncAssign(Hc1Module * partner)
{
    auto pq = dynamic_cast<PedalParamQuantity*>(getParamQuantity(Params::P_PEDAL_ASSIGN));
    assert(pq);
    if (pq->syncValue()) {
        PedalInfo & pedal = partner->getPedal(pedal_id);
        pedal.cc = pq->last_cc;
        partner->notifyPedalChanged(pedal_id);
    }
}

void PedalCore::syncValue(Hc1Module * partner)
{
    auto pqv = getParamQuantity(Params::P_PEDAL_VALUE);
    auto value = static_cast<uint8_t>(std::round(pqv->getValue()));
    if (value != last_pedal_value) {
        last_pedal_value = value;
        if (!partner->readyToSend()) return;
        PedalInfo & pedal = partner->getPedal(pedal_id);
        partner->sendControlChange(0, pedal.cc, value);
    }

    auto pq = getParamQuantity(Params::P_PEDAL_MIN);
    value = static_cast<uint8_t>(std::round(pq->getValue()));
    if (value != last_pedal_min) {
        last_pedal_min = value;
        pqv->minValue = value;
        if (!partner->readyToSend()) return;
        PedalInfo & pedal = partner->getPedal(pedal_id);
        pedal.min = value;
        //partner->sendControlChange(EM_SettingsChannel, pedal_id ? EMCC_MinPedal2 : EMCC_MinPedal1, value);
        partner->sendControlChange(0, pedal_id ? EMCC_MinPedal2 : EMCC_MinPedal1, value);
    }

    pq = getParamQuantity(Params::P_PEDAL_MAX);
    value = static_cast<uint8_t>(std::round(pq->getValue()));
    if (value != last_pedal_max) {
        last_pedal_max = value;
        pqv->maxValue = value;
        if (!partner->readyToSend()) return;
        PedalInfo & pedal = partner->getPedal(pedal_id);
        pedal.max = value;
        //partner->sendControlChange(EM_SettingsChannel, pedal_id ? EMCC_MaxPedal2 : EMCC_MaxPedal1, value);
        partner->sendControlChange(0, pedal_id ? EMCC_MaxPedal2 : EMCC_MaxPedal1, value);
    }
}

void PedalCore::process(const ProcessArgs& args)
{
    auto partner = getPartner();
    if (!partner || !partner->readyToSend()) return;

    auto pedal = partner->getPedal(pedal_id);
    if (getOutput(Outputs::O_PEDAL_VALUE).isConnected()) {
        getOutput(Outputs::O_PEDAL_VALUE).setVoltage(10.f * pedal.value / 127);
    }

    if (0 == ((args.frame + id) % CV_INTERVAL)) {
        auto input = getInput(Inputs::I_PEDAL_VALUE);
        if (input.isConnected()) {
            float v = input.getVoltage();
            uint8_t value = static_cast<uint8_t>(std::round(v / 10.f * 127));
            if (value != last_pedal_value) {
                last_pedal_value = value;
                partner->sendControlChange(0, pedal.cc, value);
            }
        }
    }

    if (control_rate.process()) {
        syncAssign(partner);
        syncValue(partner);
    }
}


}

Model *modelPedal1 = createModel<pachde::Pedal1Module, pachde::Pedal1UI>("pachde-hc-pedal-1");
Model *modelPedal2 = createModel<pachde::Pedal2Module, pachde::Pedal2UI>("pachde-hc-pedal-2");