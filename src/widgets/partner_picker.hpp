#pragma once
#ifndef PARTNER_PICKER_HPP_INCLUDED
#define PARTNER_PICKER_HPP_INCLUDED
#include "../hc_events.hpp"
#include "../module_broker.hpp"
#include "label_widget.hpp"
#include "tip_widget.hpp"
namespace pachde {

struct PartnerPicker : TipWidget, IHandleHcEvents
{
    StaticTextLabel* device_label;
    PartnerBinding* binding;

    PartnerPicker()
    {
        box.size.x = 100.f;
        box.size.y = 12.f;
        addChild(device_label = createStaticTextLabel<StaticTextLabel>(
            Vec(0.f, 0.f), 100.f, "", TextAlignment::Left, 10.f, false, GetStockColor(StockColor::pachde_blue_medium)));
    }
    void setPartnerBinding(PartnerBinding* binder) { binding = binder; }

    void onDisconnect(const DisconnectEvent& e) override
    {
        device_label->text("");
        describe("(No associated HC-1)");
    }
    void onDeviceChanged(const DeviceChangedEvent& e) override
    {
        device_label->text(e.device ? e.device->info.friendly(false) : "");
        describe(e.device ? e.device->info.friendly(true) : "");
    }
    void step() override {
        device_label->box.size.x = box.size.x;
    }

    void appendContextMenu(Menu* menu) override {
        // enumerate registered modules
        
    }
};

template <typename PPW = PartnerPicker>
PPW* createPartnerPicker(float x, float y, float width, PartnerBinding* binding)
{
    PPW* w = createWidget<PPW>(Vec(x,y));
    w->setPartnerBinding(binding);
    w->box.size.x = width;
    return w;
}

}
#endif
