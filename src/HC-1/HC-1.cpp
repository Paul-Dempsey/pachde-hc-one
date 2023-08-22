#include "HC-1.hpp"
#include "cc_param.hpp"
#include "../misc.hpp"
namespace pachde {

Hc1Module::Hc1Module()
{
    system_presets.reserve(700);
    user_presets.reserve(128);
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);
    configCCParam(em_midi::EMCC_i,   true, this, M1_PARAM, 0.f, EM_Max14f, EM_Max14f/2.f, "i");
    configCCParam(em_midi::EMCC_ii,  true, this, M2_PARAM, 0.f, EM_Max14f, EM_Max14f/2.f, "ii");
    configCCParam(em_midi::EMCC_iii, true, this, M3_PARAM, 0.f, EM_Max14f, EM_Max14f/2.f, "iii");
    configCCParam(em_midi::EMCC_iv,  true, this, M4_PARAM, 0.f, EM_Max14f, EM_Max14f/2.f, "iv");
    configCCParam(em_midi::EMCC_v,   true, this, M5_PARAM, 0.f, EM_Max14f, EM_Max14f/2.f, "v");
    configCCParam(em_midi::EMCC_vi,  true, this, M6_PARAM, 0.f, EM_Max14f, EM_Max14f/2.f, "vi");
    configCCParam(em_midi::EMCC_R1,   false, this, R1_PARAM, 0.f, 127.f, 64.f, "R1"); //->snapEnabled = true;
    configCCParam(em_midi::EMCC_R2,   false, this, R2_PARAM, 0.f, 127.f, 64.f, "R2"); //->snapEnabled = true;
    configCCParam(em_midi::EMCC_R3,   false, this, R3_PARAM, 0.f, 127.f, 64.f, "R3"); //->snapEnabled = true;
    configCCParam(em_midi::EMCC_R4,   false, this, R4_PARAM, 0.f, 127.f, 64.f, "R4"); //->snapEnabled = true;
    configCCParam(em_midi::EMCC_RMIX, false, this, RMIX_PARAM, 0.f, 127.f, 64.f, "Recirculator Mix"); //->snapEnabled = true;
    configSwitch(M1_REL_PARAM, 0.f, 1.f, 0.f, "i CV relative");
    configSwitch(M2_REL_PARAM, 0.f, 1.f, 0.f, "ii CV relative");
    configSwitch(M3_REL_PARAM, 0.f, 1.f, 0.f, "iii CV relative");
    configSwitch(M4_REL_PARAM, 0.f, 1.f, 0.f, "iv CV relative");
    configSwitch(M5_REL_PARAM, 0.f, 1.f, 0.f, "v CV relative");
    configSwitch(M6_REL_PARAM, 0.f, 1.f, 0.f, "vi CV relative");
    configSwitch(R1_REL_PARAM, 0.f, 1.f, 0.f, "R1 CV relative");
    configSwitch(R2_REL_PARAM, 0.f, 1.f, 0.f, "R2 CV relative");
    configSwitch(R3_REL_PARAM, 0.f, 1.f, 0.f, "R3 CV relative");
    configSwitch(R4_REL_PARAM, 0.f, 1.f, 0.f, "R4 CV relative");
    configSwitch(RMIX_REL_PARAM, 0.f, 1.f, 0.f, "RMix CV relative");

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

    configLight(Lights::HEART_LIGHT, "Device status");

    getLight(HEART_LIGHT).setBrightness(1.0f);
    findEM();
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
}

void Hc1Module::onSave(const SaveEvent& e) {
    Module::onSave(e);
    savePresets();
}

json_t * Hc1Module::dataToJson()
{
    auto root = json_object();
    json_object_set_new(root, "midi-in", midi::Input::toJson());
    json_object_set_new(root, "preset-tab", json_integer(tab));

    auto ar = json_array();
    for (int pg: page) {
        json_array_append_new(ar, json_integer(pg));
    }
    json_object_set_new(root, "tab_page", ar);

    if (current_preset) {
        json_object_set_new(root, "preset", current_preset->toJson());
    }
    json_object_set_new(root, "cache_presets", json_boolean(cache_presets));
    json_object_set_new(root, "heartbeat",  json_boolean(heartbeat));
    return root;
}

void Hc1Module::dataFromJson(json_t *root)
{
    heartbeat = GetBool(root, "heartbeat", heartbeat);
    auto j = json_object_get(root, "preset-tab");
    if (j) {
        tab = static_cast<PresetTab>(clamp(json_integer_value(j), PresetTab::First, PresetTab::Last));
    }

    j = json_object_get(root, "tab_page");
    if (j) {
        for (int i = PresetTab::First; i < PresetTab::Last; ++i) {
            auto el = json_array_get(j, i);
            if (el) {
                page[i] = json_integer_value(el);
            }
        }
    }

    j = json_object_get(root, "preset");
    if (j) {
        saved_preset = std::make_shared<MinPreset>();
        saved_preset->fromJson(j);
    }

    j = json_object_get(root, "midi-in");
    if (j) {
        midi::Input::fromJson(j);
        midi::Input::setChannel(-1);
    }
    device_name = midi::Input::getDeviceName(midi::Input::deviceId);
    is_eagan_matrix = is_EMDevice(device_name);
    findEMOut();
    cache_presets = GetBool(root, "cache_presets", cache_presets);
    if (cache_presets) {
        loadPresets();
    }
}

