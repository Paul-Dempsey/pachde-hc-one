// Copyright (C) Paul Chase Dempsey
#include "HC-4.hpp"
#include "../widgets/port.hpp"

namespace pachde {

Hc4ModuleWidget::Hc4ModuleWidget(Hc4Module * module)
: my_module(module)
{
    setModule(module);
    if (module) {
        my_module->ui_event_sink = this;
    }
    setPanel(createPanel(asset::plugin(pluginInstance, "res/HC-4.svg")));
    addChild(partner_picker = createPartnerPicker());
}

Hc1Module* Hc4ModuleWidget::getPartner()
{
    return module ? my_module->getPartner() : nullptr;
}

void Hc4ModuleWidget::onDeviceChanged(const DeviceChangedEvent& e)
{
    partner_picker->onDeviceChanged(e);
}

void Hc4ModuleWidget::onDisconnect(const DisconnectEvent& e)
{
    partner_picker->onDisconnect(e);
}

void Hc4ModuleWidget::appendContextMenu(Menu *menu)
{
    if (!my_module) return;
    menu->addChild(new MenuSeparator);
    my_module->partner_binding.appendContextMenu(menu);
}

}