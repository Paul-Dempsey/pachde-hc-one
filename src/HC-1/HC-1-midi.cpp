#include "HC-1.hpp"
#include "../module_broker.hpp"
#include "../widgets/cc_param.hpp"
#include "../em_device.hpp"

namespace pachde {

void Hc1Module::onRenewClaim()
{
    if (device_claim.empty()) return;
    using CR = MidiDeviceBroker::ClaimResult;

    auto oldConnection = connection;
    connection = nullptr;
    int old_output_device_id = midi_output.getDeviceId();

    auto device_broker = MidiDeviceBroker::get();
    auto r = device_broker->claim_device(Module::getId(), device_claim);
    switch (r) {
        case CR::Ok:
            connection = device_broker->get_connection(device_claim);
            assert(connection);
            if (connection->output_device_id != old_output_device_id) {
                midi_dispatch.clear();
                midi_output.reset();
                midi_output.setDeviceId(connection->output_device_id);
                assert((connection->output_device_id == midi_output.getDeviceId())); // subscribing failed: should handle it?
                midi_output.setChannel(-1);

                device_input_state = InitState::Uninitialized;
                Input::reset();
                heart_time = post_output_delay;
                // these should be picked up by input device init
                //Input::setDeviceId(connection->input_device_id);
                //Input::setChannel(-1);
            }
            break;
        case CR::AlreadyClaimed: 
        case CR::ArgumentError:
        case CR::NoMidiDevices:
        case CR::NotAvailable: // device isn't plugged in
            reboot();
            break;
    }
}

void Hc1Module::setMidiDevice(const std::string & claim)
{
    if (0 == device_claim.compare(claim)) return;
    device_claim = claim;
    reboot();
}

void Hc1Module::queueMidiMessage(uMidiMessage msg)
{
    if (dupe) {
        return;
    }
    if (midi_dispatch.full()) {
        reboot();
    } else {
        midi_dispatch.push(msg);
    }
}

void Hc1Module::sendControlChange(uint8_t channel, uint8_t cc, uint8_t value)
{
    queueMidiMessage(uMidiMessage(MidiStatus_CC|channel, cc, value));
}

void Hc1Module::sendProgramChange(uint8_t channel, uint8_t program)
{
    queueMidiMessage(uMidiMessage(MidiStatus_ProgramChange|channel, program));
}

void Hc1Module::sendResetAllreceivers()
{
    queueMidiMessage(uMidiMessage(0xff));
}

void Hc1Module::set_init_midi_rate() {
    switch (init_midi_rate) {
    case 1: sendControlChange(EM_SettingsChannel, EMCC_Download, EM_DownloadItem::midiTxThird); break;
    case 2: sendControlChange(EM_SettingsChannel, EMCC_Download, EM_DownloadItem::midiTxTweenth); break;
    default: break;
    }
}

void Hc1Module::restore_midi_rate() {
    if (init_midi_rate) {
        sendControlChange(EM_SettingsChannel, EMCC_Download, EM_DownloadItem::midiTxFull);
    }
}

void Hc1Module::transmitRequestUpdates()
{
    request_updates_state = InitState::Complete; // one-shot
    sendControlChange(EM_SettingsChannel, EMCC_Preserve, 1); // bit 1 means request config
}

void Hc1Module::transmitDeviceHello()
{
    device_hello_state = InitState::Pending;
#ifdef VERBOSE_LOG
    log_midi = true;
#endif
    beginPreset(); // force preset
    set_init_midi_rate();
    sendEditorPresent(false);
    //sendControlChange(EM_SettingsChannel, EMCC_Download, EM_DownloadItem::gridToFlash); // needed for pedals
    sendControlChange(EM_SettingsChannel, EMCC_Download, EM_DownloadItem::configToMidi);
}

void Hc1Module::transmitRequestConfiguration()
{
#ifdef VERBOSE_LOG
    log_midi = true;
#endif
    beginPreset(); // force preset
    config_state = InitState::Pending;
    set_init_midi_rate();
    sendEditorPresent(false);
    sendControlChange(EM_SettingsChannel, EMCC_Download, EM_DownloadItem::gridToFlash);
    sendControlChange(EM_SettingsChannel, EMCC_Download, EM_DownloadItem::configToMidi);
}

void Hc1Module::transmitRequestSystemPresets()
{
    clearCCValues();
    system_presets.clear();
    system_preset_state = InitState::Pending;
    // consider: save/restore EM MIDI routing to disable surface > midi/cvc to avoid interruption while loading
    set_init_midi_rate();
    sendControlChange(EM_SettingsChannel, EMCC_Download, EM_DownloadItem::sysToMidi);
}

void Hc1Module::transmitRequestUserPresets()
{
    clearCCValues();
    user_presets.clear();
    user_preset_state = InitState::Pending;
    // consider: save/restore EM MIDI routing to disable surface > midi/cvc to avoid interruption while loading
    set_init_midi_rate();
    sendControlChange(EM_SettingsChannel, EMCC_Download, EM_DownloadItem::userToMidi);
}

void Hc1Module::setPreset(std::shared_ptr<Preset> preset)
{
    beginPreset(); // force preset
    notifyPresetChanged();
    notifyPedalsChanged();

    current_preset = preset;
    if (!preset) return;

    set_init_midi_rate();
    config_state = InitState::Pending;
    sendEditorPresent(false);
    sendControlChange(EM_SettingsChannel, EMCC_Download, EM_DownloadItem::gridToFlash);
    sendControlChange(EM_SettingsChannel, MidiCC_BankSelect, preset->bank_hi);
    sendControlChange(EM_SettingsChannel, EMCC_Category, preset->bank_lo);
    sendProgramChange(EM_SettingsChannel, preset->number);
}

void Hc1Module::sendSavedPreset()
{
    if (!saved_preset) {
        saved_preset_state = InitState::Complete;
        return;
    }
    set_init_midi_rate();
    saved_preset_state = InitState::Pending;
    sendEditorPresent(false);
    sendControlChange(EM_SettingsChannel, EMCC_Download, EM_DownloadItem::gridToFlash);
    sendControlChange(EM_SettingsChannel, MidiCC_BankSelect, saved_preset->bank_hi);
    sendControlChange(EM_SettingsChannel, EMCC_Category, saved_preset->bank_lo);
    sendProgramChange(EM_SettingsChannel, saved_preset->number);
}

void Hc1Module::silence(bool reset)
{
    for (uint8_t channel = 0; channel < 12; ++channel) {
        if (reset) {
            sendControlChange(channel, MidiCC_Reset, 127);
        }
        sendControlChange(channel, MidiCC_AllSoundOff, 127);
        sendControlChange(channel, MidiCC_AllNotesOff, 127);
    }
}

void Hc1Module::sendEditorPresent(bool init_handshake)
{
    if (init_handshake) { handshake = InitState::Pending; }
    sendControlChange(EM_SettingsChannel, EMCC_EditorPresent, tick_tock ? 85 : 42);
    tick_tock = !tick_tock;
}

void Hc1Module::sendNoteOn(uint8_t channel, uint8_t note, uint8_t velocity)
{
    queueMidiMessage(uMidiMessage(MidiStatus_NoteOn|channel, note, velocity));
}

void Hc1Module::sendNoteOff(uint8_t channel, uint8_t note, uint8_t velocity)
{
    queueMidiMessage(uMidiMessage(MidiStatus_NoteOff|channel, note, velocity));
}

void Hc1Module::setMacroCCValue(int id, uint8_t value)
{
    auto pq = dynamic_cast<CCParamQuantity*>(getParamQuantity(id));
    if (pq) {
        assert(pq->high_resolution);
        pq->setValueSilent((value << 7) | pedal_fraction);
    }
    pedal_fraction = 0;
}

void Hc1Module::setRecirculatorCCValue(int id, uint8_t value)
{
    auto pq = dynamic_cast<CCParamQuantity*>(getParamQuantity(id));
    if (pq) {
        assert(!pq->high_resolution);
        pq->setValueSilent(value);
    }
}

void Hc1Module::clearPreset()
{
    current_preset = nullptr;
    preset0.clear();
    clearCCValues();

    rounding.clear();
    pedal1.clear();
    pedal2.clear();
    recirculator = 0;
    middle_c = 60;
}

void Hc1Module::beginPreset()
{
    in_preset = true;
    clearPreset();
    dsp[2] = dsp[1] = dsp[0] = 0;
}

void Hc1Module::onChannel16CC(uint8_t cc, uint8_t value)
{
    ch15_cc_value[cc] = value;
    switch (cc) {
        case MidiCC_BankSelect: 
            // When a sys preset is currently selected, this comes in as the edit slot,
            // so use parsing context to set it to the correct value for selecting the preset.
            preset0.bank_hi = in_sys_names ? 127 : value;
            break;

        case EMCC_Category:
            preset0.bank_lo = value;
            break;

        case EMCC_PedalType: {
            auto new_p1 = static_cast<PedalType>(value & 0x07);
            bool p1_change = new_p1 != pedal1.type;
            pedal1.type = new_p1;

            auto new_p2 = static_cast<PedalType>((value >> 3) & 0x07);
            bool p2_change = new_p2 != pedal2.type;
            pedal2.type = new_p2;

            if (!in_preset) {
                if (p1_change) notifyPedalChanged(0);
                if (p2_change) notifyPedalChanged(1);
            }
        } break;

        case EMCC_MiddleC:
            middle_c = value;
            break;

        case EMCC_TuningGrid: {
            auto new_value = static_cast<Tuning>(value);
            if (rounding.tuning != new_value) {
                rounding.tuning = new_value;
                notifyRoundingChanged();
            }
        } break;

        case EMCC_Pedal1CC:
            pedal1.cc = value;
            if (!in_preset) {
                notifyPedalChanged(0);
            }
            break;

        case EMCC_Pedal2CC:
            pedal2.cc = value;
            if (!in_preset) {
                notifyPedalChanged(0);
            }
            break;

        case EMCC_DataStream: {
            switch (value) {
                case EM_StreamType::Name: {
                    preset0.clear_name();
                    if (data_stream != -1) {
                        broken = true;
                    } else {
                        data_stream = value;
                        if (in_sys_names || in_user_names) {
                            beginPreset();
                        }
                    }
                } break;
 
                case EM_StreamType::ConText: {
                    preset0.clear_text();
                    if (data_stream != -1) {
                        broken = true;
                    } else {
                        data_stream = value;
                    }
                } break;

                // case EM_StreamType::Sys: {
                //     system_data.clear();
                //     data_stream = value;
                // } break;

                case EM_StreamType::DataEnd: {
                    switch (data_stream) {
                        case EM_StreamType::Name:
                            break;
                        case EM_StreamType::ConText:
                            if (!is_gathering_presets() && !broken) {
                                preset0.parse_text();
                            }
                            break;
                        // case EM_StreamType::Sys:
                        //     //DebugLog("End Sys data");
                        //     break;
                    }
                    data_stream = -1;
                } break;

                default:
                    break;
            }

        } break;

        case EMCC_Reverse_Rounding: {
            reverse_surface = static_cast<bool>(value & 1);

            auto kind = static_cast<RoundKind>((value & 0x06) >>1);
            if (kind != rounding.kind) {
                rounding.kind = kind;
                notifyRoundingChanged();
            }
        } break;

        case EMCC_RecirculatorType:
            recirculator = value;
            getParamQuantity(RECIRC_EXTEND_PARAM)->setValue(isExtendRecirculator() * 1.f);
            break;

        case EMCC_MinPedal1:
            pedal1.min = value;
            if (!in_preset) {
                notifyPedalChanged(0);
            }
            break;
        case EMCC_MaxPedal1:
            pedal1.max = value;
            if (!in_preset) {
                notifyPedalChanged(0);
            }
            break;
        case EMCC_MinPedal2:
            pedal2.min = value;
            if (!in_preset) {
                notifyPedalChanged(1);
            }
            break;
        case EMCC_MaxPedal2:
            pedal2.max = value;
            if (!in_preset) {
                notifyPedalChanged(1);
            }
            break;

        case EMCC_VersionHigh:
            firmware_version = value;
            break;

        case EMCC_VersionLow:
            firmware_version = (firmware_version << 7) | value;
            break;

        case EMCC_CVCHi_Hardware:
            cvc_version = static_cast<uint16_t>(value & 0x03) << 14;
            hardware = static_cast<EM_Hardware>((value & 0x7c) >> 2);
            break;

        case EMCC_CVCMid:
            cvc_version |= static_cast<uint16_t>(value) << 7;
            break;

        case EMCC_CVCLo:
            cvc_version |= static_cast<uint16_t>(value);
            break;

        case EMCC_Download:
            //download_message_id = value;
            switch (value) {
                case EM_DownloadItem::beginUserNames:
                    in_user_names = true;
                    break;

                case EM_DownloadItem::endUserNames:
                    in_user_names = false;
                    user_preset_state = broken ? InitState::Broken : InitState::Complete;
                    if (!broken) {
                        std::sort(user_presets.begin(), user_presets.end(), preset_system_order);
                    }
                    restore_midi_rate();
                    heart_time = post_user_delay;
                    break;

                case EM_DownloadItem::beginSysNames:
                    in_sys_names = true;
                    break;

                case EM_DownloadItem::endSysNames:
                    in_sys_names = false;
                    system_preset_state = broken ? InitState::Broken : InitState::Complete;
                    if (!broken) {
                        std::sort(system_presets.begin(), system_presets.end(), getPresetSort(preset_order));
                        readFavorites();
                    }
                    restore_midi_rate();
                    heart_time = post_system_delay;
                    break;
            }
            break;

        case EMCC_Status: {
            StatusItem led = static_cast<StatusItem>(value & StatusItem::sLedBits);
            if (led) {
                getLight(HEART_LIGHT).setBrightness(0.5f);
                ledColor = StatusColor(led);
            } else {
                getLight(HEART_LIGHT).setBrightness(0.0f);
            }
        } break;

        case EMCC_DSP: {
            auto d = (value >> 5) -1;
            auto pct = (value & 0x1F) * 4;
            dsp[d] = pct;
        } break;

        case EMCC_EditorReply:{
            if (InitState::Pending == handshake) {
                first_beat = true;
                handshake = InitState::Complete;
            } else {
                //DebugLog("Extra Editor Reply");
            }
        } break;
    }
}

void Hc1Module::onChannel16Message(const midi::Message& msg)
{
    auto status = GetRawStatus(msg);
    assert(status & 0x80);
    switch (status) {
        case MidiStatus_CC:
            onChannel16CC(GetCC(msg), msg.getValue());
            break;

        case MidiStatus_ProgramChange:
            if (in_preset) {
                in_preset = false;
                preset0.number = msg.getNote();
                if (savedPresetPending()) {
#ifdef VERBOSE_LOG
                    if (0 != saved_preset->name.compare(preset0.name())) {
                        DEBUG("Unexpected: saved %s != set: %s", saved_preset->name.c_str(), preset0.name());
                    }
#endif
                    restore_midi_rate();
                    current_preset = findDefinedPreset(saved_preset);
                    saved_preset_state = config_state = InitState::Complete;
                    notifyPresetChanged();
                    notifyPedalsChanged();
                } else
                if (deviceHelloPending()) {
#ifdef VERBOSE_LOG
                    log_midi = false;
#endif
                    restore_midi_rate();
                    device_hello_state = InitState::Complete;
                    notifyDeviceChanged();
                    notifyPresetChanged();
                    notifyPedalsChanged();
                } else
                if (configPending()) {
#ifdef VERBOSE_LOG
                    log_midi = false;
#endif
                    restore_midi_rate();
                    config_state = broken ? InitState::Broken : InitState::Complete;
                    if (!broken) {
                        if (!preset0.name_empty()) {
                            if (nullptr == current_preset || current_preset->is_same_preset(preset0)) {
                                current_preset = findDefinedPreset(nullptr);
                            }
                        }
                    }
                    notifyPresetChanged();
                    notifyPedalsChanged();
                } else
                if (is_gathering_presets()) {
                    if (broken) {
                        restore_midi_rate();
                    }
                    if (!broken && !preset0.name_empty()) {
                        std::string name = preset0.name();
                        if ((126 != preset0.bank_hi)
                            && "-" != name
                            && "--" != name
                            && "Empty" != name)
                        {
                            std::shared_ptr<Preset> preset = std::make_shared<Preset>(preset0);
                            //preset->ensure_category_list();
                            if (in_user_names) {
                                assert(!preset->isSysPreset());
                                user_presets.push_back(preset);
                            } else {
                                assert(in_sys_names);
                                assert(preset->isSysPreset());
                                system_presets.push_back(preset);
                            }
                        }
                    }
                }
            }
            break;

        case MidiStatus_ChannelPressure:
            if (in_preset && !broken) {
                switch (data_stream) {
                    case EM_StreamType::Name:
                        preset0.build_name(msg.bytes[1]);
                        break;
                    case EM_StreamType::ConText:
                        preset0.build_text(msg.bytes[1]);
                        break;
                }
            }
            // if (EM_StreamType::Sys == data_stream) {
            //     system_data.push_back(msg.bytes[1]);
            // }
            break;
        case MidiStatus_PitchBend:
            break;

        case MidiStatus_SysEx:
        case MidiStatus_SysExEnd:
            break;
    }
}

void Hc1Module::onNoteOn(uint8_t channel, uint8_t note, uint8_t velocity)
{
    this->note = note;
    if (velocity) {
        notesOn++;
    } else {
        notesOn--;
    }
}

void Hc1Module::onNoteOff(uint8_t channel, uint8_t note, uint8_t velocity)
{
    if (notesOn) { notesOn--; }
}

void Hc1Module::onSoundOff()
{
    if (!in_preset) {
        current_preset = nullptr;
        beginPreset();
        if (saved_preset_state != InitState::Pending) {
            config_state = InitState::Pending;
        }
    }
}

void Hc1Module::onChannelOneCC(uint8_t cc, uint8_t value)
{
    ch0_cc_value[cc] = value;

    // Continuum doesn't send high resolution for pedals
    if (cc == pedal1.cc) {
        pedal1.value = value;
        if (!in_preset) {
            notifyPedalChanged(0);
        }
    } else if (cc == pedal2.cc) {
        pedal2.value = value;
        if (!in_preset) {
            notifyPedalChanged(1);
        }
    }

    switch (cc) {
        // case EMCC_Jack1:
        //     DEBUG("EMCC_Jack1 %d", value);
        //     break;
        // case EMCC_Jack2:
        //     DEBUG("EMCC_Jack2 %d", value);
        //     break;
        // case MidiCC_Hold:
        //     DEBUG("MidiCC_Hold %d", value);
        //     break;

        case EMCC_PedalFraction: 
            pedal_fraction =  value; // setMacroCCValue consumes pedal_fraction
            break;

        case EMCC_i:   setMacroCCValue(M1_PARAM, value); break;
        case EMCC_ii:  setMacroCCValue(M2_PARAM, value); break;
        case EMCC_iii: setMacroCCValue(M3_PARAM, value); break;
        case EMCC_iv:  setMacroCCValue(M4_PARAM, value); break;
        case EMCC_v:   setMacroCCValue(M5_PARAM, value); break;
        case EMCC_vi:  setMacroCCValue(M6_PARAM, value); break;

        case EMCC_R1:   setRecirculatorCCValue(R1_PARAM, value); break;
        case EMCC_R2:   setRecirculatorCCValue(R2_PARAM, value); break;
        case EMCC_R3:   setRecirculatorCCValue(R3_PARAM, value); break;
        case EMCC_R4:   setRecirculatorCCValue(R4_PARAM, value); break;
        case EMCC_RMIX: setRecirculatorCCValue(RMIX_PARAM, value); break;

        case EMCC_PostLevel: setMacroCCValue(VOLUME_PARAM, value); break;

        case EMCC_RoundRate:{
            if (rounding.rate != value) {
                rounding.rate = value;
                notifyRoundingChanged();
            }
        } break;

        case EMCC_RoundInitial: {
            bool new_initial = value; // seems to be 0 or 127
            if (rounding.initial != new_initial) {
                rounding.initial = new_initial;
                notifyRoundingChanged();
            }
        } break;

        case EMCC_RoundEqual: {
            RoundEqual rq = rounding.equal;
            switch (value) {
            case   0: rq = RoundEqual::Disabled; break;
            case  64: rq = RoundEqual::Enabled; break;
            case 127: rq = RoundEqual::Equal; break;
            }
            if (rq != rounding.equal) {
                rounding.equal = rq;
                notifyRoundingChanged();
            }
        } break;

        case MidiCC_AllSoundOff: onSoundOff(); break;
    }
}

void Hc1Module::onChannelOneMessage(const midi::Message& msg)
{
    switch (GetRawStatus(msg)) {
        case MidiStatus_NoteOff:
            onNoteOff(0, msg.bytes[1], msg.bytes[2]);
            break;
        case MidiStatus_NoteOn:
            onNoteOn(0, msg.bytes[1], msg.bytes[2]);
            break;
        case MidiStatus_CC:
            onChannelOneCC(msg.bytes[1], msg.bytes[2]);
            break;
    }
}

bool isLoopbackDetect(const midi::Message& msg) {
    auto channel = msg.getChannel();
    if (channel == 0 || channel == 15) {
        return (msg.getNote() == 117) && (MidiStatus_CC == GetRawStatus(msg));
    }
    return false;
}

void Hc1Module::onMidiMessage(uMidiMessage umsg)
{
    auto msg = umsg.toMidiMessage();
#ifdef VERBOSE_LOG
    if (log_midi) {
        DebugLog("%s", ToFormattedString(msg).c_str());
    }
#endif
    bool loopbackmsg = !isLoopbackDetect(msg);
    if (broken && loopbackmsg) {
        broken_idle = 0.f;
    }

    ++midi_receive_count;

    auto channel = msg.getChannel();
    switch (channel) {
    case EM_MasterChannel: // 0 (channel 1)
        onChannelOneMessage(msg);
        break;

    case EM_KentonChannel: // 13 (channel 14)
        break;

    case EM_MatrixChannel:  // 14 (channel 15)
        break;

    case EM_SettingsChannel: // 15 (channel 16)
        onChannel16Message(msg);
        break;

    default:
        switch (GetRawStatus(msg)) {
        case MidiStatus_NoteOff:
            onNoteOff(channel, msg.bytes[1], msg.bytes[2]);
            break;
        case MidiStatus_NoteOn:
            onNoteOn(channel, msg.bytes[1], msg.bytes[2]);
            break;
        case MidiStatus_ChannelPressure:
        case MidiStatus_PitchBend:
            break;
        }
        break;
    }
}

void Hc1Module::onMessage(const midi::Message& msg)
{
    midi_input_worker->post_message(uMidiMessage(msg));
}

// ======================================================
void MidiInputWorker::start()
{
    my_thread = std::thread(&MidiInputWorker::run, this);
}

void MidiInputWorker::post_quit()
{
    std::unique_lock<std::mutex> lock(m);
    stop = true;
    cv.notify_one();
}

void MidiInputWorker::pause()
{
    std::unique_lock<std::mutex> lock(m);
    midi_consume.clear();
    pausing = true;
    cv.notify_one();
}

void MidiInputWorker::resume()
{
    std::unique_lock<std::mutex> lock(m);
    pausing = false;
    cv.notify_one();
}

void MidiInputWorker::post_message(uMidiMessage msg)
{
    if (stop || pausing) return;
    std::unique_lock<std::mutex> lock(m);
    midi_consume.push(msg);
    cv.notify_one();
}

void MidiInputWorker::run() {
    contextSet(context);
	system::setThreadName("Midi Input worker");
    while (1)
    {
        {
            std::unique_lock<std::mutex> lock(m);
            cv.wait(lock, [this]{ return stop || !midi_consume.empty(); });
            if (stop) {
                return;
            }
            if (pausing) {
                continue;
            }
        }
        while (!midi_consume.empty()) {
            auto msg = midi_consume.shift();
            if (msg.channel() == 0 || msg.channel() == 15) {
                hc1->onMidiMessage(msg);
            } else {
                switch (msg.status()){
                    case MidiStatus_NoteOn:
                    case MidiStatus_NoteOff:
                    case MidiStatus_ChannelPressure:
                    case MidiStatus_PitchBend:
                        hc1->onMidiMessage(msg);
                        break;
                }
            }
        }
    }
}



}