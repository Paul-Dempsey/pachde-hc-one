#include "HC-1.hpp"
#include "../cc_param.hpp"
#include "../module_broker.hpp"
#include "../misc.hpp"

namespace pachde {

Hc1Module::Hc1Module()
{
    system_presets.reserve(700);
    user_presets.reserve(128);
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);
    configCCParam(em_midi::EMCC_i,   true, this, M1_PARAM, M1_INPUT, M1_REL_PARAM, M1_REL_LIGHT, 0.f, EM_Max14f, EM_Max14f/2.f, "i");
    configCCParam(em_midi::EMCC_ii,  true, this, M2_PARAM, M2_INPUT, M2_REL_PARAM, M2_REL_LIGHT, 0.f, EM_Max14f, EM_Max14f/2.f, "ii");
    configCCParam(em_midi::EMCC_iii, true, this, M3_PARAM, M3_INPUT, M3_REL_PARAM, M3_REL_LIGHT, 0.f, EM_Max14f, EM_Max14f/2.f, "iii");
    configCCParam(em_midi::EMCC_iv,  true, this, M4_PARAM, M4_INPUT, M4_REL_PARAM, M4_REL_LIGHT, 0.f, EM_Max14f, EM_Max14f/2.f, "iv");
    configCCParam(em_midi::EMCC_v,   true, this, M5_PARAM, M5_INPUT, M5_REL_PARAM, M5_REL_LIGHT, 0.f, EM_Max14f, EM_Max14f/2.f, "v");
    configCCParam(em_midi::EMCC_vi,  true, this, M6_PARAM, M6_INPUT, M6_REL_PARAM, M6_REL_LIGHT, 0.f, EM_Max14f, EM_Max14f/2.f, "vi");
    configCCParam(em_midi::EMCC_R1,   false, this, R1_PARAM, R1_INPUT, R1_REL_PARAM, R1_REL_LIGHT, 0.f, 127.f, 64.f, "R1");
    configCCParam(em_midi::EMCC_R2,   false, this, R2_PARAM, R2_INPUT, R2_REL_PARAM, R2_REL_LIGHT, 0.f, 127.f, 64.f, "R2");
    configCCParam(em_midi::EMCC_R3,   false, this, R3_PARAM, R3_INPUT, R3_REL_PARAM, R3_REL_LIGHT, 0.f, 127.f, 64.f, "R3");
    configCCParam(em_midi::EMCC_R4,   false, this, R4_PARAM, R4_INPUT, R4_REL_PARAM, R4_REL_LIGHT, 0.f, 127.f, 64.f, "R4");
    configCCParam(em_midi::EMCC_RMIX, false, this, RMIX_PARAM, RMIX_INPUT, RMIX_REL_PARAM, RMIX_REL_LIGHT, 0.f, 127.f, 64.f, "Recirculator Mix");
    configCCParam(em_midi::EMCC_PostLevel, true, this, VOLUME_PARAM, VOLUME_INPUT, VOLUME_REL_PARAM, VOLUME_REL_LIGHT, 0.f, EM_Max14f, EM_Max14f/2.f, "Post level");

    configSwitch(M1_REL_PARAM, 0.f, 1.f, 0.f, "Macro i relative-CV", {"off", "on"});
    configSwitch(M2_REL_PARAM, 0.f, 1.f, 0.f, "Macro ii relative-CV", {"off", "on"});
    configSwitch(M3_REL_PARAM, 0.f, 1.f, 0.f, "Macro iii relative-CV", {"off", "on"});
    configSwitch(M4_REL_PARAM, 0.f, 1.f, 0.f, "Macro iv relative-CV", {"off", "on"});
    configSwitch(M5_REL_PARAM, 0.f, 1.f, 0.f, "Macro v relative-CV", {"off", "on"});
    configSwitch(M6_REL_PARAM, 0.f, 1.f, 0.f, "Macro vi relative-CV", {"off", "on"});
    configSwitch(R1_REL_PARAM, 0.f, 1.f, 0.f, "R-1 relative-CV", {"off", "on"});
    configSwitch(R2_REL_PARAM, 0.f, 1.f, 0.f, "R-2 relative-CV", {"off", "on"});
    configSwitch(R3_REL_PARAM, 0.f, 1.f, 0.f, "R-3 relative-CV", {"off", "on"});
    configSwitch(R4_REL_PARAM, 0.f, 1.f, 0.f, "R-4 relative-CV", {"off", "on"});
    configSwitch(RMIX_REL_PARAM, 0.f, 1.f, 0.f, "R Mix relative-CV", {"off", "on"});
    configSwitch(VOLUME_REL_PARAM, 0.f, 1.f, 0.f, "Post level relative-CV", {"off", "on"});
    configSwitch(MUTE_PARAM, 0.f, 1.f, 0.f, "Mute", {"off", "on"});
    configSwitch(RECIRC_EXTEND_PARAM, 0.f, 1.f, 0.f, "Extended recirculator", {"off", "on"});

