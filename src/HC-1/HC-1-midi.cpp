#include "HC-1.hpp"
#include "cc_param.hpp"

namespace pachde {

void Hc1Module::sendControlChange(uint8_t channel, uint8_t cc, uint8_t value)
{
    midi::Message msg;
    SetCC(msg, channel, cc, value);
    midiOutput.sendMessage(msg);
}

void Hc1Module::sendProgramChange(uint8_t channel, uint8_t program)
{
    midi::Message msg;
    SetProgramChange(msg, channel, program);
    midiOutput.sendMessage(msg);
}

void Hc1Module::sendResetAllreceivers()
{
    midi::Message msg;
    msg.bytes[0] = 0xff;
    msg.bytes.resize(1);
    midiOutput.sendMessage(msg);
}

void Hc1Module::transmitRequestUpdates()
{
    DebugLog("Request updates");
    requested_updates = InitState::Complete; // one-shot
    sendControlChange(EM_SettingsChannel, EMCC_Preserve, 1);
}

void Hc1Module::transmitRequestConfiguration()
{
    DebugLog("Request configuration");
    clearCCValues();
    current_preset = nullptr;
    config_state = InitState::Pending;
#ifdef VERBOSE_LOG
    //log_midi = true;
#endif
    beginPreset(); // force preset
    sendControlChange(EM_SettingsChannel, EMCC_Download, EM_DownloadItem::configToMidi);
}

void Hc1Module::transmitRequestPresets()
{
    DebugLog("Request presets");
    clearCCValues();
    user_presets.clear();
    system_presets.clear();
    favorite_presets.clear();
    preset_state = InitState::Pending;
    //silence(false);
    // todo: save/restore EM MIDI routing to disable surface > midi/cvc to avoid interruption while loading
    sendControlChange(EM_SettingsChannel, EMCC_Download, EM_DownloadItem::midiTxTweenth);
    sendControlChange(EM_SettingsChannel, EMCC_Download, EM_DownloadItem::sysToMidi);
}

void Hc1Module::setPreset(std::shared_ptr<MinPreset> preset)
{
    current_preset = preset;
    if (!preset) return;

#if defined VERBOSE_LOG
    DebugLog("Setting preset [%s]", preset ? preset->describe_short().c_str(): "(none)");
#endif
    sendControlChange(15, MidiCC_BankSelect, preset->bank_hi);
    sendControlChange(15, EMCC_Category, preset->bank_lo);
    sendProgramChange(15, preset->number);
    config_state = InitState::Pending;
}

void Hc1Module::sendSavedPreset() {
    if (!saved_preset) {
        DebugLog("No saved preset");
        saved_preset_state = InitState::Complete;
        return;
    }
    DebugLog("Sending saved preset [%s]", saved_preset->name.c_str());
    saved_preset_state = InitState::Pending;
    heart_time = .25f;
    sendControlChange(15, MidiCC_BankSelect, saved_preset->bank_hi);
    sendControlChange(15, EMCC_Category, saved_preset->bank_lo);
    sendProgramChange(15, saved_preset->number);
}

void Hc1Module::silence(bool reset)
{
    for (uint8_t channel = 0; channel < 12; ++channel) {
        if (reset) {
            sendControlChange(channel, MidiCC_Reset, 0);
        }
        sendControlChange(channel, MidiCC_AllSoundOff, 0);
    }
}

void Hc1Module::transmitInitDevice()
{
    DebugLog("INIT DEVICE (Editor present)");
    //sendResetAllreceivers();
    //silence(true);
    device_state = InitState::Pending;
    sendControlChange(EM_SettingsChannel, EMCC_EditorPresent, tick_tock ? 85 : 42);
    tick_tock = !tick_tock;
    heart_time = 0.25f;
}

void Hc1Module::sendEditorPresent()
{
    DebugLog("Editor present");
    handshake = InitState::Pending;
    sendControlChange(EM_SettingsChannel, EMCC_EditorPresent, tick_tock ? 85 : 42);
    tick_tock = !tick_tock;
    //download_message_id = -1;
}

void Hc1Module::sendNoteOn(uint8_t channel, uint8_t note, uint8_t velocity)
{
    midi::Message msg;
    SetNoteOn(msg, channel, note, velocity);
    midiOutput.sendMessage(msg);
}

void Hc1Module::sendNoteOff(uint8_t channel, uint8_t note, uint8_t velocity)
{
    midi::Message msg;
    SetNoteOff(msg, channel, note, velocity);
    midiOutput.sendMessage(msg);
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

void Hc1Module::handle_ch16_cc(uint8_t cc, uint8_t value)
{
    ch15_cc_value[cc] = value;
    switch (cc) {
        case MidiCC_BankSelect: 
            // When sys preset is currently selected, this comes in as the edit slot,
            // so use parsing context to set it to the correct value for selecting the preset.
            preset0.bank_hi = in_sys_names ? 127 : value;
            break;

        case EMCC_Category:
            preset0.bank_lo = value;
            break;

        case EMCC_RecirculatorType:
            recirculator = value;
            break;

        case EMCC_DataStream: {
            switch (value) {
                case EM_StreamType::Name: {
                    preset0.clear_name();
                    if (data_stream != -1) {
                        DebugLog("!!!! BROKEN !!!!");
                        broken = true;
                        sendControlChange(EM_SettingsChannel, EMCC_Download, EM_DownloadItem::midiTxFull);
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
                        sendControlChange(EM_SettingsChannel, EMCC_Download, EM_DownloadItem::midiTxFull);
                    } else {
                        //DebugLog("Begin Text");
                        data_stream = value;
                    }
                } break;

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
                    }
                    data_stream = -1;
                } break;

                default:
                    break;
            }

        } break;

        case EMCC_VersionHigh:
            firmware_version = value;
            break;

        case EMCC_VersionLow:
            firmware_version = (firmware_version << 7) | value;
            break;

        case EMCC_Download:
            download_message_id = value;
            switch (value) {
                case EM_DownloadItem::beginUserNames:
                    DebugLog("[---- beginUserNames ----]");
                    in_user_names = true;
                    break;

                case EM_DownloadItem::endUserNames:
                    DebugLog("[---- endUserNames ----]");
                    if (!broken) {
                        std::sort(user_presets.begin(), user_presets.end(), preset_order);
                        std::sort(system_presets.begin(), system_presets.end(), preset_order);
                        readFavorites();
                    }
                    sendControlChange(EM_SettingsChannel, EMCC_Download, EM_DownloadItem::midiTxFull);
                    preset_state = broken ? InitState::Broken : InitState::Complete;
                    DebugLog("End Presets as %s", InitStateName(preset_state));
                    in_user_names = false;
                    break;

                case EM_DownloadItem::beginSysNames:
                    DebugLog("[---- beginSysNames ----]");
                    in_sys_names = true;
                    break;

                case EM_DownloadItem::endSysNames:
                    DebugLog("[---- endSysNames ----]");
                    in_sys_names = false;
                    // chain to user names
                    if (!broken) {
                        sendControlChange(EM_SettingsChannel, EMCC_Download, EM_DownloadItem::userToMidi);
                    }
                    break;
            }
            break;

        case EMCC_Status: {
            StatusItem led = static_cast<StatusItem>(value & StatusItem::sLedBits);
            if (led) {
                getLight(HEART_LIGHT).setBrightness(0.2f);
                ledColor = StatusColor(led);
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
            if (InitState::Pending == device_state) {
                device_state = InitState::Complete;
            } else if (InitState::Pending == handshake) {
                handshake = InitState::Complete;
            } else {
                DebugLog("Extra Editor Reply");
            }
        } break;
    }
}

void Hc1Module::handle_ch16_message(const midi::Message& msg)
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
            handle_ch16_cc(GetCC(msg), msg.getValue());
            break;

