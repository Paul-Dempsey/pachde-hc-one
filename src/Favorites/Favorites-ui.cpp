// Copyright (C) Paul Chase Dempsey
#include "Favorites.hpp"
#include "../misc.hpp"
#include "../module_broker.hpp"
#include "../widgets/small_push.hpp"
#include "preset_file_widget.hpp"

namespace pachde {

constexpr const float START_ROW = 47.5f;
constexpr const float ITEM_INTERVAL = 20.f;
constexpr const float DIVIDER_OFFSET = 5.f;

std::string favorites_sample_data[] = {
    "Experimental", "Strings", "Winds", "Guitars", "Pads", "Leads", "Album 4", "Album 5",
    "Mon practice", "", "Th Jam", "Blues setlist", "Recital 6/15", "Community Concert 8/21", "" ,"empty"
};

FavoritesModuleWidget::FavoritesModuleWidget(FavoritesModule* module)
:   my_module(module)
{
    setModule(module);
    if (module) {
        my_module->ui_event_sink = this;
    }
    setPanel(createPanel(asset::plugin(pluginInstance, "res/HC-3.svg")));
    addChild(partner_picker = createPartnerPicker());
    partner_picker->setFormat(TextFormatLength::Short);

    float y = START_ROW;
    float x = 15.f;
    for (auto i = 0; i < 16; ++i) {
        addChild(createPFWidget<PresetFileWidget>(Vec(x - 7.5, y - 7.5), module, i, &drawButton));
        addChild(createLightCentered<SmallLight<BlueLight>>(Vec(x,y), module, FavoritesModule::Lights::SETLIST + i));
        y += ITEM_INTERVAL;
        if (i == 7) {
            y += DIVIDER_OFFSET;
        }
    }
}

void FavoritesModuleWidget::onDeviceChanged(const DeviceChangedEvent& e)
{
    partner_picker->onDeviceChanged(e);
}

void FavoritesModuleWidget::onDisconnect(const DisconnectEvent& e)
{
    partner_picker->onDisconnect(e);
}

void FavoritesModuleWidget::onFavoritesFileChanged(const FavoritesFileChangedEvent& e)
{
    refreshDescriptions();
}

void FavoritesModuleWidget::step() 
{
    ModuleWidget::step();
    if (!module && !hacked_lights) {
        // Rack turns every light to full brightness in the module browser
        // we hack it here to show a more representative state.
        int n = 0;
        for (auto child: children) {
            auto light = dynamic_cast<BlueLight*>(child);
            if (light) {
                NVGcolor co = *light->baseColors.begin();
                co.a = n == CHOSEN_SAMPLE ? 1.f : favorites_sample_data[n].empty() ? 0.f : .3f;
                light->color = co;
                ++n;
            }
        }
        hacked_lights = true;
    }
}

void FavoritesModuleWidget::refreshDescriptions()
{
    for (auto child: children) {
        auto w = dynamic_cast<PresetFileWidget*>(child);
        if (w) {
            if (my_module) {
                w->describe(format_string("Open %s", my_module->files[w->getId()].c_str()));
            } else {
                w->describe("");
            }
        }
    }
}

void FavoritesModuleWidget::appendContextMenu(Menu *menu)
{
    if (!module) return;
    ///pick companion hc1

    menu->addChild(new MenuSeparator);

    auto count = std::count_if(my_module->files.cbegin(), my_module->files.cend(), [](const std::string& s){ return !s.empty(); });
    bool any = count > 0;
    menu->addChild(createMenuItem("Clear", "", [=](){ 
        my_module->clearFiles();
        my_module->setSynchronizedLoadedId(-1);
        refreshDescriptions();
    }));
    
    menu->addChild(createMenuItem("Sort", "", [=](){
        std::string selected = my_module->loaded_id >= 0 ? my_module->files[my_module->loaded_id] : "";
        std::sort(my_module->files.begin(), my_module->files.end(), alpha_order);
        if (my_module->loaded_id >= 0) {
            auto it = std::find_if(my_module->files.cbegin(), my_module->files.cend(), [&](std::string const& f)->bool { return f == selected; });
            assert(it != my_module->files.cend());
            my_module->setSynchronizedLoadedId(static_cast<int>(std::distance(my_module->files.cbegin(), it)));
        }
        refreshDescriptions();
    }, !any));

    menu->addChild(createMenuItem("Compact", "", [=](){
        int gap = 0;
        std::vector<std::string> items;
        items.reserve(16);
        int n = 0;
        for (auto s: my_module->files) {
            if (s.empty()) {
                ++gap;
            } else {
                items.push_back(s);
                if (n == my_module->loaded_id) {
                    my_module->setSynchronizedLoadedId(n - gap);
                }
            }
            ++n;
        }
        for (n = 0; n < gap; ++n) {
            items.push_back("");
        }
        my_module->files = items;
        refreshDescriptions();
    }, !any || 16 == count));

    menu->addChild(new MenuSeparator);
    my_module->partner_binding.appendContextMenu(menu);
    menu->addChild(createMenuItem("Clear HC-1 Favorite tab", "", [=]() {
            auto partner = my_module->getPartner();
            if (partner) {
                partner->clearFavorites();
                partner->openFavoritesFile("");
            }
    }));

}

}