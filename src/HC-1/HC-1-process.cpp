#include "HC-1.hpp"
#include "../module_broker.hpp"
#include "../widgets/cc_param.hpp"
#include "midi_input_worker.hpp"

namespace pachde {

void Hc1Module::clearPreset()
{
    current_preset = nullptr;
    preset0.clear();
    clearCCValues();
    em.clear();
}

void Hc1Module::beginPreset()
{
    in_preset = true;
    clearPreset();
    dsp[2] = dsp[1] = dsp[0] = 0;
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

void Hc1Module::syncStatusLights()
{
    bool round = em.rounding.rate > 0;
    getLight(Lights::ROUND_Y_LIGHT).setBrightness(1.0f * (round && (em.rounding.kind >= RoundKind::Y)));
    getLight(Lights::ROUND_INITIAL_LIGHT).setBrightness(1.0f * (em.rounding.initial));
    getLight(Lights::ROUND_LIGHT).setBrightness(1.0f * round);
    getLight(Lights::ROUND_RELEASE_LIGHT).setBrightness(1.0f * (round && (em.rounding.kind <= RoundKind::Release)));

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
        if (extended != em.recirculator.extended()) {
            em.recirculator.setExtended(extended);
            sendControlChange(EM_SettingsChannel, EMCC_RecirculatorType, em.recirculator);
        }
        getLight(Lights::RECIRC_EXTEND_LIGHT).setBrightness(isExtendRecirculator() * 1.0f);
    }
    // recirculator enabled
    {
        auto pq = getParamQuantity(RECIRC_ENABLE_PARAM);
        bool disabled = pq->getValue() <= 0.5f;
        if (disabled != em.recirculator.disabled()) {
            em.recirculator.setDisabled(disabled);
            em.global_ActionAesMenuRecirc = (em.global_ActionAesMenuRecirc & ~0x40) | (disabled * 0x40);
            sendControlChange(EM_SettingsChannel, EMCC_ActionAesMenuRecirc, em.global_ActionAesMenuRecirc);
        }
        getLight(Lights::RECIRC_ENABLE_LIGHT).setBrightness((!disabled) * 1.0f);
    }

}

void Hc1Module::processReadyTrigger(bool ready, const ProcessArgs& args)
{
    bool high = ready_trigger.process(args.sampleTime);
    if ((ready || first_beat)
        && getOutput(READY_TRIGGER).isConnected()
        && !high
        && !ready_sent
        ) {
        ready_trigger.trigger();
        ready_sent = true;
    }
    getOutput(READY_TRIGGER).setVoltage(high * 10.f);
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

void Hc1Module::initOutputDevice()
{
    auto device_broker = MidiDeviceBroker::get();
    using CR = MidiDeviceBroker::ClaimResult;
    connection = nullptr;
    bool completed = false;
     // look for persisted name, if any
     if (!device_claim.empty()) {
        auto r = device_broker->claim_device(Module::getId(), device_claim);
        switch (r) {
            case CR::Ok:
                connection = device_broker->get_connection(device_claim);
                if (connection) {
                    midi_output.setDeviceId(connection->output_device_id);
                    if (connection->output_device_id != midi_output.getDeviceId()) {
                        // subscribing failed
                        reboot();
                        return;
                    }
                    midi_output.setChannel(-1);
                    finish_phase(InitPhase::DeviceOutput);
                    completed = true;
                } else {
                    reboot();
                    return;
                }
                break;
            case CR::AlreadyClaimed: 
            case CR::ArgumentError:
            case CR::NoMidiDevices:
            case CR::NotAvailable: // device isn't plugged in
                break;
        }
     } else {
        auto claim = device_broker->claim_new_device(Module::getId());
        if (!claim.empty()) {
            connection = device_broker->get_connection(claim);
            if (connection) {
                device_claim = claim;
                midi_output.setDeviceId(connection->output_device_id);
                if (connection->output_device_id != midi_output.getDeviceId()) {
                    // subscribing failed
                    reboot();
                    return;
                }
                midi_output.setChannel(-1);
                finish_phase(InitPhase::DeviceOutput);
                completed = true;
            } else {
                reboot();
                return;
            }
        }
    }

    if (completed) {
        assert(connection);
        is_eagan_matrix = is_EMDevice(connection->info.input_device_name);
        notifyDeviceChanged();
    }
}

void Hc1Module::process_init_phase(const ProcessArgs& args)
{
    InitPhaseInfo * phase = nullptr;
    if (InitPhase::Done == current_phase) {
        auto pit = std::find_if(init_phase.begin(), init_phase.end(), 
            [](const InitPhaseInfo& info) {
                return info.state != InitState::Complete; 
            });
        if (init_phase.end() != pit) {
            phase_attempt = 0;
            current_phase = pit->id;
            phase = &*pit;
        } else {
            return;
        }
    }
    if (!phase) {
        phase = get_phase(current_phase);
    }
    if (!phase) {
        current_phase = InitPhase::DeviceOutput;
        phase_attempt = 0;
        return;
    }
    //
    // phase timing, retry, midi rate
    //
    switch (phase->state) {
    case InitState::Uninitialized:
        phase_time = 0.f;
        if (midi_input_worker->pausing) {
            midi_input_worker->resume();
        }
        break;

    case InitState::Pending:
        phase_time += args.sampleTime;
        if ((phase->budget > 0.f) && (phase_time > phase->budget)) {
            ++phase_attempt;
            if (phase_attempt > 3) {
                reboot();
            } else {
                switch (phase->midi_rate) {
                case EMMidiRate::Full: send_init_midi_rate(EMMidiRate::Third); break;
                case EMMidiRate::Third: send_init_midi_rate(EMMidiRate::Twentieth); break;
                case EMMidiRate::Twentieth: break;
                }
                phase->refresh();
            }
        }
        return;

    case InitState::Broken:
        switch (phase->midi_rate) {
        case EMMidiRate::Full: send_init_midi_rate(EMMidiRate::Third); break;
        case EMMidiRate::Third: send_init_midi_rate(EMMidiRate::Twentieth); break;
        case EMMidiRate::Twentieth: break;
        }
        phase_attempt = 0;
        phase->refresh();
        return;

    case InitState::Complete:
        restore_midi_rate();
        if (phase_pause) {
            phase_time += args.sampleTime;
            if (phase_time > phase->post_delay) {
                phase_pause = false;
                current_phase = NextPhase(current_phase);
                phase_attempt = 0;
            }
        } else {
            if (phase->post_delay > 0.f) {
                phase_pause = true;
                phase_time = 0.f;
            } else {
                current_phase = NextPhase(current_phase);
                phase_attempt = 0;
            }
        }
        return;
    }

    //
    // phase initiation
    //
    switch (current_phase) {
    case InitPhase::None: break;

    case InitPhase::DeviceOutput: {
        initOutputDevice();
    } break;

    case InitPhase::DeviceInput: {
        //assert(finished(InitPhase::DeviceOutput));
        //assert(connection);
        midi::Input::setDeviceId(connection->input_device_id);
        if (connection->input_device_id != midi::Input::getDeviceId()) {
            // subscribing failed
            reboot();
            return;
        }
        midi::Input::setChannel(-1);
        phase->finish();
    } break;

    case InitPhase::DeviceHello: {
        first_beat = false;
        phase->pend();
        silence(true);
        dispatchMidi();
        if (0 == phase_attempt) {
            send_init_midi_rate(phase->midi_rate);
        } 
        sendEditorPresent();
    } break;
 
    case InitPhase::DeviceConfig: {
        first_beat = false;
        phase->pend();
        if (0 == phase_attempt) {
            send_init_midi_rate(phase->midi_rate);
        } 
        transmitDeviceConfig();        
    } break;
 
    case InitPhase::CachedPresets: {
        tryCachedPresets();
        phase->finish();
        if (finished(InitPhase::SystemPresets)
            && finished(InitPhase::UserPresets)) {
            current_phase = InitPhase::Favorites;
         }
    } break;
 
    case InitPhase::UserPresets: {
        first_beat = false;
        phase->pend();
        if (0 == phase_attempt) {
            send_init_midi_rate(phase->midi_rate);
        } 
        transmitRequestUserPresets();
    } break;
 
    case InitPhase::SystemPresets: {
        first_beat = false;
        phase->pend();
        if (0 == phase_attempt) {
            send_init_midi_rate(phase->midi_rate);
        } 
        transmitRequestSystemPresets();
    } break;
 
    case InitPhase::Favorites: {
        readFavoritesFile(favoritesFile.empty() ? moduleFavoritesPath() : favoritesFile, true);
        phase->finish();
    } break;
 
    case InitPhase::SavedPreset: {
        if (!saved_preset) {
            phase->finish();
            return;
        }
        first_beat = false;
        phase->pend();
        if (0 == phase_attempt) {
            send_init_midi_rate(phase->midi_rate);
        } 
        sendSavedPreset();
    } break;
 
    case InitPhase::PresetConfig: {
        first_beat = false;
        phase->pend();
        if (0 == phase_attempt) {
            send_init_midi_rate(phase->midi_rate);
        } 
        transmitRequestConfiguration();
    } break;
 
    case InitPhase::RequestUpdates: {
        sendControlChange(EM_SettingsChannel, EMCC_Preserve, 1); // bit 1 means request config
        phase->finish();
    } break;

    case InitPhase::Heartbeat: {
        phase->pend();
        sendEditorPresent();
    } break;

    case InitPhase::Done: {
        phase->finish();
    } break;

    default: break;
    }
}

void Hc1Module::process(const ProcessArgs& args)
{
    bool is_ready = ready();

    if (is_ready && (0 == ((args.frame + id) % CV_INTERVAL))) {
        processAllCV();
        syncStatusLights();

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

    syncParams(args.sampleTime);
    
    processReadyTrigger(is_ready, args);

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
      
    }

    if (broken) {
        broken_idle += args.sampleTime;
        if (broken_idle > 1.75f) {
            reboot();
        }
        return;
    }

    // primary hc1 drives device sync
    if (is_ready
        && (ModuleBroker::get()->get_primary() == this)
        && (device_sync.process(args.sampleTime) > DEVICE_SYNC_PERIOD)) {
        device_sync.reset();
        MidiDeviceBroker::get()->sync();
    }

    process_init_phase(args);

} // process

}