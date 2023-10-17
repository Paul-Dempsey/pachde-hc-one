#include "HC-1.hpp"
#include "../module_broker.hpp"
#include "../widgets/cc_param.hpp"

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

        ++midi_send_count;
        midi_output.sendMessage(msg);
    }
}

#if defined CHECK_DEVICE_CHANGE
bool Hc1Module::checkDeviceChange()
{
    if (InitState::Complete != device_output_state
        || InitState::Complete != device_input_state) {
        return false;
    }
    // handle device changes
    if (connection) {
        auto driver = Input::getDriver(); 
        auto driver2 = ::midi::getDriver(connection->driver_id);

        if (!driver || !driver2) {
            DEBUG("!LOST MIDI DRIVER %s", connection->info.driver_name.c_str());
            reboot();
            return true;
        }
        auto name = FilterDeviceName(Input::getDeviceName(Input::getDeviceId())); 
        //auto name2 = FilterDeviceName(driver->getInputDeviceName(connection->input_device_id));
        if (name.empty() || (0 != name.compare(connection->info.input()))) {
            DEBUG("!LOST MIDI INPUT %s", connection->info.input().c_str());
            reboot();
            return true;
        }
        name = FilterDeviceName(midi_output.getDeviceName(midi_output.getDeviceId())); // FilterDeviceName(driver->getOutputDeviceName(connection->output_device_id));
        if (name.empty() || (0 != name.compare(connection->info.output()))) {
            DEBUG("!LOST MIDI OUTPUT %s", connection->info.output().c_str());
            reboot();
            return true;
        }
    }
    return false;
}
#endif

void Hc1Module::initOutputDevice()
{
    assert(InitState::Uninitialized == device_output_state);

    auto device_broker = MidiDeviceBroker::get();
    using CR = MidiDeviceBroker::ClaimResult;
    connection = nullptr;

     // look for persisted name, if any
     if (!device_claim.empty()) {
        auto r = device_broker->claim_device(Module::getId(), device_claim);
        switch (r) {
            case CR::Ok:
                connection = device_broker->get_connection(device_claim);
                assert(connection);
                midi_output.setDeviceId(connection->output_device_id);
                assert((connection->output_device_id == midi_output.getDeviceId())); // subscribing failed: should handle it?
                midi_output.setChannel(-1);
                device_output_state = InitState::Complete;
                break;
            case CR::AlreadyClaimed: 
                dupe = true;
                break;
            case CR::ArgumentError:
            case CR::NoMidiDevices:
            case CR::NotAvailable: // device isn't plugged in
                break;
        }
     } else {
        auto claim = device_broker->claim_new_device(Module::getId());
        if (!claim.empty()) {
            connection = device_broker->get_connection(claim);
            device_claim = claim;
            midi_output.setDeviceId(connection->output_device_id);
            assert((connection->output_device_id == midi_output.getDeviceId())); // subscribing failed: should handle it?
            midi_output.setChannel(-1);
            device_output_state = InitState::Complete;
        }
    }

    if (InitState::Complete == device_output_state) {
        assert(connection);
        is_eagan_matrix = is_EMDevice(connection->info.input_device_name);
        notifyDeviceChanged();
    }
}

void Hc1Module::process(const ProcessArgs& args)
{
    bool is_ready = ready() && !dupe;

#if defined PERIODIC_DEVICE_CHECK
    device_check.process(args.sampleTime);
    if (connection) {
        if (device_check.getTime() > 4.f) {
            device_check.reset();
            if (ping_device) {
                if (anyPending()) {
                    DEBUG("DEVICE CHECK FAILED FOR %s - rebooting", connection->info.friendly(true).c_str());
                    reboot();
                } else {
                    DebugLog("Device Check OK for %s", connection->info.friendly(true).c_str());
                    ping_device = false;
                }
            } else {
                if (is_ready) {
                    DebugLog("Checking device connection for %s", connection->info.friendly(true).c_str());
                    sendEditorPresent(true);
                    ping_device = true;
                }
            }
        }
    }
#endif

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

    // if (init_step_time > 0.f) {
    //     init_step_phase += args.sampleTime;
    //     if (init_step_phase >= init_step_time) {
    //         // restart step
    //     }
    // }

    // primary hc1 drives device sync
    if (ready()
        && (ModuleBroker::get()->get_primary() == this)
        && (device_sync.process(args.sampleTime) > DEVICE_SYNC_PERIOD)) {
        device_sync.reset();
        MidiDeviceBroker::get()->sync();
    }

    heart_phase += args.sampleTime;
    if (heart_phase >= heart_time) {
        heart_phase -= heart_time;
        heart_time = hearbeat_period;

        if (!anyPending()
            && (notesOn <= 0)
            && !in_preset
            && !dupe
            ) {

            if (InitState::Uninitialized == device_output_state) {
                initOutputDevice();
                if (InitState::Complete == device_output_state) {
                    heart_time = post_output_delay;
                }
                return;
            }

            if (InitState::Uninitialized == device_input_state) {
                if (InitState::Complete == device_output_state) {
                    midi::Input::setDeviceId(connection->input_device_id);
                    assert((connection->input_device_id == midi::Input::getDeviceId())); // subscribing failed: should handle it?
                    midi::Input::setChannel(-1);
                    device_input_state = InitState::Complete;
                    heart_time = post_input_delay;
                }
                return;
            }

            if (InitState::Uninitialized == device_hello_state) {
                transmitDeviceHello();
                return;
            }

            if (InitState::Uninitialized == cached_preset_state) {
                tryCachedPresets();
                if (system_preset_state != InitState::Complete
                    || user_preset_state != InitState::Complete) {
                    heart_time = post_hello_delay;
                    return;
                }
            }

            if (system_preset_state != InitState::Complete) {
                transmitRequestSystemPresets();
                return;
            }

            if (user_preset_state != InitState::Complete) {
                transmitRequestUserPresets();
                return;
            }

            if (InitState::Uninitialized == apply_favorite_state) {
                if (favoritesFile.empty()) {
                    readFavorites();
                } else {
                    readFavoritesFile(favoritesFile, true);
                }
                apply_favorite_state = InitState::Complete;
                return;
            }

            if (InitState::Uninitialized == saved_preset_state) {
                sendSavedPreset();
                return;
            }

            if (InitState::Uninitialized == config_state) {
                transmitRequestConfiguration();
                return;
            }

            if (InitState::Uninitialized == request_updates_state) {
                transmitRequestUpdates();
                return;
            }
            
            if (heart_beating || !first_beat) {
                sendEditorPresent(true);
                return;
            }

#if defined CHECK_DEVICE_CHANGE
            checkDeviceChange();
#endif
        }
    }// heart_beating
} // process

}