    configInput(M1_INPUT, "Macro i");
    configInput(M2_INPUT, "Macro ii");
    configInput(M3_INPUT, "Macro iii");
    configInput(M4_INPUT, "Macro iv");
    configInput(M5_INPUT, "Macro v");
    configInput(M6_INPUT, "Macro vi");
    configInput(R1_INPUT, "R2");
    configInput(R2_INPUT, "R2");
    configInput(R3_INPUT, "R3");
    configInput(R4_INPUT, "R4");
    configInput(RMIX_INPUT, "Recirculator mix");
    configInput(VOLUME_INPUT, "Post level");
    configInput(MUTE_INPUT, "Mute trigger");

    configLight(Lights::HEART_LIGHT, "Device status");
    configLight(Lights::ROUND_Y_LIGHT, "Round on Y");
    configLight(Lights::ROUND_INITIAL_LIGHT, "Round initial");
    configLight(Lights::ROUND_LIGHT, "Rounding");
    configLight(Lights::ROUND_RELEASE_LIGHT, "Round on release");

    getLight(HEART_LIGHT).setBrightness(.8f);
    clearCCValues();

    ModuleBroker::get()->registerHc1(this);
}

Hc1Module::~Hc1Module()
{
    notifyDisconnect();
    ModuleBroker::get()->unregisterHc1(this);
    if (restore_ui_data) {
        delete restore_ui_data;
    }
}

//
// IHandleHcEvents notification
//
void Hc1Module::subscribeHcEvents(IHandleHcEvents*client)
{
    if (event_subscriptions.empty()
        || event_subscriptions.cend() == std::find(event_subscriptions.cbegin(), event_subscriptions.cend(), client)) 
    {
        event_subscriptions.push_back(client);

        auto dce = IHandleHcEvents::DeviceChangedEvent{device_name};
        client->onDeviceChanged(dce);

        auto pce = IHandleHcEvents::PresetChangedEvent{current_preset};
        client->onPresetChanged(pce);

        auto rce = IHandleHcEvents::RoundingChangedEvent{rounding};
        client->onRoundingChanged(rce);
    }
}
void Hc1Module::unsubscribeHcEvents(IHandleHcEvents*client)
{
    auto it = std::find(event_subscriptions.begin(), event_subscriptions.end(), client);
    if (it != event_subscriptions.end()) {
        event_subscriptions.erase(it);
    }
}
void Hc1Module::notifyPresetChanged()
{
    if (event_subscriptions.empty()) return;
    auto event = IHandleHcEvents::PresetChangedEvent{current_preset};
    for (auto client: event_subscriptions) {
        client->onPresetChanged(event);
    }
}
void Hc1Module::notifyRoundingChanged()
{
    if (event_subscriptions.empty()) return;
    auto event = IHandleHcEvents::RoundingChangedEvent{rounding};
    for (auto client: event_subscriptions) {
        client->onRoundingChanged(event);
    }
}
void Hc1Module::notifyDeviceChanged()
{
    if (event_subscriptions.empty()) return;
    auto event = IHandleHcEvents::DeviceChangedEvent{device_name};
    for (auto client: event_subscriptions) {
        client->onDeviceChanged(event);
    }
}
void Hc1Module::notifyDisconnect()
{
    if (event_subscriptions.empty()) return;
    auto event = IHandleHcEvents::DisconnectEvent{};
    for (auto client: event_subscriptions) {
        client->onDisconnect(event);
    }
}
void Hc1Module::notifyFavoritesFileChanged()
{
    if (event_subscriptions.empty()) return;
    auto event = IHandleHcEvents::FavoritesFileChangedEvent{favoritesFile};
    for (auto client: event_subscriptions) {
        client->onFavoritesFileChanged(event);
    }
}

