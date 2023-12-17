#pragma once
#ifndef POLYMIDI_HPP_INCLUDED
#define POLYMIDI_HPP_INCLUDED
#include "bend_param.hpp"
#include "../hc_events.hpp"
#include "../module_broker.hpp"
#include "../plugin.hpp"
#include "../widgets/label_widget.hpp"
#include "../widgets/partner_picker.hpp"
#include "../widgets/symbol_widget.hpp"
#include "mpe_burger.hpp"
namespace pachde {

using Symbol = SymbolWidget::Symbol;


struct PolyMidiModule : Module, IHandleHcEvents
{
    enum Params
    {
        P_POLY,
        P_EXPAND,
        P_COMPUTE,
        P_MPE,
        P_X_BEND,
        P_Y,
        P_Z,
        P_PRI,
        P_VELOCITY,
        P_ROUTE_SURFACE_MIDI,
        P_ROUTE_SURFACE_DSP,
        P_ROUTE_SURFACE_CVC,
        P_ROUTE_MIDI_MIDI,
        P_ROUTE_MIDI_DSP,
        P_ROUTE_MIDI_CVC,
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
        L_EXPAND,
        L_COMPUTE,
        L_ROUTE_SURFACE_MIDI,
        L_ROUTE_SURFACE_DSP,
        L_ROUTE_SURFACE_CVC,
        L_ROUTE_MIDI_MIDI,
        L_ROUTE_MIDI_DSP,
        L_ROUTE_MIDI_CVC,
        NUM_LIGHTS
    };

    PartnerBinding partner_binding;

    IHandleHcEvents * ui_event_sink = nullptr;
    const int CV_INTERVAL = 128;

    PolyMidiModule();
    virtual ~PolyMidiModule();
    Hc1Module * getPartner();

    uint8_t routing;
    Mpe mpe;
    Polyphony polyphony;
    NotePriority priority;
    VelocitySplit velocity;

    uint8_t getKnobRouting();
    void setRoutingLights();
    
    // IHandleHcEvents
    // void onPresetChanged(const PresetChangedEvent& e) override;
    // void onRoundingChanged(const RoundingChangedEvent& e) override;
    // void onPedalChanged(const PedalChangedEvent& e) override;
    void onRoutingChanged(const RoutingChangedEvent& e) override;
    void onPolyphonyChanged(const PolyphonyChangedEvent& e) override;
    void onNotePriorityChanged(const NotePriorityChangedEvent& e) override;
    void onMpeChanged(const MpeChangedEvent& e) override;
    void onVelocityChanged(const VelocityChangedEvent& e) override;
    void onDeviceChanged(const DeviceChangedEvent& e) override;
    void onDisconnect(const DisconnectEvent& e) override;
    // void onFavoritesFileChanged(const FavoritesFileChangedEvent& e) override;

    Hc1Module * processPolyphony(Hc1Module *partner);
    Hc1Module * processPriority(Hc1Module *partner);
    Hc1Module * processRouting(Hc1Module *partner);
    Hc1Module * processMpe(Hc1Module *partner);
    Hc1Module * processXBend(Hc1Module *partner);
    Hc1Module * processY(Hc1Module *partner);
    Hc1Module * processZ(Hc1Module *partner);
    Hc1Module * processVelocity(Hc1Module *partner);

    // Module
    json_t *dataToJson() override;
    void dataFromJson(json_t *root) override;
    void process(const ProcessArgs& args) override;
};

struct PolyMidiModuleWidget : ModuleWidget, IHandleHcEvents
{
    PolyMidiModule * my_module;
    PartnerPicker* partner_picker = nullptr;
    DynamicTextLabel* poly_text = nullptr;
    DynamicTextLabel* priority_text = nullptr;
    MpeBurger* midi_ui = nullptr;
    DynamicTextLabel* midi_text = nullptr;

    explicit PolyMidiModuleWidget(PolyMidiModule * module);

    virtual ~PolyMidiModuleWidget() {
        if (my_module) {
            my_module->ui_event_sink = nullptr;
        }
    }

    Hc1Module * getPartner();

    void createRouting();

    // IHandleHcEvents
    // void onPresetChanged(const PresetChangedEvent& e) override;
    // void onRoundingChanged(const RoundingChangedEvent& e) override;
    // void onFavoritesFileChanged(const FavoritesFileChangedEvent& e) override;
    // void onRoutingChanged(const RoutingChangedEvent& e) override;
    void onPolyphonyChanged(const PolyphonyChangedEvent& e) override;
    void onNotePriorityChanged(const NotePriorityChangedEvent& e) override;
    void onMpeChanged(const MpeChangedEvent& e) override;
    //void onVelocityChanged(const VelocityChangedEvent& e) override;
    void onDeviceChanged(const DeviceChangedEvent& e) override;
    void onDisconnect(const DisconnectEvent& e) override;

    void appendContextMenu(Menu *menu) override;
};



}
#endif