#include "HC-4.hpp"

namespace pachde {

Hc4Module::Hc4Module()
{
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);
    configOutput(Outputs::O_PEDAL1, "Pedal 1");
    configOutput(Outputs::O_PEDAL2, "Pedal 2");
}

Hc4Module::~Hc4Module()
{
    if (!partner_subscribed) return;
    auto partner = partner_binding.getPartner();
    if (partner){
        partner->unsubscribeHcEvents(this);
        partner_subscribed = false;
    }
}

Hc1Module* Hc4Module::getPartner()
{
    auto partner = partner_binding.getPartner();
    if (!partner) return nullptr;
    if (!partner_subscribed) {
        partner->subscribeHcEvents(this);
        partner_subscribed = true;
    }
    return partner;
}

void Hc4Module::onPedalChanged(const PedalChangedEvent& e)
{
    switch (e.pedal.jack) {
    case 0 : getOutput(O_PEDAL1).setVoltage(e.pedal.value * 10.f / 127.f); break;
    case 1 : getOutput(O_PEDAL2).setVoltage(e.pedal.value * 10.f / 127.f); break;
    default: break;
    }
}

void Hc4Module::onDeviceChanged(const DeviceChangedEvent& e)
{
    partner_binding.setDevice(e.name);
    if (ui_event_sink) {
        ui_event_sink->onDeviceChanged(e);
    }
}

void Hc4Module::onDisconnect(const DisconnectEvent& e)
{
    partner_subscribed = false;
    partner_binding.forgetModule();
    if (ui_event_sink) {
        ui_event_sink->onDisconnect(e);
    }
}

// void Hc4Module::onFavoritesFileChanged(const FavoritesFileChangedEvent& e)
// {
//     if (ui_event_sink) {
//         ui_event_sink->onFavoritesFileChanged(e);
//     }
// }

void Hc4Module::process(const ProcessArgs& args)
{
    if (++check_cv > CV_INTERVAL) {
        check_cv = 0;
        auto partner = getPartner();
        if (partner) {
            getOutput(Outputs::O_PEDAL1).setVoltage(10.f * partner->pedal1.value / 127);
            getOutput(Outputs::O_PEDAL2).setVoltage(10.f * partner->pedal2.value / 127);
        }
    }
}

}

Model *modelHc4 = createModel<pachde::Hc4Module, pachde::Hc4ModuleWidget>("pachde-hc-4");