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
            my_module->preset_order = order;
            std::sort(my_module->system_presets.begin(), my_module->system_presets.end(), getPresetSort(my_module->preset_order));
            if (PresetTab::System == tab) {
                populatePresetWidgets();
                showCurrentPreset(false);
            }
         }
    ));
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

    menu->addChild(createSubmenuItem("Favorites", "", [=](Menu* menu) {
        menu->addChild(createMenuItem("Clear favorites", "", [=](){ 
            my_module->clearFavorites();
            if (tab == PresetTab::Favorite) {
                updatePresetWidgets();
            }
        }, !ready));
        menu->addChild(createMenuItem("Open favorites...", "", [=]() {
            std::string path;
            std::string folder = asset::user(pluginInstance->slug);
            system::createDirectories(folder);
            if (openFileDialog(
                folder,
                "Favorites (.fav):fav;Json (.json):json;Any (*):*))",
                "",
                path)) {
                my_module->readFavoritesFile(path);
                updatePresetWidgets();
            }
            }, !ready));
        menu->addChild(createMenuItem("Save favorites as...", "", [=]() {
            std::string path;
            std::string folder = asset::user(pluginInstance->slug);
            system::createDirectories(folder);
            if (saveFileDialog(
                folder,
                "Favorites (.fav):fav;Json (.json):json;Any (*):*))",
                "my_favorites.fav",
                path)) {
                my_module->writeFavoritesFile(path);
            }
        }, !ready));
    }, !ready));

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

    menu->addChild(createSubmenuItem("Sort System presets", "", [=](Menu* menu) {
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

}