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

    // device name in title
    device_label = createStaticTextLabel<StaticTextLabel>(
        Vec(7.f, 21.f), 180.f, "", TextAlignment::Left, 12.f, false, GetStockColor(StockColor::pachde_blue_medium));
    addChild(device_label);
}

Hc1Module* Hc4ModuleWidget::getPartner()
{
    if (!module) return nullptr;
    return my_module->getPartner();
}

void Hc4ModuleWidget::onDisconnect(const DisconnectEvent& e)
{
    device_label->text("");
}

void Hc4ModuleWidget::onDeviceChanged(const DeviceChangedEvent& e)
{
    device_label->text(e.name);
}


}