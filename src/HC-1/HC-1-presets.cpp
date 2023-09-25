#include "HC-1.hpp"
namespace pachde {

void Hc1Module::tryCachedPresets() {
    if (cache_presets) {
        loadSystemPresets();
        if (system_presets.empty()) {
            system_preset_state = InitState::Uninitialized;
        }
        loadUserPresets();
        if (user_presets.empty()) {
            user_preset_state = InitState::Uninitialized;
        }
        if (favoritesFile.empty()) {
            favoritesFromPresets();
        }
    }
    if (!favoritesFile.empty()) {
        if (system_presets.empty() || user_presets.empty()) {
            apply_favorite_state = InitState::Uninitialized;
        } else {
            if (readFavoritesFile(favoritesFile, true)) {
                apply_favorite_state = InitState::Complete;
            } else {
                apply_favorite_state = InitState::Broken;
            }
        }
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

std::string Hc1Module::userPresetsPath()
{
    if (deviceName().empty()) return "";
    return asset::user(format_string("%s/%s-user.json", pluginInstance->slug.c_str(), deviceName().c_str()));
}

std::string Hc1Module::systemPresetsPath()
{
    if (deviceName().empty()) return "";
    return asset::user(format_string("%s/%s-system.json", pluginInstance->slug.c_str(), deviceName().c_str()));
}

void Hc1Module::saveUserPresets()
{
    auto path = userPresetsPath();
    DebugLog("Saving user presets: %s ", path.c_str());
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
		return;
    }

	json_dumpf(root, file, JSON_INDENT(2));
	std::fclose(file);
	system::remove(path);
	system::rename(tmpPath, path);
}

void Hc1Module::saveSystemPresets()
{
    auto path = systemPresetsPath();
    DebugLog("Saving system presets: %s ", path.c_str());
    if (path.empty()) return;
    auto dir = system::getDirectory(path);
    system::createDirectories(dir);

    auto root = json_object();
    if (!root) { return; }
	DEFER({json_decref(root);});
    systemPresetsToJson(root);

	std::string tmpPath = system::join(dir, TempName(".tmp.json"));
	FILE* file = std::fopen(tmpPath.c_str(), "w");
	if (!file) { return; }

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
    user_preset_state = InitState::Pending;
    user_presets.clear();

    FILE* file = std::fopen(path.c_str(), "r");
	if (!file) {
        user_preset_state = InitState::Broken;
		return;
    }
	DEFER({std::fclose(file);});
	json_error_t error;
	json_t* root = json_loadf(file, 0, &error);
	if (!root) {
        user_preset_state = InitState::Broken;
		DebugLog("Invalid JSON at %d:%d %s in %s", error.line, error.column, error.text, path.c_str());
        return;
    }
	DEFER({json_decref(root);});
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
    user_preset_state = InitState::Complete;
}

void Hc1Module::loadSystemPresets()
{
    auto path = systemPresetsPath();
    if (path.empty()) return;
    
    system_preset_state = InitState::Pending;

    system_presets.clear();

    FILE* file = std::fopen(path.c_str(), "r");
	if (!file) {
        system_preset_state = user_preset_state = InitState::Broken;
		return;
    }
	DEFER({std::fclose(file);});

	json_error_t error;
	json_t* root = json_loadf(file, 0, &error);
	if (!root) {
        system_preset_state = user_preset_state = InitState::Broken;
		DebugLog("Invalid JSON at %d:%d %s in %s", error.line, error.column, error.text, path.c_str());
        return;
    }
	DEFER({json_decref(root);});
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
    system_preset_state = InitState::Complete;
}

void Hc1Module::favoritesFromPresets()
{
    favorite_presets.clear();
    for (auto p: user_presets) {
        if (p->favorite) {
            favorite_presets.push_back(p);
        }
    }
    for (auto p: system_presets) {
        if (p->favorite) {
            favorite_presets.push_back(p);
        }
    }
    sortFavorites();
    apply_favorite_state = InitState::Complete;
}

void Hc1Module::userPresetsToJson(json_t* root)
{
    auto device = deviceName();
    json_object_set_new(root, "device", json_stringn(deviceName().c_str(), deviceName().size()));

    auto jaru = json_array();
    for (auto preset: user_presets) {
        json_array_append_new(jaru, preset->toJson());
    }
    json_object_set_new(root, "user", jaru);
}

void Hc1Module::systemPresetsToJson(json_t* root)
{
    auto device = deviceName();
    json_object_set_new(root, "device", json_stringn(deviceName().c_str(), deviceName().size()));

    auto jars = json_array();
    for (auto preset: system_presets) {
        json_array_append_new(jars, preset->toJson());
    }
    json_object_set_new(root, "system", jars);
}


std::string Hc1Module::moduleFavoritesPath()
{
    if (broken || !is_eagan_matrix) return "";
    return asset::user(format_string("%s/fav-%s.json", pluginInstance->slug.c_str(), deviceName().c_str()));
}

void Hc1Module::clearFavorites()
{
    favorite_presets.clear();
    for (auto p: user_presets) {
        if (p->favorite) p->favorite = false;
    }
    for (auto p: system_presets) {
        if (p->favorite) p->favorite = false;
    }
}

json_t* Hc1Module::favoritesToJson()
{
    json_t* root = json_object();
    auto device = FilterDeviceName(deviceName());
    json_object_set_new(root, "device", json_stringn(deviceName().c_str(), deviceName().size()));
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
    FILE* file = std::fopen(path.c_str(), "r");
	if (!file) {
		return false;
    }
	DEFER({std::fclose(file);});
	json_error_t error;
	json_t* root = json_loadf(file, 0, &error);
	if (!root) {
		DebugLog("Invalid JSON at %d:%d %s in %s", error.line, error.column, error.text, path.c_str());
        return false;
    }
	DEFER({json_decref(root);});
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

std::shared_ptr<Preset> Hc1Module::findDefinedPreset(std::shared_ptr<Preset> preset)
{
    if (preset) {
        if (!user_presets.empty()) {
            auto it = std::find_if(user_presets.begin(), user_presets.end(), [preset](std::shared_ptr<Preset>& p) { return p->is_same_preset(*preset); });
            if (it != user_presets.end()) {
                return *it;
            }
        }
        if (!system_presets.empty()) {
            auto it = std::find_if(system_presets.begin(), system_presets.end(), [preset](std::shared_ptr<Preset>& p) { return p->is_same_preset(*preset); });
            if (it != system_presets.end()) {
                return *it;
            }
        }
    } else {
        if (!user_presets.empty()) {
            auto it = std::find_if(user_presets.begin(), user_presets.end(), [this](std::shared_ptr<Preset>& p) { return p->is_same_preset(preset0); });
            if (it != user_presets.end()) {
                return *it;
            }
        }
        if (!system_presets.empty()) {
            auto it = std::find_if(system_presets.begin(), system_presets.end(), [this](std::shared_ptr<Preset>& p) { return p->is_same_preset(preset0); });
            if (it != system_presets.end()) {
                return *it;
            }
        }
    }
    return nullptr;
}

void Hc1Module::readFavorites()
{
    if (system_presets.empty()) return;
    if (!favorite_presets.empty()) return;
    auto path = moduleFavoritesPath();
    readFavoritesFile(path, true);
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
}

void Hc1Module::addFavorite(std::shared_ptr<Preset> preset)
{
#if defined VERBOSE_LOG
    if (preset->bank_hi == 126) {
        DebugLog("Favoriting an edit slot! %s", preset->describe_short().c_str());
    }
#endif
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