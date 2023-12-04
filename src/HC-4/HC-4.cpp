#include "HC-4.hpp"

namespace pachde {

Hc4Module::Hc4Module()
{
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);
    partner_binding.setClient(this);
}

Hc4Module::~Hc4Module()
{
    partner_binding.unsubscribe();
}

json_t * Hc4Module::dataToJson()
{
    auto root = json_object();
    json_object_set_new(root, "device", json_string(partner_binding.claim.c_str()));
    return root;
}

void Hc4Module::dataFromJson(json_t *root)
{
    auto j = json_object_get(root, "device");
    if (j) {
        partner_binding.setClaim(json_string_value(j));
    }
    getPartner();
}

Hc1Module* Hc4Module::getPartner()
{
    return partner_binding.getPartner();
}

void Hc4Module::onPedalChanged(const PedalChangedEvent& e)
{

}

void Hc4Module::onDeviceChanged(const DeviceChangedEvent& e)
{
    partner_binding.onDeviceChanged(e);
    if (ui_event_sink) {
        ui_event_sink->onDeviceChanged(e);
    }
}

void Hc4Module::onDisconnect(const DisconnectEvent& e)
{
    partner_binding.onDisconnect(e);
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
    if (0 == ((args.frame + id) % CV_INTERVAL)) {
        auto partner = getPartner();
        if (partner) {
            // getOutput(Outputs::O_PEDAL1).setVoltage(10.f * partner->pedal1.value / 127);
            // getOutput(Outputs::O_PEDAL2).setVoltage(10.f * partner->pedal2.value / 127);
        }
    }
}

}

Model *modelHc4 = createModel<pachde::Hc4Module, pachde::Hc4ModuleWidget>("pachde-hc-4");