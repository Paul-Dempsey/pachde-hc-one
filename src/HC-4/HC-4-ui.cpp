#include "HC-4.hpp"
#include "../widgets/port.hpp"

namespace pachde {

constexpr const float ROW_1 = 50.f; // center
constexpr const float COL_1 = 75.f; //center
constexpr const float OUTPUT_SPREAD = 18.f;
constexpr const float ROW_SPREAD = 20.f;
constexpr const float LABEL_VOFFSET = 6.f;
constexpr const float LABEL_WIDTH = 52.f;

Hc4ModuleWidget::Hc4ModuleWidget(Hc4Module * module)
: my_module(module)
{
    setModule(module);
    if (module) {
        my_module->ui_event_sink = this;
    }
    setPanel(createPanel(asset::plugin(pluginInstance, "res/HC-4.svg")));
    addChild(partner_picker = createPartnerPicker(7.f, 14.f, 180.f, module ? &module->partner_binding : nullptr));
}

Hc1Module* Hc4ModuleWidget::getPartner()
{
    if (!module) return nullptr;
    return my_module->getPartner();
}

void Hc4ModuleWidget::onDeviceChanged(const DeviceChangedEvent& e)
{
    partner_picker->onDeviceChanged(e);
}

void Hc4ModuleWidget::onDisconnect(const DisconnectEvent& e)
{
    partner_picker->onDisconnect(e);
}


}