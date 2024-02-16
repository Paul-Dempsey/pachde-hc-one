// Copyright (C) Paul Chase Dempsey
#pragma once
#ifndef Favorites_HPP_INCLUDED
#define Favorites_HPP_INCLUDED
#include <stdint.h>
#include "../HC-1/HC-1.hpp"
#include "../hc_events.hpp"
#include "../module_broker.hpp"
#include "../plugin.hpp"
#include "../presets.hpp"
#include "../widgets/label_widget.hpp"
#include "../widgets/partner_picker.hpp"
#include "../widgets/square_button.hpp"

namespace pachde {

extern std::string favorites_sample_data[];
constexpr const int CHOSEN_SAMPLE = 11;

struct FavoritesModule : Module, IHandleHcEvents
{
    enum Params {
        SELECTED_PARAM,
        NUM_PARAMS
    };
    enum Inputs{NUM_INPUTS};
    enum Outputs{NUM_OUTPUTS};
    enum Lights
    {
        ENUMS(SETLIST, 16),
        NUM_LIGHTS
    };

    int loaded_id;
    std::vector<std::string> files;
    PartnerBinding partner_binding;
    IHandleHcEvents * ui_event_sink = nullptr;
    std::string favorites_folder;

    const float POLL_RATE = 1.5f;
    rack::dsp::Timer poll_timer;

    FavoritesModule();
    virtual ~FavoritesModule();
    void clearFiles();

    Hc1Module* getPartner();

    void openFile(int id);
    void setSynchronizedLoadedId(int id);
    void useCurrentFavoriteFile(int id);

    // IHandleHcEvents
    // void onPresetChanged(const PresetChangedEvent& e) override {}
    // void onRoundingChanged(const RoundingChangedEvent& e) override {}
    void onDeviceChanged(const DeviceChangedEvent& e) override;
    void onDisconnect(const DisconnectEvent& e) override;
    void onFavoritesFileChanged(const FavoritesFileChangedEvent& e) override;

    void onRandomize() override;
    json_t* dataToJson() override;
    void dataFromJson(json_t* root) override;
    void onReset() override;
    void process(const ProcessArgs& args) override;
};

struct FavoritesModuleWidget : ModuleWidget, IHandleHcEvents
{
    FavoritesModule* my_module;
    DrawSquareButton drawButton;
//    StaticTextLabel* device_label = nullptr;
    PartnerPicker* partner_picker = nullptr;
    bool hacked_lights = false;

    explicit FavoritesModuleWidget(FavoritesModule* module);

    void refreshDescriptions();

    // IHandleHcEvents
    // void onPresetChanged(const PresetChangedEvent& e) override {}
    // void onRoundingChanged(const RoundingChangedEvent& e) override {}
    void onDeviceChanged(const DeviceChangedEvent& e) override;
    void onDisconnect(const DisconnectEvent& e) override;
    void onFavoritesFileChanged(const FavoritesFileChangedEvent& e) override;

    void step() override;
    void appendContextMenu(Menu *menu) override;

};

#endif
}