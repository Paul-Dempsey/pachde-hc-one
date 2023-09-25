#include "HC-1.hpp"
#include "../open_file.hpp"

namespace pachde {
    
// void AddCategoryFilterItem(Menu *menu, Hc1Module * my_module, std::string name, uint16_t code)
// {
//     menu->addChild(createCheckMenuItem(name, "", 
//         [=](){ return !my_module->preset_filter.isFiltered() || my_module->preset_filter.isShow(ST); },
//         [=](){ my_module->preset_filter.toggleShow(ST); }
//     ));
// }

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
    menu->addChild(createCheckMenuItem(RecirculatorName(kind), "", 
         [=](){ return my_module->recirculatorType() == kind; },
         [=](){
            my_module->recirculator =  kind | EM_Recirculator::Extend;
            my_module->sendControlChange(EM_SettingsChannel, EMCC_RecirculatorType, my_module->recirculator);
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

    // menu->addChild(createSubmenuItem("Filter by Category", "", [=](Menu* menu) {
    //     AddCategoryFilterItem(menu, my_module, "Strings", ST);
    //     AddCategoryFilterItem(menu, my_module, "Winds", WI);
    //     AddCategoryFilterItem(menu, my_module, "Vocal", VO);
    //     AddCategoryFilterItem(menu, my_module, "Keyboard", KY);
    //     AddCategoryFilterItem(menu, my_module, "Classic", CL);
    //     AddCategoryFilterItem(menu, my_module, "Other", OT);
    //     AddCategoryFilterItem(menu, my_module, "Percussion", PE);
    //     AddCategoryFilterItem(menu, my_module, "Tuned Percussion",PT);
    //     AddCategoryFilterItem(menu, my_module, "Processor", PR);
    //     AddCategoryFilterItem(menu, my_module, "Drone", DO);
    //     AddCategoryFilterItem(menu, my_module, "Midi", MD);
    //     AddCategoryFilterItem(menu, my_module, "Control Voltage",CV);
    //     AddCategoryFilterItem(menu, my_module, "Utility", UT);
    //     }));
}

void Hc1ModuleWidget::addFavoritesMenu(Menu *menu)
{
    bool ready = my_module->ready();

    std::string filename = my_module->favoritesFile.empty() ? "(none)" : system::getFilename(my_module->favoritesFile);

    menu->addChild(new MenuSeparator);
    menu->addChild(createMenuItem(format_string("File: %s", filename.c_str()), "", [=](){ }, true));

    menu->addChild(new MenuSeparator);
    menu->addChild(createMenuItem("Clear", "", [=](){ 
        my_module->clearFavorites();
        my_module->saveFavorites();
        if (tab == PresetTab::Favorite) {
            updatePresetWidgets();
        }
    }, !ready));

    if (!my_module->favoritesFile.empty()) {
        menu->addChild(createMenuItem(format_string("Forget %s", filename.c_str()), "", [=]() {
            my_module->favoritesFile = "";
            }, !ready));
    }

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
        }, !ready));

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
        }, !ready));

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
    }, !ready));
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
    }, !ready || my_module->favoritesFile.empty()));

}

void Hc1ModuleWidget::appendContextMenu(Menu *menu)
{
    if (!my_module) { return; }
    //menu->addChild(createMenuItem("", "", [this](){}));
    bool ready = my_module->ready();

    menu->addChild(new MenuSeparator);
    menu->addChild(createSubmenuItem("Knob control", "", [=](Menu* menu) {
        menu->addChild(createMenuItem("Center knobs", "", [=](){ my_module->centerKnobs(); }, !ready));
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

    menu->addChild(createSubmenuItem("Module", "", [=](Menu* menu) {
        menu->addChild(createMenuItem("Reboot HC-1", "",     [=](){ my_module->reboot(); }));
        menu->addChild(createCheckMenuItem("Suppress heartbeat handshake", "",
            [=](){ return !my_module->heart_beating; },
            [=](){ my_module->heart_beating = !my_module->heart_beating; }));
        menu->addChild(createMenuItem("One handshake", "",   [=](){ my_module->sendEditorPresent(); }));
        menu->addChild(createMenuItem("Request config", "",  [=](){ my_module->transmitRequestConfiguration(); }));
        menu->addChild(createMenuItem("Reset Midi I/O", "",  [=]() { my_module->resetMidiIO(); }));
    }));

    // now right click on favorites tab
    // menu->addChild(createSubmenuItem("Favorites", "", [=](Menu* menu) {
    //     addFavoritesMenu(menu);
    // }));

    menu->addChild(createSubmenuItem("Presets", "", [=](Menu* menu) {
        menu->addChild(createCheckMenuItem("Restore last preset on startup", "", 
            [=](){ return my_module->restore_saved_preset; },
            [=](){ my_module->restore_saved_preset = !my_module->restore_saved_preset; }
            ));
        menu->addChild(createCheckMenuItem("Use saved presets", "",
            [=](){ return my_module->cache_presets; },
            [=](){
                my_module->cache_presets = !my_module->cache_presets;
                if (my_module->cache_presets) {
                    my_module->savePresets();
                }
            }));
        menu->addChild(new MenuSeparator);
        menu->addChild(createMenuItem("Save presets", "", [=](){ my_module->savePresets(); }, !ready));
        menu->addChild(createMenuItem("Refresh User presets", "", [=](){ my_module->transmitRequestUserPresets(); }));
    }));

    // now right click on system tab
    // addSystemMenu(menu);
}

}