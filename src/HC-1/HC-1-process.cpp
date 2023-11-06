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
                assert(connection);
                midi_output.setDeviceId(connection->output_device_id);
                if (connection->output_device_id != midi_output.getDeviceId()) {
                    // subscribing failed
                    reboot();
                    return;
                }
                midi_output.setChannel(-1);
                finish_phase(InitPhase::DeviceOutput);
                completed = true;
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
        }
    }

    if (completed) {
        assert(connection);
        is_eagan_matrix = is_EMDevice(connection->info.input_device_name);
        notifyDeviceChanged();
    }
}

void Hc1Module::advanceInitPhase()
{
    auto phase = std::find_if(init_phase.begin(), init_phase.end(), 
        [](const InitPhaseInfo& info) {
            return info.state != InitState::Complete; 
        });

    if (init_phase.end() == phase) {
        restore_midi_rate();
        return;
    }
    if (phase->pending()) return;
    if (phase->broken()) phase->refresh();

    switch (phase->id) {
    case InitPhase::None:
    case InitPhase::Done:
    default:
        assert(false);
        return;

    case InitPhase::DeviceOutput:
        initOutputDevice();
        if (phase->finished()) {
            heart_time = phase->post_delay;
        }
        break;

    case InitPhase::DeviceInput:
        if (finished(InitPhase::DeviceOutput)) {
            assert(connection);
            midi::Input::setDeviceId(connection->input_device_id);
            if (connection->input_device_id != midi::Input::getDeviceId()) {
                // subscribing failed
                reboot();
                return;
            }
            midi::Input::setChannel(-1);
            phase->finish();
            heart_time = phase->post_delay;
        }
        break;

    case InitPhase::DeviceHello:
        first_beat = false;
        phase->pend();
        silence(true);
        dispatchMidi();
        send_init_midi_rate(phase->midi_rate);
        sendEditorPresent();
        break;

    case InitPhase::DeviceConfig:
        first_beat = false;
        phase->pend();
        send_init_midi_rate(phase->midi_rate);
        transmitDeviceConfig();
        break;

    case InitPhase::CachedPresets:
        tryCachedPresets();
        phase->finish();
        if (finished(InitPhase::SystemPresets)
            && finished(InitPhase::UserPresets)) {
            heart_time = phase->post_delay;
        }
        return;

    case InitPhase::UserPresets:
        first_beat = false;
        phase->pend();
        send_init_midi_rate(phase->midi_rate);
        transmitRequestUserPresets();
        return;

    case InitPhase::SystemPresets:
        first_beat = false;
        phase->pend();
        send_init_midi_rate(phase->midi_rate);
        transmitRequestSystemPresets();
        return;

    case InitPhase::Favorites:
        if (favoritesFile.empty()) {
            readFavorites();
        } else {
            readFavoritesFile(favoritesFile, true);
        }
        phase->finish();
        heart_time = phase->post_delay;
        return;

    case InitPhase::SavedPreset:
        if (!saved_preset) {
            phase->finish();
            return;
        }
        first_beat = false;
        phase->pend();
        send_init_midi_rate(phase->midi_rate);
        sendSavedPreset();
        break;

    case InitPhase::PresetConfig:
        first_beat = false;
        phase->pend();
        send_init_midi_rate(phase->midi_rate);
        transmitRequestConfiguration();
        break;

    case InitPhase::RequestUpdates:
        sendControlChange(EM_SettingsChannel, EMCC_Preserve, 1); // bit 1 means request config
        phase->finish();
        heart_time = phase->post_delay;
       break;

    case InitPhase::Heartbeat:
        restore_midi_rate();
        phase->pend();
        sendEditorPresent();
        heart_time = phase->post_delay;
        break;
    }
}

void Hc1Module::process(const ProcessArgs& args)
{
    bool is_ready = ready();

    if (++check_cv > CV_INTERVAL) {
        check_cv = 0;
        if (is_ready) {
            processAllCV();
            syncStatusLights();
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
    if (is_ready
        && (ModuleBroker::get()->get_primary() == this)
        && (device_sync.process(args.sampleTime) > DEVICE_SYNC_PERIOD)) {
        device_sync.reset();
        MidiDeviceBroker::get()->sync();
    }

    heart_phase += args.sampleTime;

    if (heart_phase >= heart_time) {
        heart_phase -= heart_time;
        if (is_ready) {
            heart_time = heartbeat_period;
        }
        if (!anyPending() && !in_preset) {
            if (midi_input_worker->pausing) {
                midi_input_worker->resume();
            }
            if (first_beat && heart_beating) {
                fresh_phase(InitPhase::Heartbeat);
            }
            advanceInitPhase();
        }
    }// heart_beating
} // process

}