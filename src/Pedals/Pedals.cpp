#include "Pedals.hpp"
#include "../widgets/pedal_param.hpp"
namespace pachde {

PedalCore::PedalCore(uint8_t pedal)
: pedal_id(pedal)
{
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);

    configPedalParam(pedal_id, PedalAssign::Sustain, this, Params::P_PEDAL_ASSIGN);

    auto p = configParam(P_PEDAL_VALUE, 0.f, 127.f, 0.f, format_string("Pedal %d value", pedal_id));
    p->snapEnabled = true;

    configInput(Inputs::I_PEDAL_VALUE, "Pedal");
    configOutput(Outputs::O_PEDAL_VALUE, "Pedal");
}

PedalCore::~PedalCore()
{
    if (!partner_subscribed) return;
    auto partner = partner_binding.getPartner();
    if (partner){
        partner->unsubscribeHcEvents(this);
        partner_subscribed = false;
    }
}

Hc1Module* PedalCore::getPartner()
{
    auto partner = partner_binding.getPartner();
    if (partner) {
        if (!partner_subscribed) {
            partner->subscribeHcEvents(this);
            partner_subscribed = true;
        }
    }
    return partner;
}

// IHandleHcEvents
void PedalCore::onPedalChanged(const PedalChangedEvent& e)
{
    if (e.pedal.jack != pedal_id) return;
    static_cast<PedalParamQuantity*>(getParamQuantity(Params::P_PEDAL_ASSIGN))
        ->setAssignSilent(PedalAssignFromCC(e.pedal.cc));

    last_pedal_value = e.pedal.value;
    getParamQuantity(Params::P_PEDAL_VALUE)->setValue(e.pedal.value);

    if (ui_event_sink) {
        ui_event_sink->onPedalChanged(e);
    }
}

void PedalCore::onDeviceChanged(const DeviceChangedEvent& e)
{
    partner_binding.setDevice(e.name);
    if (ui_event_sink) {
        ui_event_sink->onDeviceChanged(e);
    }
}

void PedalCore::onDisconnect(const DisconnectEvent& e)
{
    partner_subscribed = false;
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
        // if (ui_event_sink) {
        //     ui_event_sink->onPedalChanged(PedalChangedEvent{pedal});
        // }
    }
}

void PedalCore::syncValue(Hc1Module * partner)
{
    auto pq = getParamQuantity(Params::P_PEDAL_VALUE);
    auto value = static_cast<uint8_t>(pq->getValue());
    if (value != last_pedal_value) {
        last_pedal_value = value;
        PedalInfo & pedal = partner->getPedal(pedal_id);
        partner->sendControlChange(0, pedal.cc, value);
    }
}

void PedalCore::process(const ProcessArgs& args)
{
    auto partner = getPartner();
    if (!partner) return;

    auto pedal = partner->getPedal(pedal_id);
    getOutput(Outputs::O_PEDAL_VALUE).setVoltage(10.f * pedal.value / 127);

    if (!partner->readyToSend()) return;

    if (++check_cv > CV_INTERVAL) {
        check_cv = 0;
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

Model *modelPedal1 = createModel<pachde::Pedal1Module, pachde::Pedal1UI>("pachde-pedal1");
Model *modelPedal2 = createModel<pachde::Pedal2Module, pachde::Pedal2UI>("pachde-pedal2");