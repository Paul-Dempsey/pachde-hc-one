#include "HC-1.hpp"
#include "cc_param.hpp"
namespace pachde {

bool is_EMDevice(const std::string name) {
    std::string text = name;
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c){ return std::tolower(c); });
    if (0 == text.compare(0, 8, "continuu", 0, 8)) { return true; }
    if (0 == text.compare(0, 6, "osmose", 0, 6)) { return true; }
    if (0 == text.compare(0, 5, "eagan", 0, 5)) { return true; }
    return false;
}

Hc1Module::Hc1Module() : midiInput(this)
{
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
    json_object_set_new(root, "midi-in", midiInput.toJson());
    return root;
}

void Hc1Module::dataFromJson(json_t *root)
{
    auto j = json_object_get(root, "midi-in");
    if (j) {
        midiInput.fromJson(j);
        midiInput.setChannel(-1);
    }
    device_name = midiInput.getDeviceName(midiInput.deviceId);
    is_eagan_matrix = is_EMDevice(device_name);
    findEMOut();
}

void Hc1Module::onReset()
{
    midiInput.reset();
    midiOutput.reset();
    notesOn = 0;
    data_stream = -1;
    download_message_id = -1;
    waiting_for_handshake = false;
    download_message_id = -1; 
    recirculator = 0;
    requested_config = false;
    have_config = false;
    requested_updates = false;
    preset0.clear();
    findEM();
    midiInput.setChannel(-1);
    midiOutput.setChannel(-1);
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
            midiOutput.setChannel(15);
        }
    }
}

void Hc1Module::findEM() {
    for (auto id : midiInput.getDeviceIds()) {
        if (is_EMDevice(midiInput.getDeviceName(id))) {
            midiInput.setDeviceId(id);
            midiInput.setChannel(-1);
            inputDeviceId = id;
            device_name = midiInput.getDeviceName(id);
            is_eagan_matrix = true;
            break;
        }
    }
    findEMOut();
}

void Hc1Module::sendCC(uint8_t channel, uint8_t cc, uint8_t value)
{
    midi::Message msg;
    SetCC(msg, channel, cc, value);
    midiOutput.sendMessage(msg);
}

void Hc1Module::transmitRequestUpdates()
{
    DebugLog("request updates");
    requested_updates = true;
    sendCC(EM_SettingsChannel, EMCC_Preserve, 1);
}

void Hc1Module::transmitRequestConfiguration()
{
    //sendEditorPresent();
    DebugLog("request configuration");
    requested_config = true;
    sendCC(EM_SettingsChannel, EMCC_Download, configToMidi);
}

void Hc1Module::sendEditorPresent()
{
    DebugLog("Editor present");
    waiting_for_handshake = true;
    sendCC(EM_SettingsChannel, EMCC_EditorPresent, tick_tock ? 85 : 42);
    tick_tock = !tick_tock;
    //download_message_id = -1;
}
    
void Hc1Module::sendNote(uint8_t channel, uint8_t note, uint8_t velocity)
{
    midi::Message msg;
    SetNoteOn(msg, channel, note, velocity);
    auto save_channel = midiOutput.getChannel();
    midiOutput.setChannel(channel);
    midiOutput.sendMessage(msg);
    midiOutput.setChannel(save_channel);
}

void Hc1Module::sendNoteOff(uint8_t channel, uint8_t note)
{
    midi::Message msg;
    SetNoteOff(msg, channel, note, 0);
    auto save_channel = midiOutput.getChannel();
    midiOutput.setChannel(channel);
    midiOutput.sendMessage(msg);
    midiOutput.setChannel(save_channel);
}

