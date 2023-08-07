#include "HC-1.hpp"
#include "cc_param.hpp"
#include "../misc.hpp"
namespace pachde {

bool preset_order(const std::shared_ptr<MinPreset>& p1, const std::shared_ptr<MinPreset>& p2)
{
    uint32_t c1 = (p1->bank_hi << 16) | (p1->bank_lo << 8) | p1->number;
    uint32_t c2 = (p2->bank_hi << 16) | (p2->bank_lo << 8) | p2->number;
    return c1 < c2;
}

Hc1Module::Hc1Module() : midiInput(this)
{
    presets.reserve(700);
    user_presets.reserve(128);
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);
    configCCParam(em_midi::EMCC_i,   true, this, M1_PARAM, 0.f, EM_Max14f, EM_Max14f/2.f, "i");
    configCCParam(em_midi::EMCC_ii,  true, this, M2_PARAM, 0.f, EM_Max14f, EM_Max14f/2.f, "ii");
    configCCParam(em_midi::EMCC_iii, true, this, M3_PARAM, 0.f, EM_Max14f, EM_Max14f/2.f, "iii");
    configCCParam(em_midi::EMCC_iv,  true, this, M4_PARAM, 0.f, EM_Max14f, EM_Max14f/2.f, "iv");
    configCCParam(em_midi::EMCC_v,   true, this, M5_PARAM, 0.f, EM_Max14f, EM_Max14f/2.f, "v");
    configCCParam(em_midi::EMCC_vi,  true, this, M6_PARAM, 0.f, EM_Max14f, EM_Max14f/2.f, "vi");
    configCCParam(em_midi::EMCC_R1,   false, this, R1_PARAM, 0.f, 127.f, 64.f, "R1")->snapEnabled = true;
    configCCParam(em_midi::EMCC_R2,   false, this, R2_PARAM, 0.f, 127.f, 64.f, "R2")->snapEnabled = true;
    configCCParam(em_midi::EMCC_R3,   false, this, R3_PARAM, 0.f, 127.f, 64.f, "R3")->snapEnabled = true;
    configCCParam(em_midi::EMCC_R4,   false, this, R4_PARAM, 0.f, 127.f, 64.f, "R4")->snapEnabled = true;
    configCCParam(em_midi::EMCC_RMIX, false, this, RMIX_PARAM, 0.f, 127.f, 64.f, "Recirculator Mix")->snapEnabled = true;
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
    configLight(Lights::HEART_LIGHT, "Device status");
    getLight(HEART_LIGHT).setBrightness(1.0f);
    findEM();
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

json_t * Hc1Module::dataToJson()
{
    auto root = json_object();
//    json_object_set_new(root, "midi-in", midiInput.toJson());
    return root;
}

void Hc1Module::dataFromJson(json_t *root)
{
    // auto j = json_object_get(root, "midi-in");
    // if (j) {
    //     midiInput.fromJson(j);
    //     midiInput.setChannel(-1);
    // }
    // device_name = midiInput.getDeviceName(midiInput.deviceId);
    // is_eagan_matrix = is_EMDevice(device_name);
    // findEMOut();
}

void Hc1Module::onReset()
{
    midiInput.reset();
    midiOutput.reset();

    broken = false;
    broken_idle = 0.f;
    heart_phase = 0.f;
    heart_time = 2.f;

    preset0.clear();
    presets.clear();
    user_presets.clear();
    config_state = InitState::Uninitialized;
    preset_state = InitState::Uninitialized;
    device_state = InitState::Uninitialized;
    handshake = InitState::Uninitialized;

    requested_updates = false;
    in_preset = in_sys_names = in_user_names = false;

    pedal_fraction = 0;
    notesOn = 0;
    data_stream = -1;
    download_message_id = -1;
    recirculator = 0;
    midi_count = 0;
    findEM();
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
    for (auto id : midiInput.getDeviceIds()) {
        auto dev_name = midiInput.getDeviceName(id);
        if (is_EMDevice(dev_name)) {
            midiInput.setDeviceId(id);
            midiInput.setChannel(-1);
            inputDeviceId = id;
            device_name = dev_name;
            is_eagan_matrix = true;
            //heart_time = 0.25f;
            break;
        }
    }
    findEMOut();
}

void Hc1Module::setPreset(std::shared_ptr<MinPreset> preset)
{
    DebugLog("Setting preset [%s]", preset ? preset->describe(false).c_str() : "(none)");
    if (preset) {
        sendControlChange(15, MidiCC_BankSelect, preset->bank_hi);
        sendControlChange(15, EMCC_Category, preset->bank_lo);
        sendProgramChange(15, preset->number);
        //transmitRequestConfiguration();
    }
}

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
    midiOutput.sendMessage(msg);
}

void Hc1Module::transmitRequestUpdates()
{
    DebugLog("Request updates");
    requested_updates = true;
    sendControlChange(EM_SettingsChannel, EMCC_Preserve, 1);
}

