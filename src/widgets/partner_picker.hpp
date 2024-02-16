// Copyright (C) Paul Chase Dempsey
#pragma once
#ifndef PARTNER_PICKER_HPP_INCLUDED
#define PARTNER_PICKER_HPP_INCLUDED
#include "../common_layout.hpp"
#include "../hc_events.hpp"
#include "../module_broker.hpp"
#include "label_widget.hpp"
#include "tip_widget.hpp"
namespace pachde {

static const char * const LABEL_NONE = "none";
static const char * const DESCRIBE_NONE = "No associated HC-1 available";

struct PartnerPicker : TipWidget, IHandleHcEvents
{
    StaticTextLabel* device_label;
    TextFormatLength label_size;

    PartnerPicker()
    : label_size(TextFormatLength::Compact)
    {
        box.size.x = 100.f;
        box.size.y = 12.f;
        addChild(device_label = createStaticTextLabel<StaticTextLabel>(
            Vec(0.f, 0.f), 100.f, LABEL_NONE, TextAlignment::Left, 10.f, false, GetStockColor(StockColor::pachde_blue_light)));
        describe(DESCRIBE_NONE);
    }
    void setFormat(TextFormatLength format)
    {
        label_size = format;
    }

    void onDisconnect(const DisconnectEvent& e) override
    {
        device_label->text(LABEL_NONE);
        describe(DESCRIBE_NONE);
    }
    void onDeviceChanged(const DeviceChangedEvent& e) override
    {
        device_label->text(e.device ? e.device->info.friendly(label_size) : LABEL_NONE);
        describe(e.device ? e.device->info.friendly(TextFormatLength::Long) : DESCRIBE_NONE);
    }
    void step() override {
        device_label->box.size.x = box.size.x;
    }
};

template <typename PPW = PartnerPicker>
PPW* createPartnerPicker(float x = PARTNER_LEFT, float y = PARTNER_TOP, float width = PARTNER_WIDTH)
{
    PPW* w = createWidget<PPW>(Vec(x,y));
    w->box.size.x = width;
    return w;
}

}
#endif
