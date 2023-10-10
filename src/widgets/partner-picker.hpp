#pragma once
#ifndef PARTNER_PICKER_HPP_INCLUDED
#define PARTNER_PICKER_HPP_INCLUDED
#include "../module_broker.hpp"
#include "label_widget.hpp"
#include "tip_widget.hpp"
namespace pachde {

struct PartnerPicker : TipWidget
{
    StaticTextLabel* device_label;
    PartnerBinding* binding;

    PartnerPicker()
    {
        box.size.x = 100.f;
        box.size.y = 12.f;
    }
    void setPartnerBinding(PartnerBinding* binder) { binding = binder; }

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
