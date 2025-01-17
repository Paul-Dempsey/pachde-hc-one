// Copyright (C) Paul Chase Dempsey
#include "HC-1.hpp"
#include "../open_file.hpp"

namespace pachde {
    
void Hc1ModuleWidget::addSortBy(Menu *menu, std::string name, PresetOrder order)
{
    menu->addChild(createCheckMenuItem(name, "", 
         [=](){ return my_module->preset_order == order; },
         [=](){
            my_module->setPresetOrder(order);
            if (PresetTab::System == tab) {
                populatePresetWidgets();
                showCurrentPreset(false);
            }
         }
    ));
}

void Hc1ModuleWidget::addJumpCategory(Menu *menu, uint16_t category)
{
    auto title = format_string("%s (%s)", hcCategoryCode.categoryName(category).c_str(), CategoryCode(category).to_string().c_str());
    menu->addChild(createMenuItem(title, "", 
         [=](){ toCategory(category); }));
}

void Hc1ModuleWidget::addRecirculator(Menu *menu, EM_Recirculator kind)
{
    if (!my_module) { return; }
    menu->addChild(createCheckMenuItem(RecirculatorName(kind), "", 
         [=](){ return my_module->recirculatorType() == kind; },
         [=](){
            bool extend = my_module->em.recirculator.extended();
            my_module->em.recirculator.setKind(kind);
            my_module->em.recirculator.setExtended(extend);
            my_module->sendControlChange(EM_SettingsChannel, EMCC_RecirculatorType, my_module->em.recirculator.getValue());
         }
    ));
}

void Hc1ModuleWidget::addSystemMenu(Menu *menu)
{
    menu->addChild(createSubmenuItem("Go to category", "", [=](Menu* menu) {
        addJumpCategory(menu, ST);
        addJumpCategory(menu, WI);
        addJumpCategory(menu, VO);
        addJumpCategory(menu, KY);
        addJumpCategory(menu, CL);
        addJumpCategory(menu, OT);
        addJumpCategory(menu, PE);
        addJumpCategory(menu, PT);
        addJumpCategory(menu, PR);
        addJumpCategory(menu, DO);
        addJumpCategory(menu, MD);
        addJumpCategory(menu, CV);
        addJumpCategory(menu, UT);
    }));

    menu->addChild(createSubmenuItem("Sort", "", [=](Menu* menu) {
        addSortBy(menu, "Alphabetically", PresetOrder::Alpha);
        addSortBy(menu, "by Category", PresetOrder::Category);
        addSortBy(menu, "in System order", PresetOrder::System);
    }));

}

void Hc1ModuleWidget::addFavoritesMenu(Menu *menu)
{
    if (!my_module) { return; }
    bool unready = !my_module->ready();

    std::string filename = my_module->favoritesFile.empty() ? "(none)" : system::getFilename(my_module->favoritesFile);

    menu->addChild(createMenuItem("Favorite presets", "", [](){}, true));
    menu->addChild(new MenuSeparator);
    menu->addChild(createSubmenuItem("Sort", "", [=](Menu* menu) {
        menu->addChild(createMenuItem("Alphabetically", "", [=](){
            my_module->sortFavorites(PresetOrder::Alpha);
            my_module->saveFavorites();
            populatePresetWidgets();
            }, unready));
        menu->addChild(createMenuItem("by Category", "", [=](){
            my_module->sortFavorites(PresetOrder::Category);
            my_module->saveFavorites();
            populatePresetWidgets();
            }, unready));
        }, unready));

    menu->addChild(createMenuItem("Open...", "", [=]() {
        std::string path;
        std::string folder = asset::user(pluginInstance->slug);
        system::createDirectories(folder);
        if (openFileDialog(
            folder,
            "Favorites (.fav):fav;Json (.json):json;Any (*):*))",
            "",
            path)) {
            my_module->openFavoritesFile(path);
        }
        }, unready));
        
    menu->addChild(createMenuItem("Add from...", "", [=]() {
        std::string path;
        std::string folder = asset::user(pluginInstance->slug);
        system::createDirectories(folder);
        if (openFileDialog(
            folder,
            "Favorites (.fav):fav;Json (.json):json;Any (*):*))",
            "",
            path)) {
            my_module->readFavoritesFile(path, false);
            updatePresetWidgets();
        }
        }, unready));

    menu->addChild(createMenuItem("Import Haken Editor group list...", "", [=]() {
        std::string path;
        std::string folder = asset::user(pluginInstance->slug);
        system::createDirectories(folder);
        if (openFileDialog(
            folder,
            "Haken Editor group list (.txt):txt;Any (*):*.*",
            "",
            path)) {
            my_module->importHEGroupFile(path);
        }
        }, unready));
    menu->addChild(createMenuItem("Forget file and clear", "", [=]() {
        my_module->favoritesFile = "";
        my_module->clearFavorites();
        my_module->notifyFavoritesFileChanged();
    }));
    menu->addChild(new MenuSeparator);
    menu->addChild(createMenuLabel(format_string("File: %s", filename.c_str())));

    if (!my_module->favoritesFile.empty()) {
        menu->addChild(createMenuItem(format_string("Forget %s", filename.c_str()), "", [=]() {
            my_module->favoritesFile = "";
            }, unready));
    }

    std::string prompt = my_module->favoritesFile.empty() ? "Save as..." : format_string("Save %s as...", filename.c_str());
    menu->addChild(createMenuItem(prompt, "", [=]() {
        std::string path;
        std::string folder = asset::user(pluginInstance->slug);
        system::createDirectories(folder);
        if (saveFileDialog(
            folder,
            "Favorites (.fav):fav;Json (.json):json;Any (*):*))",
            filename.empty() ? "my_favorites.fav" : filename,
            path)) {
            my_module->writeFavoritesFile(path);
            my_module->favoritesFile = path;
            my_module->notifyFavoritesFileChanged();
        }
    }, unready));

    menu->addChild(createMenuItem("Save copy as...", "", [=]() {
        std::string path;
        std::string folder = asset::user(pluginInstance->slug);
        system::createDirectories(folder);
        std::string name = system::getStem(my_module->favoritesFile) + " Copy.fav";
        if (saveFileDialog(
            folder,
            "Favorites (.fav):fav;Json (.json):json;Any (*):*))",
            name,
            path)) {
            my_module->writeFavoritesFile(path);
        }
    }, unready || my_module->favoritesFile.empty()));

    prompt = my_module->favoritesFile.empty() ? "Clear" : format_string("Clear %s", filename.c_str());
    menu->addChild(createMenuItem(prompt, "", [=](){ 
        my_module->clearFavorites();
        my_module->saveFavorites();
        if (tab == PresetTab::Favorite) {
            updatePresetWidgets();
        }
    }, unready || my_module->favorite_presets.empty()));

}

void Hc1ModuleWidget::appendContextMenu(Menu *menu)
{
    if (!my_module) { return; }
    bool ready = my_module->ready();
    //menu->addChild(createMenuItem("", "", [this](){}));

    menu->addChild(new MenuSeparator);
    menu->addChild(createSubmenuItem("Presets", "", [=](Menu* menu) {
        menu->addChild(createCheckMenuItem("Restore last preset on startup", "", 
            [=](){ return my_module->restore_saved_preset; },
            [=](){ my_module->restore_saved_preset = !my_module->restore_saved_preset; }
            ));
        menu->addChild(createCheckMenuItem("Use saved user presets", "",
            [=](){ return my_module->cache_user_presets; },
            [=](){
                my_module->cache_user_presets = !my_module->cache_user_presets;
                if (my_module->cache_user_presets) {
                    my_module->saveUserPresets();
                }
            }));
        menu->addChild(new MenuSeparator);
        menu->addChild(createMenuItem("Refresh current preset", "",  [=](){ 
            my_module->fresh_phase(InitPhase::PresetConfig);
            my_module->current_phase = InitPhase::PresetConfig;
       }));
        menu->addChild(createMenuItem("Refresh User presets", "", [=](){ 
            my_module->fresh_phase(InitPhase::UserPresets);
            my_module->current_phase = InitPhase::UserPresets;
        }));
        menu->addChild(createMenuItem("Refresh System presets", "", [=](){ 
            my_module->fresh_phase(InitPhase::SystemPresets);
            my_module->current_phase = InitPhase::SystemPresets;
        }));
    }));

    menu->addChild(createSubmenuItem("Knob control", "", [=](Menu* menu) {
        menu->addChild(createMenuItem("Center knobs", "", [=](){ my_module->centerKnobs(); }, !ready));
        menu->addChild(createMenuItem("Center Macro knobs", "", [=](){ my_module->centerMacroKnobs(); }, !ready));
        menu->addChild(createMenuItem("Zero knobs", "", [=](){ my_module->zeroKnobs(); }, !ready));
        menu->addChild(createMenuItem("Absolute CV", "", [=](){ my_module->absoluteCV(); }, !ready));
        menu->addChild(createMenuItem("Relative CV", "", [=](){ my_module->relativeCV(); }, !ready));
    }));

    menu->addChild(createSubmenuItem("Recirculator", "", [=](Menu * menu) {
        addRecirculator(menu, EM_Recirculator::Reverb);
        addRecirculator(menu, EM_Recirculator::ModDelay);
        addRecirculator(menu, EM_Recirculator::SweptEcho);
        addRecirculator(menu, EM_Recirculator::AnalogEcho);
        addRecirculator(menu, EM_Recirculator::DigitalEchoLPF);
        addRecirculator(menu, EM_Recirculator::DigitalEchoHPF);
    }));

    menu->addChild(createSubmenuItem("Utility", "", [=](Menu* menu) {
        menu->addChild(createMenuItem("Reboot HC-1", "", [=](){ my_module->reboot(); }));
        menu->addChild(createCheckMenuItem(
            "Reverse surface", "", [=](){ return my_module->em.reverse_surface; },
            [=](){ my_module->sendSurfaceDirection(!my_module->em.reverse_surface); },
            !ready
        ));
        menu->addChild(createMenuItem("Quiesce", "", [=](){ my_module->silence(true); }));
        menu->addChild(createMenuItem("Send one handshake (ping)", "", [=](){ 
            my_module->fresh_phase(InitPhase::Heartbeat);
            my_module->current_phase = InitPhase::Heartbeat;
        }));

        menu->addChild(createMenuItem("Remake QSPI Data", "", [=]() {
            my_module->sendControlChange(EM_SettingsChannel, EMCC_Download, EM_DownloadItem::remakeSRMahl); }));
        }));

    menu->addChild(createMenuItem("Refresh current preset info", "", [=](){ 
        my_module->fresh_phase(InitPhase::PresetConfig);
        my_module->current_phase = InitPhase::PresetConfig;
    }));
}

}