void Hc1Module::handle_ch16_cc(uint8_t cc, uint8_t value)
{
    switch (cc) {
        case MidiCC_BankSelect:  preset0.bank_hi = value; break;
        case 32: preset0.bank_lo = value; break;

        case EMCC_RecirculatorType: recirculator = value; break;

        case EMCC_DataStream: {
            switch (value) {
                case EM_StreamType::Name:
                    assert(data_stream == -1);
                    DebugLog("Begin name");
                    if (data_stream != EM_StreamType::Name) {
                        preset0.clear_name();
                    }
                    preset0.clear_name();
                    data_stream = value;
                    break;
                    
                case EM_StreamType::ConText:
                    assert(data_stream == -1);
                    DebugLog("Begin Text");
                    if (data_stream != EM_StreamType::ConText) {
                        preset0.clear_text();
                    }
                    data_stream = value;
                    break;

                case EM_StreamType::DataEnd:
                    switch (data_stream) {
                        case EM_StreamType::Name:
                            DebugLog("End name");
                            break;
                        case EM_StreamType::ConText:
                            DebugLog("End Text");
                            preset0.parse_text();
                            break;
                    }
                    data_stream = -1;
                    break;

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
                case EM_DownloadItem::archiveOk:
                    DebugLog("archiveOk");
                    break;
                case EM_DownloadItem::archiveFail:
                    DebugLog("archiveFail");
                    break;
                case EM_DownloadItem::beginUserNames:
                    DebugLog("beginUserNames");
                    break;
                case EM_DownloadItem::endUserNames:
                    DebugLog("endUserNames");
                    break;
                case EM_DownloadItem::beginSysNames:
                    DebugLog("beginSysNames");
                    break;
                case EM_DownloadItem::endSysNames:
                    DebugLog("endSysNames");
                    break;
            }
            break;

        case EMCC_Info: {
            switch (value) {
                case InfoItem::archiveEof:
                    DebugLog("archiveEof");
                    if (!preset0.name_empty()) {
                            have_config = true;
                    }
                    break;
            }
        } break;

        case EMCC_Status: {
            auto led = value & StatusItem::sLedBits;
            if (led) {
                getLight(HEART_LIGHT).setBrightness(0.2f);
                switch (led) {
                    case StatusItem::ledBlue:
                        ledColor = blue_light;
                        break;
                    case StatusItem::ledRed:
                        ledColor = red_light;
                        break;
                    case StatusItem::ledBrightGreen:
                        ledColor = bright_green_light;
                        break;
                    case StatusItem::ledGreen:
                        ledColor = green_light;
                        break;
                    case StatusItem::ledWhite:
                        ledColor = white_light;
                        break;
                    case StatusItem::ledYellow:
                        ledColor = yellow_light;
                        break;
                    case StatusItem::ledPurple:
                        ledColor = purple_light;
                        break;
                }
                
            }
        } break;

        case EMCC_DSP: {
            auto d = (value >> 5) -1;
            auto pct = (value & 0x1F) * 4;
            dsp[d] = pct;
            DebugLog("DSP %d = %d%%", d, pct);
        } break;

        case EMCC_EditorReply:
            DebugLog("Editor Reply");
            waiting_for_handshake = false;
            break;
    }
}

void Hc1Module::handle_ch16_message(const midi::Message& msg)
{
    auto status = GetRawStatus(msg);
    assert(status & 0x80);
    switch (status) {
        case MidiStatus_NoteOff:
        case MidiStatus_NoteOn:
            DebugLog("Note unexpected on channel 16");
            break;

        case MidiStatus_PolyKeyPressure:
            break;

        case MidiStatus_CC:
            handle_ch16_cc(GetCC(msg), msg.getValue());
            break;

        case MidiStatus_ProgramChange:
            preset0.number = msg.getValue();
            if (!preset0.name_empty()) {
                have_config = true;
            }
            in_preset = false;
            DebugLog("---- END PRESET ----");
            sendEditorPresent();
            break;

        case MidiStatus_ChannelPressure:
            switch (data_stream) {
                case EM_StreamType::Name:
                    preset0.build_name(msg.bytes[1]);
                    break;
                case EM_StreamType::ConText:
                    preset0.build_text(msg.bytes[1]);
                    break;
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

void Hc1Module::onSoundOff() {
    if (!in_preset) {
        DebugLog("---- BEGIN PRESET ----");
        in_preset = true;
        dsp[2] = dsp[1] = dsp[0] = 0;
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
    DebugLog("%lld %s", static_cast<long long int>(msg.frame), ToFormattedString(msg).c_str());
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
                case MidiStatus_CC:
                    // already have it in ch0, which is always sent
                    // if (msg.bytes[1] == MidiCC_AllSoundOff) {
                    //     onSoundOff();
                    // }
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

    float midi_time = midi_timer.process(args.sampleTime);
    if (midi_time > MIDI_RATE) {
        midi_timer.reset();
        if (settings::headless) {
            for (int n = M1_PARAM; n < NUM_PARAMS; ++n) {
                auto pq = dynamic_cast<CCParamQuantity*>(getParamQuantity(n));
                if (pq) {
                    pq->syncValue();
                }
            }
        }
    }

    heart_phase += args.sampleTime;
    if (heart_phase >= heart_time) {
        heart_phase -= heart_time;
        heart_time = 2.5f;

        if (inputDeviceId != midiInput.deviceId) {
            device_name = midiInput.getDeviceName(midiInput.deviceId);
            is_eagan_matrix = is_EMDevice(device_name);
            inputDeviceId = midiInput.deviceId;
        } else if (!is_eagan_matrix) {
            findEM();
        } else if (is_eagan_matrix 
            && !waiting_for_handshake
            && (notesOn <= 0)
            && !in_preset
            ) {
            if (!requested_config) {
                transmitRequestConfiguration();
            } else if (have_config) {
                if (!requested_updates) {
                    transmitRequestUpdates();
                } else {
                    sendEditorPresent();
                }
            }
        }
    }
}

}

Model *modelHc1 = createModel<pachde::Hc1Module, pachde::Hc1ModuleWidget>("pachde-hc-1");
