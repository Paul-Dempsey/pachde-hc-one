#include "HC-1.hpp"
#include "../he_group.hpp"
namespace pachde {


// TODO:
//
// We have a weak compatibility scheme for all types of preset files.
//
// Need to track device class, firmware version, and device instance. The latter
// being the most difficult because we don't have unique device ids on all devices
// (Osmose, EMM) required to validate user presets/favorites.
//
// So at the moment we only validate firmware version.
// Newer firmware, or cross device favorites will require the ability to import
// and match strictly by preset name.
//
bool Hc1Module::requireFirmwareVersionMatch(const std::string &path, json_t* root)
{
    uint16_t ver = 1009;
    json_t* j = json_object_get(root, "firmware");
    if (j) {
        ver = json_integer_value(j);
    }
    if (ver != em.firmware_version) {
        WARN("%s: Loading %d presets on %d hardware", path, ver, em.firmware_version);
        return false;
    }
    return true;
}

void Hc1Module::tryCachedPresets()
{
    if (EM_Hardware::Unknown != em.hardware) {
        loadSystemPresets();
    }
    if (cache_user_presets && connection) {
        loadUserPresets();
    }
}

void Hc1Module::setPresetOrder(PresetOrder order)
{
    if (order != preset_order)
    {
        preset_order = order;
        std::sort(system_presets.begin(), system_presets.end(), getPresetSort(preset_order));
    }
}

std::string Hc1Module::startupConfigPath() {
    return asset::user(format_string("%s/startup-config.json", pluginInstance->slug.c_str()));    
}

std::string Hc1Module::userPresetsPath()
{
    if (!connection) return "";
    auto conn = to_file_safe(connection->info.spec(), false);
    return asset::user(format_string("%s/%s-user.json", pluginInstance->slug.c_str(), conn.c_str()));
}

std::string Hc1Module::systemPresetsResPath()
{
    if (EM_Hardware::Unknown == em.hardware) return "";
    return asset::plugin(pluginInstance, format_string("res/sys/%s-system.json", HardwarePresetClass(em.hardware)));
}

std::string Hc1Module::systemPresetsPath()
{
    if (EM_Hardware::Unknown == em.hardware) return "";
    return asset::user(format_string("%s/%s-system.json", pluginInstance->slug.c_str(), HardwarePresetClass(em.hardware)));
}

void Hc1Module::saveStartupConfig()
{
    auto path = startupConfigPath();
    if (system::exists(path)) return;

    auto dir = system::getDirectory(path);
    system::createDirectories(dir);

    std::string tmpPath = system::join(dir, TempName(".tmp.json"));
    FILE* file = std::fopen(tmpPath.c_str(), "w");
    if (!file) {
        system::remove(tmpPath);
        return;
    }

    auto root = json_object();
    if (!root) return;
	DEFER({json_decref(root);});

    PhasesToJson(root, init_phase);

	json_dumpf(root, file, JSON_INDENT(2));
	std::fclose(file);
    system::sleep(0.0005);
	system::remove(path);
    system::sleep(0.0005);
	system::rename(tmpPath, path);
}

void Hc1Module::loadStartupConfig()
{
    auto path = startupConfigPath();
    if (path.empty()) return;

    FILE* file = std::fopen(path.c_str(), "r");
	if (!file) {
		return;
    }
	DEFER({std::fclose(file);});

	json_error_t error;
	json_t* root = json_loadf(file, 0, &error);
	if (!root) {
		WARN("Invalid JSON at %d:%d %s in %s", error.line, error.column, error.text, path.c_str());
        return;
    }
	DEFER({json_decref(root);});

    PhasesFromJson(root, init_phase);
}

void Hc1Module::saveUserPresets()
{
    if (user_presets.empty()) return;
    if (!finished(InitPhase::UserPresets)) return;
    auto path = userPresetsPath();
    if (path.empty()) return;
    auto dir = system::getDirectory(path);
    system::createDirectories(dir);

    auto root = json_object();
    if (!root) return;
    DEFER({json_decref(root);});
    userPresetsToJson(root);

    std::string tmpPath = system::join(dir, TempName(".tmp.json"));
    FILE* file = std::fopen(tmpPath.c_str(), "w");
    if (!file) {
        system::remove(tmpPath);
        return;
    }

	json_dumpf(root, file, JSON_INDENT(2));
	std::fclose(file);
    system::sleep(0.0005);
	system::remove(path);
    system::sleep(0.0005);
	system::rename(tmpPath, path);
}

void Hc1Module::saveSystemPresets()
{
    if (system_presets.empty()) return;
    if (!finished(InitPhase::SystemPresets)) return;

    auto path = systemPresetsPath();
    if (path.empty()) return;
    auto dir = system::getDirectory(path);
    system::createDirectories(dir);

    auto root = json_object();
    if (!root) { return; }
	DEFER({json_decref(root);});
    systemPresetsToJson(root);

	std::string tmpPath = system::join(dir, TempName(".tmp.json"));
	FILE* file = std::fopen(tmpPath.c_str(), "w");
	if (!file) {
    	system::remove(tmpPath);
        return;
    }

	json_dumpf(root, file, JSON_INDENT(2));
	std::fclose(file);
	system::remove(path);
	system::rename(tmpPath, path);
}

void Hc1Module::savePresets()
{
    saveSystemPresets();
    saveUserPresets();
}

void Hc1Module::loadUserPresets()
{
    auto path = userPresetsPath();
    if (path.empty()) return;
    auto phase = get_phase(InitPhase::UserPresets);
    phase->pend();
    user_presets.clear();

    FILE* file = std::fopen(path.c_str(), "r");
	if (!file) {
        phase->fail();
		return;
    }
	DEFER({std::fclose(file);});
	json_error_t error;
	json_t* root = json_loadf(file, 0, &error);
	if (!root) {
        phase->fail();
		WARN("Invalid JSON at %d:%d %s in %s", error.line, error.column, error.text, path.c_str());
        return;
    }
	DEFER({json_decref(root);});
    if (!requireFirmwareVersionMatch(path, root)) {
        phase->fail();
        return;
    }

    auto jar = json_object_get(root, "user");
    if (jar) {
        json_t* jp;
        size_t index;
        json_array_foreach(jar, index, jp) {
            auto preset = std::make_shared<Preset>();
            preset->fromJson(jp);
            user_presets.push_back(preset);
        }
    }
    phase->finish();
}

void Hc1Module::loadSystemPresets()
{
    auto path = systemPresetsPath();
    if (path.empty()) return;
    if (!system::exists(path)) {
        path = systemPresetsResPath();
        if (path.empty()) return;
    }
    if (!system::exists(path)) {
        return;
    }
    auto phase = get_phase(InitPhase::SystemPresets);
    phase->pend();
    system_presets.clear();

    FILE* file = std::fopen(path.c_str(), "r");
	if (!file) {
        phase->fail();
		return;
    }
	DEFER({std::fclose(file);});

	json_error_t error;
	json_t* root = json_loadf(file, 0, &error);
	if (!root) {
        phase->fail();
		WARN("Invalid JSON at %d:%d %s in %s", error.line, error.column, error.text, path.c_str());
        return;
    }
	DEFER({json_decref(root);});
    if (!requireFirmwareVersionMatch(path, root)) {
        phase->fail();
        return;
    }

    auto jar = json_object_get(root, "system");
    if (jar) {
        json_t* jp;
        size_t index;
        json_array_foreach(jar, index, jp) {
            auto preset = std::make_shared<Preset>();
            preset->fromJson(jp);
            system_presets.push_back(preset);
        }
    }
    std::sort(system_presets.begin(), system_presets.end(), getPresetSort(preset_order));
    phase->finish();
}

void Hc1Module::userPresetsToJson(json_t* root)
{
    json_object_set_new(root, "connection", json_string(connection->info.spec().c_str()));
    json_object_set_new(root, "hardware", json_string(HardwarePresetClass(em.hardware)));
    json_object_set_new(root, "firmware", json_integer(em.firmware_version));

    auto jaru = json_array();
    for (auto preset: user_presets) {
        json_array_append_new(jaru, preset->toJson());
    }
    json_object_set_new(root, "user", jaru);
}

void Hc1Module::systemPresetsToJson(json_t* root)
{
    json_object_set_new(root, "hardware", json_string(HardwarePresetClass(em.hardware)));
    json_object_set_new(root, "firmware", json_integer(em.firmware_version));

    auto jars = json_array();
    for (auto preset: system_presets) {
        json_array_append_new(jars, preset->toJson());
    }
    json_object_set_new(root, "system", jars);
}

std::string Hc1Module::moduleFavoritesPath()
{
    if (!connection) return "";
    return asset::user(format_string("%s/%s-favorite.json", pluginInstance->slug.c_str(), HardwarePresetClass(em.hardware)));
}

void Hc1Module::clearFavorites()
{
    favorite_presets.clear();
    for (auto p: user_presets) {
        p->favorite = false;
    }
    for (auto p: system_presets) {
        p->favorite = false;
    }
}

json_t* Hc1Module::favoritesToJson()
{
    json_t* root = json_object();
    json_object_set_new(root, "connection", json_string(connection->info.spec().c_str()));
    json_object_set_new(root, "hardware", json_string(HardwarePresetClass(em.hardware)));
    json_object_set_new(root, "firmware", json_integer(em.firmware_version));
    auto ar = json_array();
    for (auto preset: system_presets) {
        if (preset->favorite) {
            json_array_append_new(ar, preset->toJson());
        }
    }
    for (auto preset: user_presets) {
        if (preset->favorite) {
            json_array_append_new(ar, preset->toJson());
        }
    }
    json_object_set_new(root, "favorites", ar);
    return root;
}

void Hc1Module::saveFavorites()
{
    if (!favoritesFile.empty()) {
        writeFavoritesFile(favoritesFile);
    }
    writeFavoritesFile(moduleFavoritesPath());
}

void Hc1Module::writeFavoritesFile(const std::string& path)
{
    if (path.empty()) return;
    auto dir = system::getDirectory(path);
    system::createDirectories(dir);

    auto root = favoritesToJson();
    if (!root) return;

	DEFER({json_decref(root);});
	std::string tmpPath = system::join(dir, TempName(".tmp.json"));
	FILE* file = std::fopen(tmpPath.c_str(), "w");
	if (!file) {
    	system::remove(tmpPath);
		return;
    }

	json_dumpf(root, file, JSON_INDENT(2));
	std::fclose(file);
	system::remove(path);
	system::rename(tmpPath, path);
}

bool Hc1Module::readFavoritesFile(const std::string& path, bool fresh)
{
    if (system_presets.empty()) return false;
    if (path.empty()) return false;

    { //scope for deferred file close
        FILE* file = std::fopen(path.c_str(), "r");
        if (!file) {
            return false;
        }
        DEFER({std::fclose(file);});
        json_error_t error;
        json_t* root = json_loadf(file, 0, &error);
        if (!root) {
            WARN("Invalid JSON at %d:%d %s in %s", error.line, error.column, error.text, path.c_str());
            return false;
        }
        DEFER({json_decref(root);});
        if (!requireFirmwareVersionMatch(path, root)) {
            return false;
        }
        auto bulk = BulkFavoritingMode(this);
        auto jar = json_object_get(root, "favorites");
        if (jar) {
            if (fresh) {
                clearFavorites();
            }
            json_t* jp;
            size_t index;
            Preset preset;
            json_array_foreach(jar, index, jp) {
                preset.fromJson(jp);
                if (!user_presets.empty()) {
                    auto it = std::find_if(user_presets.begin(), user_presets.end(), [&preset](std::shared_ptr<Preset>& p) { return p->is_same_preset(preset); });
                    if (it != user_presets.end()) {
                        addFavorite(*it);
                    }
                }
                if (!system_presets.empty()) {
                    auto it = std::find_if(system_presets.begin(), system_presets.end(), [&preset](std::shared_ptr<Preset>& p) { return p->is_same_preset(preset); });
                    if (it != system_presets.end()) {
                        addFavorite(*it);
                    }
                }
            }
        }
        sortFavorites();
    }
    saveFavorites();
    return true;
}

void Hc1Module::openFavoritesFile(const std::string& path)
{
    favoritesFile = path;
    if (!readFavoritesFile(favoritesFile, true)) {
        favoritesFile = "";
    }
    notifyFavoritesFileChanged();
}

void Hc1Module::importHEGroupFile(const std::string& path)
{
    auto items = he_group::ReadGroupFile(path);
    favoritesFile = "";
    clearFavorites();
    auto bf = BulkFavoritingMode(this);
    for (auto item : items) {
        auto p = findDefinedPresetByName(item.name);
        if (p) {
            addFavorite(p);
        }
    }
    notifyFavoritesFileChanged();
}

std::shared_ptr<Preset> Hc1Module::findDefinedPresetByName(std::string name)
{
    if (name.empty()) return nullptr;
    if (!user_presets.empty()) {
        auto it = std::find_if(user_presets.begin(), user_presets.end(), [name](const std::shared_ptr<Preset> & p) { return p->name == name; });
        if (it != user_presets.end()) {
            return *it;
        }
    }
    if (!system_presets.empty()) {
        auto it = std::find_if(system_presets.begin(), system_presets.end(),[name](const std::shared_ptr<Preset> & p) { return p->name == name; });
        if (it != system_presets.end()) {
            return *it;
        }
    }
    return nullptr;
}

std::shared_ptr<Preset> Hc1Module::findDefinedPreset(std::shared_ptr<Preset> preset)
{
    if (preset) {
        if (!user_presets.empty()) {
            auto it = std::find_if(user_presets.begin(), user_presets.end(), [preset](const std::shared_ptr<Preset> & p) { return p->is_same_preset(*preset); });
            if (it != user_presets.end()) {
                return *it;
            }
        }
        if (!system_presets.empty()) {
            auto it = std::find_if(system_presets.begin(), system_presets.end(), [preset](const std::shared_ptr<Preset> & p) { return p->is_same_preset(*preset); });
            if (it != system_presets.end()) {
                return *it;
            }
        }
    } else {
        if (!user_presets.empty()) {
            auto it = std::find_if(user_presets.begin(), user_presets.end(), [this](const std::shared_ptr<Preset>& p) { return p->is_same_preset(preset0); });
            if (it != user_presets.end()) {
                return *it;
            }
        }
        if (!system_presets.empty()) {
            auto it = std::find_if(system_presets.begin(), system_presets.end(), [this](const std::shared_ptr<Preset>& p) { return p->is_same_preset(preset0); });
            if (it != system_presets.end()) {
                return *it;
            }
        }
    }
    return nullptr;
}

void Hc1Module::sortFavorites(PresetOrder order)
{
    std::sort(favorite_presets.begin(), favorite_presets.end(), getPresetSort(order));
    numberFavorites();
}

void Hc1Module::numberFavorites()
{
    int n = 0;
    for (auto p: favorite_presets) {
        p->favorite_order = n++;
    }
}

void expandNumbers(std::vector<std::shared_ptr<pachde::Preset>>& presets, int spacing)
{
    int n = spacing;
    for (auto p: presets) {
        p->favorite_order = n;
        n += spacing;
    }
}

void Hc1Module::moveFavorite(std::shared_ptr<Preset> preset, IPresetHolder::FavoriteMove motion)
{
    if (!preset
        || !preset->favorite
        || !in_range(motion, IPresetHolder::FavoriteMove::First, IPresetHolder::FavoriteMove::Last)) {
        return;
    }
    expandNumbers(favorite_presets, 4);
    switch (motion) {
    case IPresetHolder::FavoriteMove::First:
        preset->favorite_order = 0;
        break;
    case IPresetHolder::FavoriteMove::Previous:
        preset->favorite_order = preset->favorite_order - 5;
        break;
    case IPresetHolder::FavoriteMove::Next:
        preset->favorite_order = preset->favorite_order + 5;
        break;
    case IPresetHolder::FavoriteMove::Last:
        preset->favorite_order = 5 + 4 * favorite_presets.size();
        break;
    default:
        break;
    }
    sortFavorites();
    saveFavorites();
}

void Hc1Module::addFavorite(std::shared_ptr<Preset> preset)
{
    preset->favorite = true;
    if (favorite_presets.empty()) {
        favorite_presets.push_back(preset);
    } else {
        if (favorite_presets.cend() == std::find_if(
                favorite_presets.cbegin(), 
                favorite_presets.cend(), 
                [preset](const std::shared_ptr<Preset>& fp){ 
                    return preset->is_same_preset(*fp); 
                })) {
            favorite_presets.push_back(preset);
            if (!bulk_favoriting) {
                sortFavorites();
            }
        } else {
            return; // already have it
        }
    }
    if (!bulk_favoriting) {
        saveFavorites();
    }
}

void Hc1Module::unFavorite(std::shared_ptr<Preset> preset)
{
    preset->favorite = false;
    if (favorite_presets.empty()) return;
    favorite_presets.erase(std::remove(favorite_presets.begin(), favorite_presets.end(), preset), favorite_presets.end());
    numberFavorites();
    saveFavorites();
}

}