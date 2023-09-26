#include "HC-1.hpp"
#include "../cc_param.hpp"
#include "../module_broker.hpp"

namespace pachde {

void Hc1Module::setMidiDevice(int id)
{
    if (-1 == id) {
        resetMidiIO();
        device_name = "(no Eagan Matrix device)";
        return;
    }
    output_device_id = id;
    midi_output.setDeviceId(id);
    device_name = midi_output.getDeviceName(id);
    int id_in = findMatchingInputDevice(device_name);
    midi::Input::setDeviceId(id_in);
    input_device_id = id_in;
    device_output_state = id == -1 ? InitState::Uninitialized : InitState::Complete;
    device_input_state = id_in == -1 ? InitState::Uninitialized : InitState::Complete;
    notifyDeviceChanged();
    checkDuplicate();
}

void Hc1Module::resetMidiIO()
{
    device_name.clear();

    midi_dispatch.clear();

    midi::Input::reset();
    input_device_id = -1;
    device_input_state = InitState::Uninitialized;

    midi_output.reset();
    output_device_id = -1;
    device_output_state = InitState::Uninitialized;

    dupe = false;
    duplicate_instance = InitState::Uninitialized;

    notifyDeviceChanged();
}

void Hc1Module::checkDuplicate()
{
    dupe = false;
    auto one = ModuleBroker::get();
    if (one->Hc1count() > 1) {
        one->scan_while([=](Hc1Module* const& m) { 
            if (m != this
                && this->output_device_id >= 0 
                && (this->output_device_id == m->output_device_id)) {
                dupe = true;
                return false;
            }
            return true;
        });
    } 
    duplicate_instance = InitState::Complete;
}

void Hc1Module::queueMidiMessage(uMidiMessage msg)
{
    if (dupe) {
        DebugLog("MIDI Output disabled when duplicate");
        return;
    }
    if (midi_dispatch.full()) {
        DebugLog("MIDI Output queue full: resetting MIDI IO");
        resetMidiIO();
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

void Hc1Module::transmitRequestSystemPresets()
{
    DebugLog("Request system presets");
    clearCCValues();
    system_presets.clear();
    system_preset_state = InitState::Pending;
    //silence(false);
    // todo: save/restore EM MIDI routing to disable surface > midi/cvc to avoid interruption while loading
//    sendControlChange(EM_SettingsChannel, EMCC_Download, EM_DownloadItem::midiTxTweenth);
    sendControlChange(EM_SettingsChannel, EMCC_Download, EM_DownloadItem::sysToMidi);
}

void Hc1Module::transmitRequestUserPresets()
{
    DebugLog("Request user presets");
    clearCCValues();
    user_presets.clear();
    user_preset_state = InitState::Pending;
    //silence(false);
    // todo: save/restore EM MIDI routing to disable surface > midi/cvc to avoid interruption while loading
//    sendControlChange(EM_SettingsChannel, EMCC_Download, EM_DownloadItem::midiTxTweenth);
    sendControlChange(EM_SettingsChannel, EMCC_Download, EM_DownloadItem::userToMidi);
}

void Hc1Module::setPreset(std::shared_ptr<Preset> preset)
{
    current_preset = nullptr;
    notifyPresetChanged();

    current_preset = preset;
    if (!preset) return;

    DebugLog("Setting preset [%s]", preset ? preset->describe_short().c_str(): "(none)");
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

void Hc1Module::sendEditorPresent()
{
    DebugLog("Editor present");
    handshake = InitState::Pending;
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

        case EMCC_VersionHigh:
            firmware_version = value;
            break;

        case EMCC_VersionLow:
            firmware_version = (firmware_version << 7) | value;
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
                        heart_time = .1f; // next staep can start quickly
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
            // if (InitState::Pending == device_hello_state) {
            //     device_hello_state = InitState::Complete;
            // } else
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

void Hc1Module::onChannel0CC(uint8_t cc, uint8_t value)
{
    ch0_cc_value[cc] = value;
    switch (cc) {
        case EMCC_PedalFraction: 
            pedal_fraction =  value; // setMacroCCValue uses pedal_fraction
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

        case EMCC_RountInitial: {
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

void Hc1Module::onChannel0Message(const midi::Message& msg)
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
    //midi_receive_byte_count += msg.bytes.size();

    auto channel = msg.getChannel();
    switch (channel) {
        case EM_MasterChannel:
            onChannel0Message(msg);
            break;

        case EM_KentonChannel:
            break;

        case EM_MatrixChannel:
            break;

        case EM_SettingsChannel:
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
            }
            break;
    }
}

}