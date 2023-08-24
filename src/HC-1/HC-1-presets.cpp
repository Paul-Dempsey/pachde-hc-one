#include "HC-1.hpp"
namespace pachde {

std::string Hc1Module::presetsPath()
{
    if (broken || !is_eagan_matrix) return "";
    return asset::user(format_string("%s/pre-%s.json", pluginInstance->slug.c_str(), FilterDeviceName(deviceName()).c_str()));
}

void Hc1Module::savePresets()
{
    auto path = presetsPath();
    DebugLog("Saving presets: %s ", path.c_str());
    if (path.empty()) return;
    auto dir = system::getDirectory(path);
    system::createDirectories(dir);

    auto root = presetsToJson();
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

void Hc1Module::loadPresets()
{
    auto path = presetsPath();
    if (path.empty()) return;
    
    system_preset_state = user_preset_state = InitState::Pending;

    system_presets.clear();
    user_presets.clear();
    favorite_presets.clear();

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
    auto jar = json_object_get(root, "user");
    if (jar) {
        json_t* jp;
        size_t index;
        json_array_foreach(jar, index, jp) {
            auto preset = std::make_shared<MinPreset>();
            preset->fromJson(jp);
            user_presets.push_back(preset);
            if (preset->favorite) {
                favorite_presets.push_back(preset);
            }
        }
    }
    jar = json_object_get(root, "system");
    if (jar) {

        json_t* jp;
        size_t index;
        json_array_foreach(jar, index, jp) {
            auto preset = std::make_shared<MinPreset>();
            preset->fromJson(jp);
            system_presets.push_back(preset);
            if (preset->favorite) {
                favorite_presets.push_back(preset);
            }
        }
    }
    orderFavorites(true);
    system_preset_state = user_preset_state = InitState::Complete;
}

json_t* Hc1Module::presetsToJson()
{
    json_t* root = json_object();
    auto device = FilterDeviceName(deviceName());
    json_object_set_new(root, "device", json_stringn(device.c_str(), device.size()));

    auto jaru = json_array();
    for (auto preset: user_presets) {
        json_array_append_new(jaru, preset->toJson());
    }
    json_object_set_new(root, "user", jaru);

    auto jars = json_array();
    for (auto preset: system_presets) {
        json_array_append_new(jars, preset->toJson());
    }
    json_object_set_new(root, "system", jars);
    return root;
}

std::string Hc1Module::favoritesPath()
{
    if (broken || !is_eagan_matrix) return "";
    return asset::user(format_string("%s/fav-%s.json", pluginInstance->slug.c_str(), FilterDeviceName(deviceName()).c_str()));
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
    json_object_set_new(root, "device", json_stringn(device.c_str(), device.size()));
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
    json_object_set_new(root, "faves", ar);
    return root;
}

void Hc1Module::saveFavorites()
{
    if (favorite_presets.empty()) return;
    auto path = favoritesPath();
    writeFavoritesFile(path);
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

void Hc1Module::readFavoritesFile(const std::string& path)
{
    if (system_presets.empty()) return;
    if (path.empty()) return;
    FILE* file = std::fopen(path.c_str(), "r");
	if (!file) {
		return;
    }
	DEFER({std::fclose(file);});
	json_error_t error;
	json_t* root = json_loadf(file, 0, &error);
	if (!root) {
		DebugLog("Invalid JSON at %d:%d %s in %s", error.line, error.column, error.text, path.c_str());
        return;
    }
	DEFER({json_decref(root);});
    auto bulk = BulkFavoritingMode(this);
    auto jar = json_object_get(root, "faves");
    if (jar) {
        clearFavorites();
        json_t* jp;
        size_t index;
        MinPreset preset;
        json_array_foreach(jar, index, jp) {
            preset.fromJson(jp);
            if (!user_presets.empty()) {
                auto it = std::find_if(user_presets.begin(), user_presets.end(), [&preset](std::shared_ptr<MinPreset>& p) { return p->isSamePreset(preset); });
                if (it != user_presets.end()) {
                    addFavorite(*it);
                }
            }
            if (!system_presets.empty()) {
                auto it = std::find_if(system_presets.begin(), system_presets.end(), [&preset](std::shared_ptr<MinPreset>& p) { return p->isSamePreset(preset); });
                if (it != system_presets.end()) {
                    addFavorite(*it);
                }
            }
        }
    }
    orderFavorites(true);
    saveFavorites();
}

std::shared_ptr<MinPreset> Hc1Module::findSysUserPreset(std::shared_ptr<MinPreset> preset)
{
    if (preset) {
        if (!user_presets.empty()) {
            auto it = std::find_if(user_presets.begin(), user_presets.end(), [preset](std::shared_ptr<MinPreset>& p) { return p->isSamePreset(*preset); });
            if (it != user_presets.end()) {
                return *it;
            }
        }
        if (!system_presets.empty()) {
            auto it = std::find_if(system_presets.begin(), system_presets.end(), [preset](std::shared_ptr<MinPreset>& p) { return p->isSamePreset(*preset); });
            if (it != system_presets.end()) {
                return *it;
            }
        }
    } else {
        if (!user_presets.empty()) {
            auto it = std::find_if(user_presets.begin(), user_presets.end(), [this](std::shared_ptr<MinPreset>& p) { return p->isSamePreset(preset0); });
            if (it != user_presets.end()) {
                return *it;
            }
        }
        if (!system_presets.empty()) {
            auto it = std::find_if(system_presets.begin(), system_presets.end(), [this](std::shared_ptr<MinPreset>& p) { return p->isSamePreset(preset0); });
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
    auto path = favoritesPath();
    readFavoritesFile(path);
}

void Hc1Module::orderFavorites(bool sort)
{
    if (sort) {
        std::sort(favorite_presets.begin(), favorite_presets.end(), favorite_order);
    }
    int n = 0;
    for (auto p: favorite_presets) {
        p->favorite_order = n++;
    }
}

void Hc1Module::addFavorite(std::shared_ptr<MinPreset> preset)
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
                [preset](const std::shared_ptr<MinPreset>& fp){ 
                    return preset->isSamePreset(*fp); 
                })) {
            favorite_presets.push_back(preset);
            if (!bulk_favoriting) {
                orderFavorites(true);
            }
        }
    }
    if (!bulk_favoriting) {
        saveFavorites();
    }
}

void Hc1Module::unFavorite(std::shared_ptr<MinPreset> preset)
{
    preset->favorite = false;
    if (favorite_presets.empty()) return;
    favorite_presets.erase(std::remove(favorite_presets.begin(), favorite_presets.end(), preset), favorite_presets.end());
    saveFavorites();
}

}