void Hc1Module::transmitRequestConfiguration()
{
    //sendEditorPresent();
    DebugLog("Request configuration");
    config_state = InitState::Pending;
    sendControlChange(EM_SettingsChannel, EMCC_Download, EM_DownloadItem::configToMidi);
}

void Hc1Module::transmitRequestPresets()
{
    DebugLog("Request presets");
    presets.clear();
    preset_state = InitState::Pending;
    //silence(false);
    // todo: save/restore EM MIDI routing to disable surface > midi/cvc to avoid interruption while loading
    sendControlChange(EM_SettingsChannel, EMCC_Download, EM_DownloadItem::userToMidi);
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
    //heart_time = 0.25f;
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

void Hc1Module::handle_ch16_cc(uint8_t cc, uint8_t value)
{
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
                    } else {
                        //DebugLog("Begin name");
                        data_stream = value;
                        if (in_sys_names || in_user_names || configPending()) {
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
                    }
                    in_user_names = false;
                    // chain to sys names
                    if (!broken) {
                        sendControlChange(EM_SettingsChannel, EMCC_Download, EM_DownloadItem::sysToMidi);
                    }
                    break;

                case EM_DownloadItem::beginSysNames:
                    DebugLog("[---- beginSysNames ----]");
                    in_sys_names = true;
                    break;

                case EM_DownloadItem::endSysNames:
                    DebugLog("[---- endSysNames ----]");
                    if (!broken) {
                        std::sort(presets.begin(), presets.end(), preset_order);
                    }
                    in_sys_names = false;
                    preset_state = broken ? InitState::Broken : InitState::Complete;
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
                DebugLog("Spurious Editor Reply");
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
                //DebugLog("%s (%d.%d.%d)", preset0.name(), preset0.bank_hi, preset0.bank_lo, preset0.number);
                if (!preset0.name_empty() && configPending()) {
                    config_state = broken ? InitState::Broken : InitState::Complete;
                }

                if (!broken && is_gathering_presets()) {
                    if (!preset0.name_empty()) {
                        std::string name = preset0.name();
                        if (msg.getNote() != 126 && "-" != name && "Empty" != name) {
                            if (in_user_names) {
                                user_presets.push_back(std::make_shared<MinPreset>(preset0));
                            } else {
                                presets.push_back(std::make_shared<MinPreset>(preset0));
                            }
                        }
                    }
                }
            }
            // if (!is_gathering_presets()) {
            //      sendEditorPresent();
            // }
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
    //DebugLog("---- BEGIN PRESET ----");
    in_preset = true;
    dsp[2] = dsp[1] = dsp[0] = 0;
}

void Hc1Module::onSoundOff()
{
    if (!in_preset) {
        beginPreset();
        DebugLog("Config pending");
        config_state = InitState::Pending;
    }
}

void Hc1Module::onChannel0CC(uint8_t cc, uint8_t value)
{
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

void Hc1Module::processMidi(const midi::Message& msg)
{
    //DebugLog("%lld %s", static_cast<long long int>(msg.frame), ToFormattedString(msg).c_str());
    broken_idle = 0.f;
    ++midi_count;
    
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
            //if (!broken) {
                handle_ch16_message(msg);
            //}
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

void Hc1Module::processCV(int inputId) {
    // TODO: relative mode for bipolar?
    auto in = getInput(inputId);
    if (in.isConnected()) {
        auto v = in.getVoltage();
        ParamQuantity* pq = getParamQuantity(inputId);
        v = v * .1f * pq->getMaxValue();
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
    if (++check_cv > CV_INTERVAL) {
        check_cv = 0;
        processAllCV();
    }

    if (settings::headless) {
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
        if (broken_idle > 2.0f) {
            onReset();
        }
    }

    heart_phase += args.sampleTime;
    if (heart_phase >= heart_time) {
        heart_phase -= heart_time;
        heart_time = 2.5f;

        if (broken) return;

        // TODO: fix this mess of conditionals and make a state machine
        if (inputDeviceId != midiInput.deviceId) {
            device_name = midiInput.getDeviceName(midiInput.deviceId);
            is_eagan_matrix = is_EMDevice(device_name);
            inputDeviceId = midiInput.deviceId;
        } else if (!is_eagan_matrix) {
            findEM();
        } else if (is_eagan_matrix) {
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
                if (InitState::Uninitialized == preset_state) {
                    transmitRequestPresets();
                } else if (InitState::Uninitialized == config_state) {
                    transmitRequestConfiguration();
                } else if (InitState::Complete == config_state) {
                    if (!requested_updates) {
                        transmitRequestUpdates();
                    } else {
                        sendEditorPresent();
                    }
                }
            }
        } // matrix
    }// heartbeat
} // process

} //pachde

Model *modelHc1 = createModel<pachde::Hc1Module, pachde::Hc1ModuleWidget>("pachde-hc-1");
