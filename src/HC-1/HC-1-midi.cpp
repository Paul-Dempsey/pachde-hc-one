#include "HC-1.hpp"
#include "../module_broker.hpp"
#include "../widgets/cc_param.hpp"
#include "../em_device.hpp"

namespace pachde {

void Hc1Module::onRenewClaim()
{
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
                midi_output.reset();
                midi_dispatch.clear();
                midi_output.setDeviceId(connection->output_device_id);
                assert((connection->output_device_id == midi_output.getDeviceId())); // subscribing failed: should handle it?
                midi_output.setChannel(-1);

                heart_time = 5.f;
                device_input_state = InitState::Uninitialized;
                Input::reset();
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
        DebugLog("MIDI Output disabled when duplicate");
        return;
    }
    if (midi_dispatch.full()) {
        DebugLog("MIDI Output queue full: rebooting");
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

void Hc1Module::transmitRequestUpdates()
{
    DebugLog("Request updates");
    request_updates_state = InitState::Complete; // one-shot
    sendControlChange(EM_SettingsChannel, EMCC_Preserve, 1); // bit 1 means request config
}

void Hc1Module::transmitDeviceHello()
{
    DebugLog("Device hello (config)");
    device_hello_state = InitState::Pending;
#ifdef VERBOSE_LOG
    log_midi = true;
#endif
    beginPreset(); // force preset
    sendEditorPresent(false);
    //sendControlChange(EM_SettingsChannel, EMCC_Download, EM_DownloadItem::gridToFlash); // needed for pedals
    sendControlChange(EM_SettingsChannel, EMCC_Download, EM_DownloadItem::configToMidi);
}

void Hc1Module::transmitRequestConfiguration()
{
    DebugLog("Request configuration");
    clearCCValues();
    current_preset = nullptr;
    config_state = InitState::Pending;
#ifdef VERBOSE_LOG
    log_midi = true;
#endif
    beginPreset(); // force preset
    sendEditorPresent(false);
    sendControlChange(EM_SettingsChannel, EMCC_Download, EM_DownloadItem::gridToFlash);
    //sendControlChange(EM_SettingsChannel, EMCC_Download, EM_DownloadItem::gridToFlash);
    sendControlChange(EM_SettingsChannel, EMCC_Download, EM_DownloadItem::configToMidi);
}

void Hc1Module::transmitRequestSystemPresets()
{
    DebugLog("Request system presets");
    clearCCValues();
    system_presets.clear();
    system_preset_state = InitState::Pending;
    // consider: save/restore EM MIDI routing to disable surface > midi/cvc to avoid interruption while loading
//    sendControlChange(EM_SettingsChannel, EMCC_Download, EM_DownloadItem::midiTxTweenth);
//    sendControlChange(EM_SettingsChannel, EMCC_Download, EM_DownloadItem::midiTxThird);
    sendControlChange(EM_SettingsChannel, EMCC_Download, EM_DownloadItem::sysToMidi);
}

void Hc1Module::transmitRequestUserPresets()
{
    DebugLog("Request user presets");
    clearCCValues();
    user_presets.clear();
    user_preset_state = InitState::Pending;
    //silence(false);
    // consider: save/restore EM MIDI routing to disable surface > midi/cvc to avoid interruption while loading
    sendControlChange(EM_SettingsChannel, EMCC_Download, EM_DownloadItem::userToMidi);
}

void Hc1Module::setPreset(std::shared_ptr<Preset> preset)
{
    current_preset = nullptr;
    notifyPresetChanged();
    notifyPedalsChanged();

    current_preset = preset;
    if (!preset) return;

    DebugLog("Setting preset [%s]", preset ? preset->describe_short().c_str(): "(none)");
    sendEditorPresent(false);
    sendControlChange(15, EMCC_Download, EM_DownloadItem::gridToFlash);

    config_state = InitState::Pending;
    sendControlChange(15, MidiCC_BankSelect, preset->bank_hi);
    sendControlChange(15, EMCC_Category, preset->bank_lo);
    sendProgramChange(15, preset->number);
}

void Hc1Module::sendSavedPreset() {
    if (!saved_preset) {
        DebugLog("No saved preset");
        saved_preset_state = InitState::Complete;
        return;
    }
    DebugLog("Sending saved preset [%s]", saved_preset->name.c_str());
    sendEditorPresent(false);
    sendControlChange(EM_SettingsChannel, EMCC_Download, EM_DownloadItem::gridToFlash);

    saved_preset_state = InitState::Pending;
    sendControlChange(15, MidiCC_BankSelect, saved_preset->bank_hi);
    sendControlChange(15, EMCC_Category, saved_preset->bank_lo);
    sendProgramChange(15, saved_preset->number);
}

void Hc1Module::silence(bool reset)
{
    for (uint8_t channel = 0; channel < 12; ++channel) {
        sendControlChange(channel, MidiCC_AllSoundOff, 127);
        sendControlChange(channel, MidiCC_AllNotesOff, 127);
        if (reset) {
            sendControlChange(channel, MidiCC_Reset, 127);
        }
    }
}

void Hc1Module::sendEditorPresent(bool init_handshake)
{
    DebugLog("Editor present");
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

void Hc1Module::beginPreset()
{
    in_preset = true;
    preset0.clear();
    //clearCCValues();
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
                        DebugLog("!!!! BROKEN !!!!");
                        broken = true;
                    } else {
                        //DebugLog("Begin name");
                        data_stream = value;
                        if (in_sys_names || in_user_names) {
                            beginPreset();
                        }
                    }
                } break;
 
                case EM_StreamType::ConText: {
                    preset0.clear_text();
                    if (data_stream != -1) {
                        DebugLog("!!!! BROKEN !!!!");
                        broken = true;
                    } else {
                        //DebugLog("Begin Text");
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
                            //DebugLog("End name");
                            break;
                        case EM_StreamType::ConText:
                            //DebugLog("End Text");
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
                    DebugLog("[---- beginUserNames ----]");
                    in_user_names = true;
                    break;

                case EM_DownloadItem::endUserNames:
                    DebugLog("[---- endUserNames ----]");
                    in_user_names = false;
                    user_preset_state = broken ? InitState::Broken : InitState::Complete;
                    if (!broken) {
                        std::sort(user_presets.begin(), user_presets.end(), preset_system_order);
                    }
                    DebugLog("End User presets as %s", InitStateName(user_preset_state));
                    //sendControlChange(EM_SettingsChannel, EMCC_Download, EM_DownloadItem::midiTxFull);
                    break;

                case EM_DownloadItem::beginSysNames:
                    DebugLog("[---- beginSysNames ----]");
                    in_sys_names = true;
                    break;

                case EM_DownloadItem::endSysNames:
                    DebugLog("[---- endSysNames ----]");
                    in_sys_names = false;
                    system_preset_state = broken ? InitState::Broken : InitState::Complete;
                    if (!broken) {
                        std::sort(system_presets.begin(), system_presets.end(), getPresetSort(preset_order));
                        readFavorites();
                    }
                    DebugLog("End System presets as %s", InitStateName(system_preset_state));
                    //sendControlChange(EM_SettingsChannel, EMCC_Download, EM_DownloadItem::midiTxFull);
                    heart_time = 5.f;
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
            //DebugLog("DSP %d = %d%%", d, pct);
        } break;

        case EMCC_EditorReply:{
            DebugLog("Editor Reply");
            if (InitState::Pending == handshake) {
                first_beat = true;
                handshake = InitState::Complete;
            } else {
                DebugLog("Extra Editor Reply");
            }
        } break;
    }
}

void Hc1Module::onChannel16Message(const midi::Message& msg)
{
    auto status = GetRawStatus(msg);
    assert(status & 0x80);
    switch (status) {
        case MidiStatus_NoteOff:
        case MidiStatus_NoteOn:
            //DebugLog("Note unexpected on channel 16");
            break;

        case MidiStatus_PolyKeyPressure:
            break;

        case MidiStatus_CC:
            onChannel16CC(GetCC(msg), msg.getValue());
            break;

        case MidiStatus_ProgramChange:
            if (in_preset) {
                //DebugLog("---- END PRESET ----");
                in_preset = false;
                preset0.number = msg.getNote();
                //DebugLog("%s", preset0.describe(false).c_str());
                if (savedPresetPending()) {
#ifdef VERBOSE_LOG
                    if (0 != saved_preset->name.compare(preset0.name())) {
                        DEBUG("Unexpected: saved %s != set: %s", saved_preset->name.c_str(), preset0.name());
                    }
#endif
                    current_preset = findDefinedPreset(saved_preset);
                    saved_preset_state = config_state = InitState::Complete;
                    notifyPresetChanged();
                    notifyPedalsChanged();
                } else
                if (deviceHelloPending()) {
#ifdef VERBOSE_LOG
                    log_midi = false;
#endif
                    device_hello_state = InitState::Complete;
                    if (!cache_system_presets || (hardware == EM_Hardware::Unknown)) {
                        heart_time = 4.f;
                    }
                } else
                if (configPending()) {
#ifdef VERBOSE_LOG
                    log_midi = false;
#endif
                    config_state = broken ? InitState::Broken : InitState::Complete;
                    if (!broken) {
                        if (!preset0.name_empty()) {
                            if (nullptr == current_preset) {
                                current_preset = findDefinedPreset(nullptr);
                            } else {
                                if (!current_preset->is_same_preset(preset0)) {
                                    DebugLog("Expected %s == %s",
                                        preset0.describe_short().c_str(),
                                        current_preset->describe_short().c_str()
                                        );
                                    current_preset = findDefinedPreset(nullptr);
                                }
                            }
                        }
                    }
                    DebugLog("End config as %s with %s", InitStateName(config_state), current_preset ? current_preset->describe_short().c_str() : "nuthin");
                    notifyPresetChanged();
                    notifyPedalsChanged();
                } else
                if (!broken && is_gathering_presets()) {
                    if (!preset0.name_empty()) {
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
    heart_phase = 0;
    heart_time = 5.f;
}

void Hc1Module::onNoteOff(uint8_t channel, uint8_t note, uint8_t velocity)
{
    if (notesOn) { notesOn--; }
    heart_phase = 0;
    heart_time = 5.f;
}

void Hc1Module::onSoundOff()
{
    if (!in_preset) {
        clearCCValues();
        beginPreset();
        if (saved_preset_state != InitState::Pending) {
            DebugLog("Config pending (update)");
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
        // case EMCC_Pedal1:
        //     DEBUG("EMCC_Pedal1 %d", value);
        //     break;
        // case EMCC_Pedal2:
        //     DEBUG("EMCC_Pedal2 %d", value);
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

void Hc1Module::onMessage(const midi::Message& msg)
{
#ifdef VERBOSE_LOG
    if (log_midi) {
        DebugLog("%lld %s", static_cast<long long int>(msg.frame), ToFormattedString(msg).c_str());
    }
#endif
#if defined PERIODIC_DEVICE_CHECK
    device_check.reset();
#endif
    if (broken && !isLoopbackDetect(msg)) {
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
                case MidiStatus_CC:
                    break;
                case MidiStatus_NoteOff:
                    onNoteOff(channel, msg.bytes[1], msg.bytes[2]);
                    break;
                case MidiStatus_NoteOn:
                    onNoteOn(channel, msg.bytes[1], msg.bytes[2]);
                    break;
            }
            break;
    }
}

}