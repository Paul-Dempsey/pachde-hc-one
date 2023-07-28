#include <stdint.h>
#include "../plugin.hpp"
#include "../components.hpp"
#include "../small_push.hpp"
#include "../text.hpp"
#include "../em_midi.hpp"
#include "../presets.hpp"
#include "../midi_input_proxy.hpp"

#define VERBOSE_LOG
#ifdef VERBOSE_LOG
#define DebugLog(format, ...) DEBUG(format, ##__VA_ARGS__)
#else
#define DebugLog(format, ...) {}
#endif
using namespace em_midi;

namespace pachde {

std::string FilterDeviceName(std::string text) {
    #ifdef ARCH_WIN
    if (!text.empty()) {
        text.erase(text.find_last_not_of("0123456789"));
    }
    #endif
    return text;
}

bool is_EMDevice(const std::string name) {
    std::string text = name;
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c){ return std::tolower(c); });
    if (0 == text.compare(0, 8, "continuu", 0, 8)) { return true; }
    if (0 == text.compare(0, 6, "osmose", 0, 6)) { return true; }
    if (0 == text.compare(0, 5, "eagan", 0, 5)) { return true; }
    return false;
}

static const NVGcolor blue_light         = nvgHSL(220.f/360.f, 0.85f, 0.5f);
static const NVGcolor green_light        = nvgHSL(120.f/360.f, 0.85f, 0.5f);
static const NVGcolor bright_green_light = nvgHSL(120.f/360.f, 0.85f, 0.9f);
static const NVGcolor orange_light       = nvgHSL(30.f/360.f, 0.85f, 0.5f);
static const NVGcolor yellow_light       = nvgHSL(60.f/360.f, 0.85f, 0.5f);
static const NVGcolor red_light          = nvgHSL(0.f, 0.85f, 0.5f);
static const NVGcolor white_light        = nvgRGB(0xef, 0xef, 0xef);
static const NVGcolor purple_light       = nvgHSL(270.f/360.f, 0.85f, 0.5f);
static const NVGcolor preset_name_color  = nvgRGB(0xe6, 0xa2, 0x1a);

struct Hc1Module : Module, IProcessMidi
{
    enum Params
    {
        M1_PARAM,
        M2_PARAM,
        M3_PARAM,
        M4_PARAM,
        M5_PARAM,
        M6_PARAM,
        R1_PARAM,
        R2_PARAM,
        R3_PARAM,
        R4_PARAM,
        RMIX_PARAM,
        NUM_PARAMS
    };
    enum Inputs
    {
        M1_INPUT,
        M2_INPUT,
        M3_INPUT,
        M4_INPUT,
        M5_INPUT,
        M6_INPUT,
        R1_INPUT,
        R2_INPUT,
        R3_INPUT,
        R4_INPUT,
        RMIX_INPUT,
        NUM_INPUTS
    };
    enum Outputs
    {
        NUM_OUTPUTS
    };
    enum Lights
    {
        HEART_LIGHT,
        NUM_LIGHTS
    };
    float last_input[Inputs::NUM_INPUTS] {0.f};

    Preset preset0;
    uint16_t firmware_version = 0;
    uint8_t dsp[3] {0};
    uint8_t pedal_fraction = 0;

    bool requested_config = false;
    bool have_config = false;
    bool requested_updates = false;

    int inputDeviceId = -1;
    MidiInputProxy midiInput;
    midi::Output midiOutput;
    std::string device_name;
    float heart_phase = 0.f;
    float heart_time = 1.0;
    bool tick_tock = true;

    bool is_eagan_matrix = false;
    bool waiting_for_handshake = false;
    int64_t notesOn = 0;
    uint8_t recirculator = 0;

    int download_message_id = -1; // CC109
    NVGcolor ledColor = green_light;

    const std::string deviceName() { return device_name; }
    bool isEaganMatrix() { return is_eagan_matrix; }

    int data_stream = -1;
    bool busy = false;
    bool in_preset = false;
    
