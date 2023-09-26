#include "HC-1.hpp"
#include "../cc_param.hpp"
#include "../module_broker.hpp"
namespace pachde {

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

void Hc1Module::syncStatusLights()
{
    bool round = rounding.rate > 0;
    getLight(Lights::ROUND_Y_LIGHT).setBrightness(1.0f * (round && (rounding.kind >= RoundKind::Y)));
    getLight(Lights::ROUND_INITIAL_LIGHT).setBrightness(1.0f * (rounding.initial));
    getLight(Lights::ROUND_LIGHT).setBrightness(1.0f * round);
    getLight(Lights::ROUND_RELEASE_LIGHT).setBrightness(1.0f * (round && (rounding.kind <= RoundKind::Release)));

    // getLight(Lights::TRANSPOSE_UP_LIGHT).setBrightness(1.0 * (middle_c > 60));
    // getLight(Lights::TRANSPOSE_NONE_LIGHT).setBrightness(1.0 * (middle_c == 60));
    // getLight(Lights::TRANSPOSE_DOWN_LIGHT).setBrightness(1.0 * (middle_c < 60));
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

bool Hc1Module::checkDeviceChange()
{
    // handle device changes
    if (InitState::Complete == device_output_state 
        && InitState::Complete == device_input_state) {
        if (input_device_id != midi::Input::getDeviceId()) {
            device_name = FilterDeviceName(midi::Input::getDeviceName(deviceId));
            if (is_EMDevice(device_name)) {
                // find matching output
                is_eagan_matrix = true;
                input_device_id = midi::Input::getDeviceId();
                auto id = findMatchingOutputDevice(device_name);
                if (id >= 0) {
                    midi_output.setDeviceId(id);
                    midi_output.setChannel(-1);
                    output_device_id = id;
                    notifyDeviceChanged();
                    return true;
                }
            } 
            reboot();
            return true;

        } else if (output_device_id != midi_output.getDeviceId()) {
            device_name = FilterDeviceName(midi::Input::getDeviceName(midi_output.getDeviceId()));
            if (is_EMDevice(device_name)) {
                // find matching input
                is_eagan_matrix = true;
                output_device_id = midi_output.getDeviceId();
                auto id = findMatchingInputDevice(device_name);
                if (id >= 0) {
                    midi::Input::setDeviceId(id);
                    midi::Input::setChannel(-1);
                    input_device_id = id;
                    notifyDeviceChanged();
                    return true;
                }
            }
            reboot();
            return true;
        }
    }
    return false;
}

bool claimed(std::vector<DeviceAssociation>& claims, int64_t my_module_id, const std::string& device)
{
    auto it = std::find_if(claims.cbegin(), claims.cend(), [=](const DeviceAssociation& da){
        if (da.module_id == my_module_id) return false;
        if (da.device_name == device) return true;
        return false;
    });
    return it != claims.cend();
}

void Hc1Module::initOutputDevice()
{
    assert(InitState::Uninitialized == device_output_state);
    assert(device_name.empty());

    bool singleton = ModuleBroker::get()->Hc1count() == 1;
    auto claims = !singleton
        ? DeviceAssociation::getList()
        : std::vector<DeviceAssociation>{};

    // look for persisted name, if any
    if (!saved_device_name.empty()) {
        auto id = findMatchingOutputDevice(saved_device_name);
        if (id >= 0) {
            if (singleton || !claimed(claims, getId(), saved_device_name)) {
                device_name = saved_device_name;
                midi_output.setDeviceId(id);
                midi_output.setChannel(-1);
                heart_time = 5.f;
                device_output_state = InitState::Complete;
                notifyDeviceChanged();
                saved_device_name = "";
                return;
            }
        }
        saved_device_name = "";
    }

    // scan for unclaimed EM
    for (auto id : midi_output.getDeviceIds()) {
        auto name = FilterDeviceName(midi_output.getDeviceName(id));
        if (is_EMDevice(name)) {
            if (singleton || !claimed(claims, getId(), name)) {
                is_eagan_matrix = true;
                device_name = name;
                midi_output.setDeviceId(id);
                midi_output.setChannel(-1);
                device_output_state = InitState::Complete;
                heart_time = 5.f;
                break;
            }
        }
    }
    if (!device_name.empty()) {
        notifyDeviceChanged();
    }
}

bool Hc1Module::initDevices()
{
    switch (device_output_state) {
    case InitState::Uninitialized:
        initOutputDevice(); 
        return true;
        break;
    case InitState::Complete: break;
    case InitState::Pending:
    case InitState::Broken:
    default: assert(false); return true;
    }

    switch (device_input_state) {
    case InitState::Uninitialized: {
        assert(!device_name.empty());
        //assert(is_EMDevice(device_name));
        int best_id = findMatchingInputDevice(device_name);
        if (best_id >= 0) {
            midi::Input::setDeviceId(best_id);
            midi::Input::setChannel(-1);
            device_input_state = InitState::Complete;
            return true;
        } 
    } break;
    case InitState::Complete: return false;
    case InitState::Pending:
    case InitState::Broken:
    default: assert(false); return true;
    }

    return false;
}

void Hc1Module::process(const ProcessArgs& args)
{
    bool is_ready = ready() && !dupe;

    if (++check_cv > CV_INTERVAL) {
        check_cv = 0;
        if (is_ready) {
            processAllCV();
            syncStatusLights();
        }
    }

    syncParams(args.sampleTime);

    if (is_ready) {
        // MUTE
        if (getInput(Inputs::MUTE_INPUT).isConnected()) {
            auto v = getInput(Inputs::MUTE_INPUT).getVoltage();
            if (mute_trigger.process(v, 0.1f, 5.f)) {
                mute_trigger.reset();
                auto pq = getParamQuantity(Params::MUTE_PARAM);
                bool mute = !(pq->getValue() >= .5f); // toggle
                pq->setValue(1.0f * mute);
            }
        }
        auto pq = getParamQuantity(MUTE_PARAM);
        bool new_mute = pq->getValue() >= 0.5f;
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
        heart_time = 2.1f;

        if (checkDeviceChange()) return;
        if (initDevices()) return;

        if (is_eagan_matrix) {
            if (!anyPending()
                && (notesOn <= 0)
                && !in_preset
                ) {
                if (InitState::Uninitialized == duplicate_instance) {
                    checkDuplicate();
                }
                if (dupe) return;

                if (InitState::Uninitialized == system_preset_state || InitState::Broken == system_preset_state) {
                    if (InitState::Broken != system_preset_state) {
                        tryCachedPresets();
                    }
                    if (system_preset_state != InitState::Complete) {
                        transmitRequestSystemPresets();
                    }
                } else 
                if (InitState::Uninitialized == user_preset_state || InitState::Broken == user_preset_state) {
                    transmitRequestUserPresets();
                } else
                if (InitState::Uninitialized == apply_favorite_state) {
                    if (favoritesFile.empty()) {
                        readFavorites();
                    } else {
                        readFavoritesFile(favoritesFile, true);
                    }
                    apply_favorite_state = InitState::Complete;
                } else
                if (InitState::Uninitialized == saved_preset_state) {
                    sendSavedPreset();
                } else
                if (InitState::Uninitialized == config_state) {
                    transmitRequestConfiguration();
                } else
                if (InitState::Uninitialized == request_updates_state) {
                    transmitRequestUpdates();
                } else if (heart_beating || !first_beat) {
                    sendEditorPresent();
                }
            }
        }
    }// heart_beating
} // process

}