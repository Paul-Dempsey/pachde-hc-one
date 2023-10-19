#include "HC-1.hpp"
#include "../he_group.hpp"
namespace pachde {

void Hc1Module::tryCachedPresets()
{
    cached_preset_state = InitState::Complete;

    if (cache_system_presets && hardware != EM_Hardware::Unknown) {
        loadSystemPresets();
    }
    if (cache_user_presets && connection) {
        loadUserPresets();
    }
    if (system_presets.empty()) {
        system_preset_state = InitState::Uninitialized;
    }
    if (user_presets.empty()) {
        user_preset_state = InitState::Uninitialized;
    }

    if (InitState::Complete == system_preset_state 
        && InitState::Complete == user_preset_state
        && favoritesFile.empty()) {
        favoritesFromPresets();
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
    if ((InitState::Complete == system_preset_state)
        && (InitState::Complete == user_preset_state)) {
        heart_time = .05;
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
    return asset::user(format_string("%s/startup.json", pluginInstance->slug.c_str()));    
}

std::string Hc1Module::userPresetsPath()
{
    if (!connection) return "";
    auto conn = to_file_safe(connection->info.spec(), false);
    return asset::user(format_string("%s/%s-user.json", pluginInstance->slug.c_str(), conn.c_str()));
}

std::string Hc1Module::systemPresetsPath()
{
    if (EM_Hardware::Unknown == hardware) return "";
    return asset::user(format_string("%s/%s-system.json", pluginInstance->slug.c_str(), ShortHardwareName(hardware)));
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
    json_object_set_new(root, "init-midi-rate",    json_integer(init_midi_rate));
    json_object_set_new(root, "hearbeat-period",   json_real(hearbeat_period));
    json_object_set_new(root, "post-output-delay", json_real(post_output_delay));
    json_object_set_new(root, "post-input-delay",  json_real(post_input_delay));
    json_object_set_new(root, "post-hello-delay",  json_real(post_hello_delay));
    json_object_set_new(root, "post-system-delay", json_real(post_system_delay));
    json_object_set_new(root, "post-user-delay",   json_real(post_user_delay));
    json_object_set_new(root, "post-config-delay", json_real(post_config_delay));

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
		DebugLog("Invalid JSON at %d:%d %s in %s", error.line, error.column, error.text, path.c_str());
        return;
    }
	DEFER({json_decref(root);});

    auto j = json_object_get(root, "init-midi-rate");
    if (j) {
        init_midi_rate = static_cast<int>(json_integer_value(j));
        if (!in_range(init_midi_rate, 0, 2)) {
            init_midi_rate = 0;
        }
    }
    j = json_object_get(root, "hearbeat-period");
    if (j) {
        hearbeat_period = json_number_value(j);
    }
    j = json_object_get(root, "post-output-delay");
    if (j) {
        post_output_delay = json_number_value(j);
    }
    j = json_object_get(root, "post-input-delay");
    if (j) {
        post_input_delay = json_number_value(j);
    }
    j = json_object_get(root, "post-hello-delay");
    if (j) {
        post_hello_delay = json_number_value(j);
    }
    j = json_object_get(root, "post-system-delay");
    if (j) {
        post_system_delay = json_number_value(j);
    }
    j = json_object_get(root, "post-user-delay");
    if (j) {
        post_user_delay = json_number_value(j);
    }
    j = json_object_get(root, "post-config-delay");
    if (j) {
        post_config_delay = json_number_value(j);
    }
}

void Hc1Module::saveUserPresets()
{
    if (user_presets.empty()) return;
    if (InitState::Complete != user_preset_state) return;
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
    if (InitState::Complete != system_preset_state) return;

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
        system_preset_state = InitState::Broken;
		return;
    }
	DEFER({std::fclose(file);});

	json_error_t error;
	json_t* root = json_loadf(file, 0, &error);
	if (!root) {
        system_preset_state = InitState::Broken;
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
    auto back_insert = std::back_inserter(favorite_presets);
    std::copy_if(user_presets.cbegin(), user_presets.cend(), back_insert, [](const std::shared_ptr<Preset> & p){ return p->favorite; });
    std::copy_if(system_presets.cbegin(), system_presets.cend(), back_insert, [](const std::shared_ptr<Preset> & p){ return p->favorite; });
    sortFavorites();
    apply_favorite_state = InitState::Complete;
}

void Hc1Module::userPresetsToJson(json_t* root)
{
    json_object_set_new(root, "device", json_string(connection->info.spec().c_str()));

    auto jaru = json_array();
    for (auto preset: user_presets) {
        json_array_append_new(jaru, preset->toJson());
    }
    json_object_set_new(root, "user", jaru);
}

void Hc1Module::systemPresetsToJson(json_t* root)
{
    json_object_set_new(root, "hardware", json_string(ShortHardwareName(hardware)));

    auto jars = json_array();
    for (auto preset: system_presets) {
        json_array_append_new(jars, preset->toJson());
    }
    json_object_set_new(root, "system", jars);
}

std::string Hc1Module::moduleFavoritesPath()
{
    if (!connection) return "";
    auto device = to_file_safe(connection->info.spec(), false);
    return asset::user(format_string("%s/fav-%s.json", pluginInstance->slug.c_str(), device.c_str()));
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
    json_object_set_new(root, "device", json_string(connection ? connection->info.spec().c_str() : ""));
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