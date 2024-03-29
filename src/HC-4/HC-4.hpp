// Copyright (C) Paul Chase Dempsey
#pragma once
#ifndef HC_4_HPP_INCLUDED
#define HC_4_HPP_INCLUDED
#include "../hc_events.hpp"
#include "../module_broker.hpp"
#include "../plugin.hpp"
#include "../widgets/label_widget.hpp"
#include "../widgets/partner_picker.hpp"
#include "../widgets/symbol_widget.hpp"

namespace pachde {

using Symbol = SymbolWidget::Symbol;


struct Hc4Module : Module, IHandleHcEvents
{
    enum Params
    {
        NUM_PARAMS,
    };
    enum Inputs
    {
        NUM_INPUTS
    };
    enum Outputs
    {
        NUM_OUTPUTS
    };
    enum Lights
    {
        NUM_LIGHTS
    };

    PartnerBinding partner_binding;

    IHandleHcEvents * ui_event_sink = nullptr;
    const int CV_INTERVAL = 128;

    Hc4Module();
    virtual ~Hc4Module();
    Hc1Module * getPartner();

    // IHandleHcEvents
//    void onPresetChanged(const PresetChangedEvent& e) override;
//    void onRoundingChanged(const RoundingChangedEvent& e) override;
    void onPedalChanged(const PedalChangedEvent& e) override;
    void onDeviceChanged(const DeviceChangedEvent& e) override;
    void onDisconnect(const DisconnectEvent& e) override;
    //void onFavoritesFileChanged(const FavoritesFileChangedEvent& e) override;

    // Module
    json_t *dataToJson() override;
    void dataFromJson(json_t *root) override;
    void process(const ProcessArgs& args) override;
};

using Hc4O = Hc4Module::Outputs;

struct Hc4ModuleWidget : ModuleWidget, IHandleHcEvents
{
    Hc4Module * my_module;
    PartnerPicker* partner_picker = nullptr;

    explicit Hc4ModuleWidget(Hc4Module * module);

    virtual ~Hc4ModuleWidget() {
        if (my_module) {
            my_module->ui_event_sink = nullptr;
        }
    }

   Hc1Module * getPartner();

    // IHandleHcEvents
    // void onPresetChanged(const PresetChangedEvent& e) override;
    // void onRoundingChanged(const RoundingChangedEvent& e) override;
    void onDeviceChanged(const DeviceChangedEvent& e) override;
    void onDisconnect(const DisconnectEvent& e) override;
    // void onFavoritesFileChanged(const FavoritesFileChangedEvent& e) override;

    void appendContextMenu(Menu *menu) override;
};



}
#endif