    Hc1Module() : midiInput(this)
    {
        config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);
        configParam(M1_PARAM, 0.f, EM_Max14f, EM_Max14f/2.f, "i");
        configParam(M2_PARAM, 0.f, EM_Max14f, EM_Max14f/2.f, "ii");
        configParam(M3_PARAM, 0.f, EM_Max14f, EM_Max14f/2.f, "iii");
        configParam(M4_PARAM, 0.f, EM_Max14f, EM_Max14f/2.f, "iv");
        configParam(M5_PARAM, 0.f, EM_Max14f, EM_Max14f/2.f, "v");
        configParam(M6_PARAM, 0.f, EM_Max14f, EM_Max14f/2.f, "vi");
        configParam(R1_PARAM, 0.f, 127.f, 64.f, "R1")->snapEnabled = true;
        configParam(R2_PARAM, 0.f, 127.f, 64.f, "R2")->snapEnabled = true;
        configParam(R3_PARAM, 0.f, 127.f, 64.f, "R3")->snapEnabled = true;
        configParam(R4_PARAM, 0.f, 127.f, 64.f, "R4")->snapEnabled = true;
        configParam(RMIX_PARAM, 0.f, 127.f, 64.f, "Recirculator Mix")->snapEnabled = true;
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

    void setLastInputs (float v) {
        for (auto n = 0; n < Inputs::NUM_INPUTS; ++n)
            last_input[n] = v;
    }
    void paramToDefault(int id) {
        auto pq = getParamQuantity(id);
        getParam(id).setValue(pq->getDefaultValue());
    }
    void centerKnobs() {
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

    json_t *dataToJson() override
    {
        auto root = json_object();
        json_object_set_new(root, "midi-in", midiInput.toJson());
        return root;
    }

    void dataFromJson(json_t *root) override
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

    void onReset() override
    {
        setLastInputs(0.f);
        midiInput.reset();
        midiOutput.reset();
        notesOn = 0;
        data_stream = -1;
        download_message_id = -1;
        waiting_for_handshake = false;
        busy = false;
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

    void findEMOut() {
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
    void findEM() {
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

    EM_Recirculator recirculatorType() {
        return static_cast<EM_Recirculator>(recirculator & EM_Recirculator::Mask);
    }
    bool isExtendRecirculator() { return recirculator & EM_Recirculator::Extend; }
    const std::string recirculatorName() {
        return RecirculatorName(recirculatorType());
    }
    const std::string recirculatorParameterName(int r) {
        return RecirculatorParameterName(recirculatorType(), r);
    }

    void transmitRequestUpdates() {
        DebugLog("request updates");
        requested_updates = true;
        sendCC(EM_SettingsChannel, EMCC_Preserve, 1);
    }

    void transmitRequestConfiguration() {
        //sendEditorPresent();
        DebugLog("request configuration");
        requested_config = true;
        sendCC(EM_SettingsChannel, EMCC_Download, configToMidi);
    }

    void sendCC(uint8_t channel, uint8_t cc, uint8_t value) {
        midi::Message msg;
        SetCC(msg, channel, cc, value);
        midiOutput.sendMessage(msg);
    }

    void sendNote(uint8_t channel, uint8_t note, uint8_t velocity) {
        midi::Message msg;
        SetNoteOn(msg, channel, note, velocity);
        auto save_channel = midiOutput.getChannel();
        midiOutput.setChannel(channel);
        midiOutput.sendMessage(msg);
        midiOutput.setChannel(save_channel);
    }

    void sendNoteOff(uint8_t channel, uint8_t note) {
        midi::Message msg;
        SetNoteOff(msg, channel, note, 0);
        auto save_channel = midiOutput.getChannel();
        midiOutput.setChannel(channel);
        midiOutput.sendMessage(msg);
        midiOutput.setChannel(save_channel);
    }

    void sendEditorPresent() {
        DebugLog("Editor present");
        waiting_for_handshake = true;
        sendCC(EM_SettingsChannel, EMCC_EditorPresent, tick_tock ? 85 : 42);
        tick_tock = !tick_tock;
        //download_message_id = -1;
    }
    
    bool isRecirculatorExtend() { return recirculator & EM_Recirculator::Extend; }

    void handle_ch16_cc(uint8_t cc, uint8_t value)
    {
        switch (cc) {
            case MidiCC_BankSelect:  preset0.bank_hi = value; break;
            case 32: preset0.bank_lo = value; break;

            case EMCC_RecirculatorType: recirculator = value; break;

            case EMCC_DataStream: {
                switch (value) {
                    case EM_StreamType::Name:
                        assert(!busy && (data_stream == -1));
                        DebugLog("Begin name");
                        if (data_stream != EM_StreamType::Name) {
                            preset0.clear_name();
                        }
                        preset0.clear_name();
                        data_stream = value;
                        busy = true;
                        break;
                        
                    case EM_StreamType::ConText:
                        assert(!busy && (data_stream == -1));
                        DebugLog("Begin Text");
                        if (data_stream != EM_StreamType::ConText) {
                            preset0.clear_text();
                        }
                        data_stream = value;
                        busy = true;
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
                        busy = false;
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

    void handle_ch16_message(const midi::Message& msg)
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

    void onNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
        if (velocity) {
            notesOn++;
        } else {
            notesOn--;
        }
        heart_phase = 0;
        heart_time = 5.f;
    }

    void onNoteOff(uint8_t channel, uint8_t note, uint8_t velocity) {
        notesOn--;
        heart_phase = 0;
        heart_time = 5.f;
    }

    void setMacroCCValue(int id, uint8_t value) {
        getParam(id).setValue((value << 7) | pedal_fraction);
        pedal_fraction = 0;
    }

    void onSoundOff() {
        if (!in_preset) {
            DebugLog("---- BEGIN PRESET ----");
            in_preset = true;
            dsp[2] = dsp[1] = dsp[0] = 0;
        }
    }

    void onChannel0CC(uint8_t cc, uint8_t value)
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
            case EMCC_R1:   getParam(R1_PARAM).setValue(value); break;
            case EMCC_R2:   getParam(R2_PARAM).setValue(value); break;
            case EMCC_R3:   getParam(R3_PARAM).setValue(value); break;
            case EMCC_R4:   getParam(R4_PARAM).setValue(value); break;
            case EMCC_RMIX: getParam(RMIX_PARAM).setValue(value); break;

            case MidiCC_AllSoundOff: onSoundOff(); break;
        }
    }

    void handle_ch0_message(const midi::Message& msg) {
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
	void processMidi(const midi::Message& msg) override {
        DebugLog("%lld %s", msg.frame, ToFormattedString(msg).c_str());
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
                        // already have it ch0, which is always sent
                        // if (msg.bytes[1] == MidiCC_AllSoundOff) {
                        //     onSoundOff();
                        // }
                        break;
                }
                break;
        }
    }

    const int CV_INTERVAL = 32;
    int check_cv = 0;
    void processCV()
    {
        auto in = getInput(M1_INPUT);
        if (in.isConnected()) {
        }
    }
    void process(const ProcessArgs& args) override
    {
        if (++check_cv > CV_INTERVAL) {
            check_cv = 0;
            processCV();
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
};

struct MidiKnob : RoundSmallBlackKnob
{
    uint8_t cc;
    uint16_t lastValue = 0;
    MidiKnob(uint8_t cc) : cc(cc) { }

    void syncNewValue() {
        engine::ParamQuantity* pq = getParamQuantity();
        if (pq) {
            if (Is14BitPedalCC(cc)) {
                uint16_t value = static_cast<uint16_t>(clamp(pq->getValue(), 0.f, EM_Max14f));
                if (value != lastValue) {
                    lastValue = value;
                    auto module = dynamic_cast<Hc1Module*>(pq->module);
                    if (module) {
                        uint8_t lo = value & 0x7f;
                        if (lo) {
                            module->sendCC(EM_SettingsChannel, EMCC_PedalFraction, lo);
                        }
                        uint8_t hi = value >> 7;
                        module->sendCC(EM_SettingsChannel, cc, hi);
                    }
                }
            } else {
                uint16_t value = static_cast<uint16_t>(clamp(pq->getValue(), 0.f, 127.f));
                if (value != lastValue) {
                    lastValue = value;
                    auto module = dynamic_cast<Hc1Module*>(pq->module);
                    if (module) {
                        module->sendCC(EM_SettingsChannel, cc, value);
                    }
                }
            }
        }
    }
    void onDragMove(const DragMoveEvent& e) override {
        RoundSmallBlackKnob::onDragMove(e);
        syncNewValue();
    }
    void onHoverScroll(const HoverScrollEvent& e) override {
        RoundSmallBlackKnob::onHoverScroll(e);
        syncNewValue();
    }
};

template <class TParamWidget = MidiKnob>
TParamWidget *createMidiParam(uint8_t cc, math::Vec pos, engine::Module *module, int paramId)
{
    TParamWidget *o = new TParamWidget(cc);
    o->box.pos = pos;
    o->app::ParamWidget::module = module;
    o->app::ParamWidget::paramId = paramId;
    o->initParamQuantity();
    return o;
}
template <class TParamWidget = MidiKnob>
TParamWidget *createMidiParamCentered(uint8_t cc, math::Vec pos, engine::Module *module, int paramId)
{
    TParamWidget *o = createMidiParam<TParamWidget>(cc, pos, module, paramId);
    o->box.pos = o->box.pos.minus(o->box.size.div(2));
    return o;
}

constexpr const float KNOB_LEFT = 36.f;
constexpr const float KNOB_SPREAD = 54.f;
constexpr const float KNOB_ROW_1 = 45.f;
constexpr const float KNOB_ROW_2 = 85.f;

struct Hc1ModuleWidget : ModuleWidget
{
    Hc1Module *my_module = nullptr;
    GrayModuleLightWidget * status_light;

    Hc1ModuleWidget(Hc1Module *module)
    {
        my_module = module;
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/HC-1.svg")));

        status_light = createLightCentered<MediumLight<BlueLight>>(Vec(12.f, 12.f), my_module, Hc1Module::HEART_LIGHT);
		addChild(status_light);

        addChild(createMidiParamCentered<MidiKnob>(EMCC_i,    Vec(KNOB_LEFT, KNOB_ROW_1), module, Hc1Module::M1_PARAM));
        addChild(createMidiParamCentered<MidiKnob>(EMCC_ii,   Vec(KNOB_LEFT +       KNOB_SPREAD, KNOB_ROW_1), module, Hc1Module::M2_PARAM));
        addChild(createMidiParamCentered<MidiKnob>(EMCC_iii,  Vec(KNOB_LEFT + 2.f * KNOB_SPREAD, KNOB_ROW_1), module, Hc1Module::M3_PARAM));
        addChild(createMidiParamCentered<MidiKnob>(EMCC_iv,   Vec(KNOB_LEFT + 3.f * KNOB_SPREAD, KNOB_ROW_1), module, Hc1Module::M4_PARAM));
        addChild(createMidiParamCentered<MidiKnob>(EMCC_v,    Vec(KNOB_LEFT + 4.f * KNOB_SPREAD, KNOB_ROW_1), module, Hc1Module::M5_PARAM));
        addChild(createMidiParamCentered<MidiKnob>(EMCC_vi,   Vec(KNOB_LEFT + 5.f * KNOB_SPREAD, KNOB_ROW_1), module, Hc1Module::M6_PARAM));
        addChild(createMidiParamCentered<MidiKnob>(EMCC_R1,   Vec(KNOB_LEFT, KNOB_ROW_2), module, Hc1Module::R1_PARAM));
        addChild(createMidiParamCentered<MidiKnob>(EMCC_R2,   Vec(KNOB_LEFT +       KNOB_SPREAD, KNOB_ROW_2), module, Hc1Module::R2_PARAM));
        addChild(createMidiParamCentered<MidiKnob>(EMCC_R3,   Vec(KNOB_LEFT + 2.f * KNOB_SPREAD, KNOB_ROW_2), module, Hc1Module::R3_PARAM));
        addChild(createMidiParamCentered<MidiKnob>(EMCC_R4,   Vec(KNOB_LEFT + 3.f * KNOB_SPREAD, KNOB_ROW_2), module, Hc1Module::R4_PARAM));
        addChild(createMidiParamCentered<MidiKnob>(EMCC_RMIX, Vec(KNOB_LEFT + 4.f * KNOB_SPREAD, KNOB_ROW_2), module, Hc1Module::RMIX_PARAM));

        auto pm = createWidget<PickMidi>(Vec(7.5f, box.size.y - 20.f));
        pm->describe("Choose Midi input");
        if (my_module) {
            pm->setMidiPort(&my_module->midiInput);
        }
        addChild(pm);
        pm = createWidget<PickMidi>(Vec(20.f, box.size.y - 20.f));
        pm->describe("Choose Midi output");
        if (my_module) {
            pm->setMidiPort(&my_module->midiOutput);
        }
        addChild(pm);
        auto pb = createWidget<SmallPush>(Vec(0, 0));
        pb->box.size.x = 14.f;
        pb->box.size.y = 14.f;
        pb->center(Vec(45.f, box.size.y -10.f));
        if (module) {
            #ifdef ARCH_MAC
                pb->describe("Send test Note\nCmd+click = Note off");
            #else
                pb->describe("Send test Note\nCtrl+click = Note off");
            #endif
            pb->onClick([module](bool ctrl, bool shift) {
                if (ctrl) {
                    module->sendNoteOff(0, 60);
                } else {
                    module->sendNote(0, 60, 64);
                }
            });
        }
        addChild(pb);
    }

    void step() override {
        ModuleWidget::step();
        if (my_module)
        {
            auto co = my_module->isEaganMatrix() ? my_module->ledColor : red_light;
            if (!IS_SAME_COLOR(co, status_light->baseColors[0])) {
                status_light->baseColors[0] = co;
                my_module->getLight(Hc1Module::HEART_LIGHT).setBrightness(1.f);
                DirtyEvent e;
                status_light->onDirty(e);
            }
        }

    }

    void drawLayer(const DrawArgs& args, int layer) override
    {
        ModuleWidget::drawLayer(args, layer);
        if (1 == layer) {
            auto vg = args.vg;
            // auto color = my_module ? nvgHSL(360.f/210.f, .5f, my_module->note/128.f) : COLOR_BRAND;
            // Circle(args.vg, 12.f, 17.f, 3.f, color); // todo: halo, device state
            auto font = GetPluginFontSemiBold();
            if (FontOk(font)) {
                SetTextStyle(vg, font, preset_name_color, 16.f);
                if (my_module) {
                    CenterText(vg, box.size.x/2.f, 20.f, my_module->preset0.name(), nullptr);
                    //nvgText(vg, 130.f, 32.f, my_module->preset0.text(), nullptr);
                } else {
                    CenterText(vg, box.size.x/2.f, 20.f, "(preset name)", nullptr);
                    //nvgText(vg, 130.f, 32.f, "(preset text)", nullptr);
                }
            }
            if (my_module) {
                auto h = 10.f;
                auto w = 2.5f;
                auto x = box.size.x - 7.5f - 3.f*w - 2;
                auto y = box.size.y - 20.f - h;
                for (auto n = 0; n < 3; n++) {
                    auto pct = my_module->dsp[n];
                    auto co = pct < 85 ? green_light : red_light;
                    auto bh = h * (pct / 100.f);
                    FillRect(vg, x, y + h - bh, w, bh, co);
                    x += w + 1;
                }
            } else {

            }
        }
    }

    void draw(const DrawArgs& args) override
    {
        ModuleWidget::draw(args);
        auto vg = args.vg;

        Circle(vg, 12.f, 20.f, 3.f, my_module && my_module->waiting_for_handshake ? orange_light : blue_light );
         if (my_module) {
            Circle(vg, 12.f + (my_module->tick_tock ? 1.5f : -1.5f), 26.f, 1.25f, COLOR_MAGENTA);
         }

        auto font = GetPluginFontRegular();
        if (FontOk(font)) {
            std::string device_name;
            if (my_module) {
                device_name = FilterDeviceName(my_module->deviceName());
            } else {
                device_name = "Continuum #######";
            }
            if (device_name.empty()) {
                device_name = "(suitable device not found)";
            }
            SetTextStyle(vg, font, RampGray(G_90), 12.f);
            nvgText(vg, box.size.x/2.f + 25.f, box.size.y - 7.5f, device_name.c_str(), nullptr);

            if (my_module) {
                RightAlignText(vg, box.size.x - 7.5, box.size.y - 7.5f, format_string("v %.2f", my_module->firmware_version/100.f).c_str(), nullptr);
                if (my_module->download_message_id >= 0)
                {
                    auto message = DownloadMessage(my_module->download_message_id);
                    if (nullptr != message) {
                        nvgText(vg, 22.f, 34.f, message, nullptr);
                    }
                }
            }
        }
        font = GetPluginFontSemiBold();
        if (FontOk(font)) {
            SetTextStyle(vg, font, RampGray(G_90), 12.f);
            float y = KNOB_ROW_1 + 22.f;
            if (my_module && !my_module->busy) {
                CenterText(vg, KNOB_LEFT, y, my_module->preset0.macro[0].c_str(), nullptr);
                CenterText(vg, KNOB_LEFT +       KNOB_SPREAD, y, my_module->preset0.macro[1].c_str(), nullptr);
                CenterText(vg, KNOB_LEFT + 2.f * KNOB_SPREAD, y, my_module->preset0.macro[2].c_str(), nullptr);
                CenterText(vg, KNOB_LEFT + 3.f * KNOB_SPREAD, y, my_module->preset0.macro[3].c_str(), nullptr);
                CenterText(vg, KNOB_LEFT + 4.f * KNOB_SPREAD, y, my_module->preset0.macro[4].c_str(), nullptr);
                CenterText(vg, KNOB_LEFT + 5.f * KNOB_SPREAD, y, my_module->preset0.macro[5].c_str(), nullptr);
            } else {
                CenterText(vg, KNOB_LEFT, y, "i", nullptr);
                CenterText(vg, KNOB_LEFT +       KNOB_SPREAD, y, "ii", nullptr);
                CenterText(vg, KNOB_LEFT + 2.f * KNOB_SPREAD, y, "iii", nullptr);
                CenterText(vg, KNOB_LEFT + 3.f * KNOB_SPREAD, y, "iv", nullptr);
                CenterText(vg, KNOB_LEFT + 4.f * KNOB_SPREAD, y, "v", nullptr);
                CenterText(vg, KNOB_LEFT + 5.f * KNOB_SPREAD, y, "vi", nullptr);
            }
            auto rt = my_module ? my_module->recirculatorType() : EM_Recirculator::Reverb;
            y = KNOB_ROW_2 + 22.f;
            CenterText(vg, KNOB_LEFT, y, RecirculatorParameterName(rt, 1).c_str(), nullptr);
            CenterText(vg, KNOB_LEFT + KNOB_SPREAD, y, RecirculatorParameterName(rt, 2).c_str(), nullptr);
            CenterText(vg, KNOB_LEFT + KNOB_SPREAD * 2.f, y, RecirculatorParameterName(rt, 3).c_str(), nullptr);
            CenterText(vg, KNOB_LEFT + KNOB_SPREAD * 3.f, y, RecirculatorParameterName(rt, 4).c_str(), nullptr);
            CenterText(vg, KNOB_LEFT + KNOB_SPREAD * 4.f, y, "Mix", nullptr);

            nvgTextAlign(vg, NVG_ALIGN_LEFT);
            nvgText(vg, KNOB_LEFT + KNOB_SPREAD * 4.f + 23.f, y - 15.f, RecirculatorName(rt).c_str(), nullptr);
        }
        DrawLogo(vg, box.size.x /2.f - 12.f, RACK_GRID_HEIGHT - ONE_HP, RampGray(G_90));
    }

    // Add options to your module's menu here
    // void appendContextMenu(Menu *menu) override
    //{
    //}
};

}

Model *modelHc1 = createModel<pachde::Hc1Module, pachde::Hc1ModuleWidget>("pachde-hc-1");