void Hc1Module::centerKnobs() {
    paramToDefault(M1_PARAM);
    paramToDefault(M2_PARAM);
    paramToDefault(M3_PARAM);
    paramToDefault(M4_PARAM);
    paramToDefault(M5_PARAM);
    paramToDefault(M6_PARAM);
    paramToDefault(R1_PARAM);
    paramToDefault(R2_PARAM);
    paramToDefault(R3_PARAM);
    paramToDefault(R4_PARAM);
    paramToDefault(RMIX_PARAM);
    paramToDefault(VOLUME_PARAM);
}

void Hc1Module::zeroKnobs() {
    paramToMin(M1_PARAM);
    paramToMin(M2_PARAM);
    paramToMin(M3_PARAM);
    paramToMin(M4_PARAM);
    paramToMin(M5_PARAM);
    paramToMin(M6_PARAM);
    paramToMin(R1_PARAM);
    paramToMin(R2_PARAM);
    paramToMin(R3_PARAM);
    paramToMin(R4_PARAM);
    paramToMin(RMIX_PARAM);
    //paramToDefault(VOLUME_PARAM); // Leave volume alone
}

void Hc1Module::absoluteCV()
{
    getParam(M1_REL_PARAM).setValue(0.f);
    getParam(M2_REL_PARAM).setValue(0.f);
    getParam(M3_REL_PARAM).setValue(0.f);
    getParam(M4_REL_PARAM).setValue(0.f);
    getParam(M5_REL_PARAM).setValue(0.f);
    getParam(M6_REL_PARAM).setValue(0.f);
    getParam(R1_REL_PARAM).setValue(0.f);
    getParam(R2_REL_PARAM).setValue(0.f);
    getParam(R3_REL_PARAM).setValue(0.f);
    getParam(R4_REL_PARAM).setValue(0.f);
    getParam(RMIX_REL_PARAM).setValue(0.f);
    getParam(VOLUME_REL_PARAM).setValue(0.f);
}

void Hc1Module::relativeCV()
{
    getParam(M1_REL_PARAM).setValue(1.f);
    getParam(M2_REL_PARAM).setValue(1.f);
    getParam(M3_REL_PARAM).setValue(1.f);
    getParam(M4_REL_PARAM).setValue(1.f);
    getParam(M5_REL_PARAM).setValue(1.f);
    getParam(M6_REL_PARAM).setValue(1.f);
    getParam(R1_REL_PARAM).setValue(1.f);
    getParam(R2_REL_PARAM).setValue(1.f);
    getParam(R3_REL_PARAM).setValue(1.f);
    getParam(R4_REL_PARAM).setValue(1.f);
    getParam(RMIX_REL_PARAM).setValue(1.f);
    getParam(VOLUME_REL_PARAM).setValue(1.f);
}

void Hc1Module::onSave(const SaveEvent& e)
{
    savePresets();
    Module::onSave(e);
}

void Hc1Module::onRemove(const RemoveEvent& e)
{
    notifyDisconnect();
    ModuleBroker::get()->unregisterHc1(this);
    savePresets();
    Module::onRemove(e);
}

json_t * Hc1Module::dataToJson()
{
    auto root = json_object();
    json_object_set_new(root, "midi-device", json_stringn(device_name.c_str(), device_name.size()));

    json_object_set_new(root, "preset-order", json_integer(static_cast<int>(preset_order)));
    json_object_set_new(root, "preset-tab", json_integer(static_cast<int>(tab)));
    auto ar = json_array();
    for (int pg: page) {
        json_array_append_new(ar, json_integer(pg));
    }
    json_object_set_new(root, "tab-page", ar);

    if (current_preset) {
        json_object_set_new(root, "preset", current_preset->toJson());
    }
    json_object_set_new(root, "restore-preset", json_boolean(restore_saved_preset));
    json_object_set_new(root, "cache-presets", json_boolean(cache_presets));
    json_object_set_new(root, "heartbeat",  json_boolean(heart_beating));
    json_object_set_new(root, "favorites-file", json_stringn(favoritesFile.c_str(), favoritesFile.size()));
    return root;
}