void Hc1Module::reboot() {
    midi::Input::reset();
    midiOutput.reset();
    clearCCValues();
    midi_receive_count = 0;

    broken = false;
    broken_idle = 0.f;
    heart_phase = 0.f;
    preset0.clear();
    system_presets.clear();
    user_presets.clear();
    config_state = InitState::Uninitialized;
    preset_state = InitState::Uninitialized;
    device_state = InitState::Uninitialized;
    handshake = InitState::Uninitialized;
    requested_updates = InitState::Uninitialized;
    in_preset = in_sys_names = in_user_names = false;

    pedal_fraction = 0;
    notesOn = 0;
    data_stream = -1;
    download_message_id = -1;
    recirculator = 0;
    findEM();
    heart_time = 1.0f;
}

void Hc1Module::onRandomize(const RandomizeEvent& e)
{
    Module::onRandomize();
    if (!ready()) return;

    std::vector<std::shared_ptr<MinPreset>> rp;
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

void Hc1Module::findEMOut() {
    if (is_eagan_matrix) {
        int best_id = -1;
        int common = 0;
        for (auto id : midiOutput.getDeviceIds()) {
            auto name = midiOutput.getDeviceName(id);
            int c2 = common_prefix_length(device_name, name);
            if (c2 > common) {
                best_id = id;
                common = c2;
            }
        }
        if (best_id >= 0) {
            midiOutput.setDeviceId(best_id);
            midiOutput.setChannel(-1);
        }
    }
}

void Hc1Module::findEM() {
    for (auto id : midi::Input::getDeviceIds()) {
        auto dev_name = midi::Input::getDeviceName(id);
        if (is_EMDevice(dev_name)) {
            midi::Input::setDeviceId(id);
            midi::Input::setChannel(-1);
            inputDeviceId = id;
            device_name = dev_name;
            is_eagan_matrix = true;
            //heart_time = 0.25f;
            break;
        }
    }
    findEMOut();
}

void Hc1Module::processCV(int inputId) {
    auto in = getInput(inputId);

    relative_param[inputId] = params[NUM_KNOBS + inputId].getValue() > .5f;
    lights[inputId].setBrightness((relative_param[inputId] *.25f) + ((in.isConnected() && relative_param[inputId]) *.75f));

    if (in.isConnected()) {
        auto v = in.getVoltage();
        ParamQuantity* pq = getParamQuantity(inputId);
        v = v / 10.f * pq->getMaxValue();
        pq->setValue(v);
    }
}

void Hc1Module::processAllCV()
{
    for (int n = M1_INPUT; n <= M6_INPUT; ++n) {
        processCV(n);
    }
    for (int n = R1_INPUT; n <= RMIX_INPUT; ++n) {
        processCV(n);
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
    }

    if (settings::headless && is_ready) {
        float midi_time = midi_timer.process(args.sampleTime);
        if (midi_time > MIDI_RATE) {
            midi_timer.reset();
            for (int n = M1_PARAM; n < NUM_PARAMS; ++n) {
                auto pq = dynamic_cast<CCParamQuantity*>(getParamQuantity(n));
                if (pq) {
                    pq->syncValue();
                }
            }
        }
    }

    // Blink light at 1Hz
    // blinkPhase += args.sampleTime;
    // if (blinkPhase >= 1.f) {
    //     blinkPhase -= 1.f;
    // }
    //lights[BLINK_LIGHT].setBrightness(blinkPhase < 0.5f ? 1.f : 0.f);	
    if (broken) {
        broken_idle += args.sampleTime;
        if (broken_idle > 1.f) {
            reboot();
        }
        return;
    }

    heart_phase += args.sampleTime;
    if (heart_phase >= heart_time) {
        heart_phase -= heart_time;
        heart_time = 2.5f;

        // TODO: fix this mess of conditionals and make a state machine
        if (inputDeviceId != midi::Input::deviceId) {
            device_name = midi::Input::getDeviceName(midi::Input::deviceId);
            is_eagan_matrix = is_EMDevice(device_name);
            inputDeviceId = midi::Input::deviceId;
        } else if (!is_eagan_matrix) {
            findEM();
        } else {
            switch (device_state) {
                case InitState::Uninitialized: transmitInitDevice(); return;
                case InitState::Pending: return;
                case InitState::Broken: transmitInitDevice(); return;
                case InitState::Complete: break;
                default:
                    assert(false);
                    break;
            }
            if (!anyPending()
                && (notesOn <= 0)
                && !in_preset
                ) {
                if (InitState::Uninitialized == preset_state || InitState::Broken == preset_state) {
                    transmitRequestPresets();
                } else 
                // if (InitState::Uninitialized == requested_updates) {
                //     transmitRequestUpdates();
                // } else
                if (InitState::Uninitialized == saved_preset_state) {
                    sendSavedPreset();
                } else
                if (InitState::Uninitialized == config_state) {
                    transmitRequestConfiguration();
                } else
                if (InitState::Complete == config_state) {
                    if (InitState::Uninitialized == requested_updates) {
                        transmitRequestUpdates();
                    } else {
                        if (heartbeat) {
                            sendEditorPresent();
                        }
                    }
                }
            }
        } // matrix
    }// heartbeat
} // process

} //pachde

Model *modelHc1 = createModel<pachde::Hc1Module, pachde::Hc1ModuleWidget>("pachde-hc-1");