        case MidiStatus_ProgramChange:
            if (in_preset) {
                //DebugLog("---- END PRESET ----");
                in_preset = false;
                preset0.number = msg.getNote();
                //DebugLog("%s", preset0.describe(false).c_str());
                if (savedPresetPending()) {
                    assert(0 == saved_preset->name.compare(preset0.name()));
                    current_preset = findSysUserPreset(saved_preset);
                    saved_preset_state = config_state = InitState::Complete;
                } else if (configPending()) {
#ifdef VERBOSE_LOG
                    log_midi = false;
#endif
                    config_state = broken ? InitState::Broken : InitState::Complete;
                    if (!broken) {
                        if (!preset0.name_empty()) {
                            if (nullptr == current_preset) {
                                current_preset = findSysUserPreset(nullptr);
                            } else {
                                if (!current_preset->isSamePreset(preset0)) {
                                    DebugLog("Expected %s == %s",
                                        preset0.describe_short().c_str(),
                                        current_preset->describe_short().c_str()
                                        );
                                    current_preset = findSysUserPreset(nullptr);
                                }
                            }
                        }
                    }
                    DebugLog("End config as %s with %s", InitStateName(config_state), current_preset ? current_preset->describe_short().c_str() : "nuthin");
                } else if (!broken && is_gathering_presets()) {
                    if (!preset0.name_empty()) {
                        std::string name = preset0.name();
                        if ((126 != preset0.bank_hi)
                            && "-" != name
                            && "--" != name
                            && "Empty" != name)
                        {
                            std::shared_ptr<MinPreset> preset = std::make_shared<MinPreset>(preset0);
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
    notesOn--;
    heart_phase = 0;
    heart_time = 5.f;
}

void Hc1Module::onSoundOff()
{
    if (!in_preset) {
        clearCCValues();
        beginPreset();
        DebugLog("Config pending (update)");
        config_state = InitState::Pending;
    }
}

void Hc1Module::onChannel0CC(uint8_t cc, uint8_t value)
{
    ch0_cc_value[cc] = value;
    switch (cc) {
        case EMCC_PedalFraction: 
            pedal_fraction =  value;
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

        case MidiCC_AllSoundOff: onSoundOff(); break;
    }
}

void Hc1Module::handle_ch0_message(const midi::Message& msg)
{
    switch (GetRawStatus(msg)) {
        case MidiStatus_NoteOff:
            onNoteOff(0, msg.bytes[1], msg.bytes[2]);
            break;
        case MidiStatus_NoteOn:
            onNoteOn(0, msg.bytes[1], msg.bytes[2]);
            break;
        case MidiStatus_CC:
            onChannel0CC(msg.bytes[1], msg.bytes[2]);
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
    if (broken && !isLoopbackDetect(msg)) {
        broken_idle = 0.f;
    }
    ++midi_receive_count;
    
    auto channel = msg.getChannel();
    switch (channel) {
        case EM_MasterChannel:
            handle_ch0_message(msg);
            break;

        case EM_KentonChannel:
            break;

        case EM_MatrixChannel:
            break;

        case EM_SettingsChannel:
            handle_ch16_message(msg);
            break;

        default:
            switch (GetRawStatus(msg)) {
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