void Hc1Module::dataFromJson(json_t *root)
{
    heart_beating = GetBool(root, "heartbeat", heart_beating);
    auto j = json_object_get(root, "preset-tab");
    if (j) {
        restore_ui_data = new RestoreData();
        restore_ui_data->tab = static_cast<PresetTab>(clamp(static_cast<int>(json_integer_value(j)), static_cast<int>(PresetTab::First), static_cast<int>(PresetTab::Last)));
        tab = restore_ui_data->tab;
        j = json_object_get(root, "tab-page");
        if (j) {
            for (json_int_t i = static_cast<json_int_t>(PresetTab::First); i < static_cast<json_int_t>(PresetTab::Last); ++i) {
                auto el = json_array_get(j, i);
                if (el) {
                    auto pg = json_integer_value(el);
                    restore_ui_data->page[i] = pg;
                    page[i] = pg;
                }
            }
        }
    }
    j = json_object_get(root, "preset-order");
    if (j) {
        preset_order = static_cast<PresetOrder>(clamp(static_cast<int>(json_integer_value(j)), static_cast<int>(PresetOrder::Alpha), static_cast<int>(PresetOrder::Last)));
    }
    j = json_object_get(root, "preset");
    if (j) {
        saved_preset = std::make_shared<Preset>();
        saved_preset->fromJson(j);
    }
    restore_saved_preset = GetBool(root, "restore-preset", restore_saved_preset);
    if (!restore_saved_preset) {
        // pretend it already happened so that it doesn't get scheduled
        saved_preset_state = InitState::Complete;
    }

    j = json_object_get(root, "midi-device");
    if (j) {
        saved_device_name = json_string_value(j);
    }
    
    j = json_object_get(root, "favorites-file");
    if (j) {
        favoritesFile = json_string_value(j);
    }
    cache_presets = GetBool(root, "cache-presets", cache_presets);
    tryCachedPresets();
}

void Hc1Module::reboot()
{
    dupe = false;
    device_name = "";
    midi::Input::reset();
    midi_output.reset();
    clearCCValues();
    midi_receive_count = 0;
    midi_send_count = 0;
    broken = false;
    broken_idle = 0.f;
    heart_phase = 0.f;
    heart_time = 1.0f;
    first_beat = false;
    preset0.clear();
    system_presets.clear();
    user_presets.clear();

    device_output_state =
        device_input_state =
        system_preset_state = 
        duplicate_instance = 
        user_preset_state = 
        apply_favorite_state =
        config_state = 
        request_updates_state = 
        saved_preset_state =
        handshake
        = InitState::Uninitialized;

    in_preset = in_sys_names = in_user_names = false;

    pedal_fraction = 0;
    notesOn = 0;
    data_stream = -1;
    //download_message_id = -1;
    recirculator = 0;

    notifyDeviceChanged();
}

void Hc1Module::onRandomize(const RandomizeEvent& e)
{
    Module::onRandomize();
    if (!ready()) return;

    std::vector<std::shared_ptr<Preset>> rp;
    if (!user_presets.empty()) {
        rp.push_back(user_presets[randomZeroTo(user_presets.size())]);
    }
    if (!system_presets.empty()) {
        rp.push_back(system_presets[randomZeroTo(system_presets.size())]);
    }
    if (!favorite_presets.empty()) {
        rp.push_back(favorite_presets[randomZeroTo(favorite_presets.size())]);
    }
    if (rp.empty()) return;
    setPreset(rp[randomZeroTo(rp.size())]);
}

int Hc1Module::findMatchingInputDevice(const std::string& name)
{
    int best_id = -1;
    std::size_t common = 0;
    for (auto id: midi::Input::getDeviceIds()) {
        auto other_name = FilterDeviceName(midi::Input::getDeviceName(id));
        std::size_t c2 = common_prefix_length(name, other_name);
        if (c2 == name.size()){
            best_id = id;
            break;
        } else if (c2 > common) {
            best_id = id;
            common = c2;
        }
    }
    return best_id;
}

int Hc1Module::findMatchingOutputDevice(const std::string& name)
{
    int best_id = -1;
    std::size_t common = 0;
    for (auto id: midi_output.getDeviceIds()) {
        auto other_name = FilterDeviceName(midi_output.getDeviceName(id));
        std::size_t c2 = common_prefix_length(name, other_name);
        if (c2 == name.size()){
            best_id = id;
            break;
        } else if (c2 > common) {
            best_id = id;
            common = c2;
        }
    }
    return best_id;
}

} //pachde

Model *modelHc1 = createModel<pachde::Hc1Module, pachde::Hc1ModuleWidget>("pachde-hc-1");
