#include "HC-1.hpp"
#include "../cc_param.hpp"
#include "../misc.hpp"
namespace pachde {

Hc1Module::Hc1Module()
{
    //system_presets.reserve(700);
    //user_presets.reserve(128);
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

    configSwitch(M1_REL_PARAM, 0.f, 1.f, 0.f, "i relative-CV", {"off", "on"});
    configSwitch(M2_REL_PARAM, 0.f, 1.f, 0.f, "ii relative-CV", {"off", "on"});
    configSwitch(M3_REL_PARAM, 0.f, 1.f, 0.f, "iii relative-CV", {"off", "on"});
    configSwitch(M4_REL_PARAM, 0.f, 1.f, 0.f, "iv relative-CV", {"off", "on"});
    configSwitch(M5_REL_PARAM, 0.f, 1.f, 0.f, "v relative-CV", {"off", "on"});
    configSwitch(M6_REL_PARAM, 0.f, 1.f, 0.f, "vi relative-CV", {"off", "on"});
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

    configLight(Lights::M1_REL_LIGHT, "i CV relative");
    configLight(Lights::M2_REL_LIGHT, "ii CV relative");
    configLight(Lights::M3_REL_LIGHT, "iii CV relative");
    configLight(Lights::M4_REL_LIGHT, "iv CV relative");
    configLight(Lights::M5_REL_LIGHT, "v CV relative");
    configLight(Lights::M6_REL_LIGHT, "vi CV relative");
    configLight(Lights::R1_REL_LIGHT, "R1 CV relative");
    configLight(Lights::R2_REL_LIGHT, "R2 CV relative");
    configLight(Lights::R3_REL_LIGHT, "R3 CV relative");
    configLight(Lights::R4_REL_LIGHT, "R4 CV relative");
    configLight(Lights::RMIX_REL_LIGHT, "RMix CV relative");
    configLight(Lights::VOLUME_REL_LIGHT, "Volume CV relative");
    configLight(Lights::HEART_LIGHT, "Device status");
    configLight(Lights::MUTE_LIGHT, "Mute");
    configLight(Lights::RECIRC_EXTEND_LIGHT, "R Extend");
    configLight(Lights::ROUND_Y_LIGHT, "Round on Y");
    configLight(Lights::ROUND_INITIAL_LIGHT, "Round initial");
    configLight(Lights::ROUND_LIGHT, "Rounding");
    configLight(Lights::ROUND_RELEASE_LIGHT, "Round on release");
    configLight(Lights::TRANSPOSE_UP_LIGHT, "Transpose up");
    configLight(Lights::TRANSPOSE_NONE_LIGHT, "Transpose none");
    configLight(Lights::TRANSPOSE_DOWN_LIGHT, "Transpose down");

    //configLight(Lights::FILTER_LIGHT, "Filter presets");

    getLight(HEART_LIGHT).setBrightness(.8f);
    clearCCValues();
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


void Hc1Module::onSave(const SaveEvent& e) {
    savePresets();
    Module::onSave(e);
}
void Hc1Module::onRemove(const RemoveEvent& e) {
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
        preset_order = static_cast<PresetOrder>(clamp(static_cast<int>(json_integer_value(j)), static_cast<int>(PresetOrder::Alpha), static_cast<int>(PresetOrder::System)));
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
        device_name = json_string_value(j);
    }
    cache_presets = GetBool(root, "cache-presets", cache_presets);
    if (cache_presets) {
        loadSystemPresets();
        loadUserPresets();
        favoritesFromPresets();
    }
}

void Hc1Module::reboot()
{
    midi::Input::reset();
    midi_output.reset();
    clearCCValues();
    midi_receive_count = 0;
    midi_send_count = 0;
    broken = false;
    broken_idle = 0.f;
    heart_phase = 0.f;

    preset0.clear();
    system_presets.clear();
    user_presets.clear();

    device_output_state =
        device_input_state =
        system_preset_state = 
        user_preset_state = 
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
    heart_time = 1.0f;
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

void Hc1Module::processCV(int paramId)
{
    CCParamQuantity* pq = static_cast<CCParamQuantity*>(getParamQuantity(paramId));
    if (!pq) return;
    if (pq->inputId < 0) return;

    auto in = getInput(pq->inputId);
    bool relative =  pq->relativeId >= 0 ? getParam(pq->relativeId).getValue() > .5f : false;
    if (pq->lightId >= 0) {
        getLight(pq->lightId).setBrightness((relative *.20f) + ((in.isConnected() && relative) *.80f));
    }

    if (in.isConnected()) {
        auto v = in.getVoltage();
        if (relative) {
            pq->setRelativeVoltage(v);
        } else {
            pq->offset = 0.f;
            pq->setKnobVoltage(v);
        }
    } else {
        pq->offset = 0.f;
    }
}

void Hc1Module::processAllCV()
{
    processCV(VOLUME_PARAM);
    for (int n = M1_PARAM; n <= M6_PARAM; ++n) {
        processCV(n);
    }
    for (int n = R1_PARAM; n <= RMIX_PARAM; ++n) {
        processCV(n);
    }
    //getLight(Lights::FILTER_LIGHT).setBrightness(preset_filter.isFiltered() * 1.0f);

    // recirculator extended
    {
        auto pq = getParamQuantity(RECIRC_EXTEND_PARAM);
        bool extended = pq->getValue() > 0.5f;
        if (extended != isExtendRecirculator()) {
            if (extended) {
                recirculator |= EM_Recirculator::Extend;
            } else {
                recirculator &= ~EM_Recirculator::Extend;
            }
            sendControlChange(EM_SettingsChannel, EMCC_RecirculatorType, recirculator);
        }
        getLight(Lights::RECIRC_EXTEND_LIGHT).setBrightness(isExtendRecirculator() * 1.0f);
    }
}

void Hc1Module::syncParam(int paramId)
{
    auto pq = dynamic_cast<CCParamQuantity*>(getParamQuantity(paramId));
    if (pq) {
        pq->syncValue();
    }
}

void Hc1Module::syncParams(float sampleTime)
{
    float midi_time = midi_timer.process(sampleTime);
    if (midi_time > MIDI_RATE) {
        midi_timer.reset();
        if (ready()) {
            syncParam(M1_PARAM);
            syncParam(M2_PARAM);
            syncParam(M3_PARAM);
            syncParam(M4_PARAM);
            syncParam(M5_PARAM);
            syncParam(M6_PARAM);
            syncParam(R1_PARAM);
            syncParam(R2_PARAM);
            syncParam(R3_PARAM);
            syncParam(R4_PARAM);
            syncParam(RMIX_PARAM);
            if (!muted) {
                syncParam(VOLUME_PARAM);
            }
            syncParam(ROUND_RATE_PARAM);
        }
        dispatchMidi();
    }
}

void Hc1Module::dispatchMidi()
{
#ifdef VERBOSE_LOG
    if (midi_dispatch.size() > 50) {
        DEBUG("Large queue: %lld", midi_dispatch.size());
    }
#endif
    while (!midi_dispatch.empty()) {
        midi::Message msg;
        auto um = midi_dispatch.shift();
        um.toMidiMessage(msg);
        midi_output.sendMessage(msg);
        ++midi_send_count;
    }
}

void Hc1Module::process(const ProcessArgs& args)
{
    bool is_ready = ready();

    if (++check_cv > CV_INTERVAL) {
        check_cv = 0;
        if (is_ready) {
            processAllCV();
        }
        bool round = rounding.rate > 0;
        getLight(Lights::ROUND_Y_LIGHT).setBrightness(1.0f * (round && (rounding.kind >= RoundKind::Y)));
        getLight(Lights::ROUND_INITIAL_LIGHT).setBrightness(1.0f * (round && rounding.initial));
        getLight(Lights::ROUND_LIGHT).setBrightness(1.0f * round);
        getLight(Lights::ROUND_RELEASE_LIGHT).setBrightness(1.0f * (round && (rounding.kind <= RoundKind::Release)));
        getLight(Lights::TRANSPOSE_UP_LIGHT).setBrightness(1.0 * (middle_c > 60));
        getLight(Lights::TRANSPOSE_NONE_LIGHT).setBrightness(1.0 * (middle_c == 60));
        getLight(Lights::TRANSPOSE_DOWN_LIGHT).setBrightness(1.0 * (middle_c < 60));

    }

    syncParams(args.sampleTime);

    if (is_ready) {
        // MUTE
        auto pq = getParamQuantity(MUTE_PARAM);
        bool new_mute = pq->getValue() > 0.5f;
        if (new_mute != muted) {
            muted = new_mute;
            if (muted) {
                lights[MUTE_LIGHT].setBrightness(1.f);
                sendControlChange(EM_SettingsChannel, EMCC_PostLevel, 0);
            } else {
                lights[MUTE_LIGHT].setBrightness(0.f);
                auto vpq = dynamic_cast<CCParamQuantity*>(getParamQuantity(VOLUME_PARAM));
                vpq->sendValue();
            }
        }
    }

    if (broken) {
        broken_idle += args.sampleTime;
        if (broken_idle > 1.75f) {
            reboot();
        }
        return;
    }

    // beginning of init phase timeout impl, if we need it
    // if (init_step_time > 0.f) {
    //     init_step_phase += args.sampleTime;
    //     if (init_step_phase >= init_step_time) {
    //         // restart step
    //     }
    // }


    heart_phase += args.sampleTime;
    if (heart_phase >= heart_time) {
        heart_phase -= heart_time;
        heart_time = 2.5f;

        // handle device changes
        if (InitState::Complete == device_output_state 
            && InitState::Complete == device_input_state) {
            if (input_device_id != midi::Input::getDeviceId()) {
                auto name = FilterDeviceName(midi::Input::getDeviceName(deviceId));
                if (is_EMDevice(name)) {
                    // find matching output
                    is_eagan_matrix = true;
                    device_name = name;
                    input_device_id = midi::Input::getDeviceId();
                    auto id = findMatchingOutputDevice(name);
                    if (id >= 0) {
                        midi_output.setDeviceId(id);
                        midi_output.setChannel(-1);
                        output_device_id = id;
                        return;
                    }
                } 
                reboot();
                return;
            } else if (output_device_id != midi_output.getDeviceId()) {
                auto name = FilterDeviceName(midi::Input::getDeviceName(midi_output.getDeviceId()));
                if (is_EMDevice(name)) {
                    // find matching input
                    is_eagan_matrix = true;
                    device_name = name;
                    output_device_id = midi_output.getDeviceId();
                    auto id = findMatchingInputDevice(name);
                    if (id >= 0) {
                        midi::Input::setDeviceId(id);
                        midi::Input::setChannel(-1);
                        input_device_id = id;
                        return;
                    }
                }
                reboot();
                return;
            }
        }

        switch (device_output_state) {
        case InitState::Uninitialized: {
            // use peristed name, if any
            if (!device_name.empty()) {
                assert(is_EMDevice(device_name));
                auto id = findMatchingOutputDevice(device_name);
                if (id >= 0) {
                    midi_output.setDeviceId(id);
                    midi_output.setChannel(-1);
                    device_output_state = InitState::Complete;
                    heart_time = 5.f;
                    return;
                } else { 
                    device_name.clear();
                }
            }
            // scan for EM
            for (auto id : midi_output.getDeviceIds()) {
                auto name = FilterDeviceName(midi_output.getDeviceName(id));
                if (is_EMDevice(name)) {
                    device_name = name;
                    midi_output.setDeviceId(id);
                    midi_output.setChannel(-1);
                    device_output_state = InitState::Complete;
                    heart_time =5.f;
                }
            }
            return;
        } break;
        case InitState::Complete: break;
        case InitState::Pending:
        case InitState::Broken:
        default: assert(false); return;
        }

        switch (device_input_state) {
        case InitState::Uninitialized: {
            assert(!device_name.empty());
            assert(is_EMDevice(device_name));
            int best_id = findMatchingInputDevice(device_name);
            if (best_id >= 0) {
                midi::Input::setDeviceId(best_id);
                midi::Input::setChannel(-1);
                is_eagan_matrix = is_EMDevice(device_name);
                device_input_state = InitState::Complete;
                return;
            }
        } break;
        case InitState::Complete: break;
        case InitState::Pending:
        case InitState::Broken:
        default: assert(false); return;
        }

        if (is_eagan_matrix) {
            // switch (device_hello_state) {
            //     case InitState::Uninitialized: transmitInitDevice(); return;
            //     case InitState::Pending: return;
            //     case InitState::Broken: transmitInitDevice(); return;
            //     case InitState::Complete: break;
            //     default: assert(false); break;
            // }
            if (!anyPending()
                && (notesOn <= 0)
                && !in_preset
                ) {
                if (InitState::Uninitialized == system_preset_state || InitState::Broken == system_preset_state) {
                    transmitRequestSystemPresets();
                } else 
                if (InitState::Uninitialized == user_preset_state || InitState::Broken == user_preset_state) {
                    transmitRequestUserPresets();
                } else
                if (InitState::Uninitialized == saved_preset_state) {
                    sendSavedPreset();
                } else
                if (InitState::Uninitialized == config_state) {
                    transmitRequestConfiguration();
                } else
                if (InitState::Uninitialized == request_updates_state) {
                    transmitRequestUpdates();
                } else if (heart_beating) {
                    sendEditorPresent();
                }
            }
        }
    }// heart_beating
} // process

} //pachde

Model *modelHc1 = createModel<pachde::Hc1Module, pachde::Hc1ModuleWidget>("pachde-hc-1");
