#pragma once
#ifndef HC_ONE_HPP_INCLUDED
#define HC_ONE_HPP_INCLUDED
#include <stdint.h>
#include "../colors.hpp"
#include "../em_midi.hpp"
#include "../em_types.hpp"
#include "../hc_events.hpp"
#include "../plugin.hpp"
#include "../preset_meta.hpp"
#include "../presets.hpp"
#include "../text.hpp"
#include "../widgets/em_picker.hpp"
#include "../widgets/favorite_widget.hpp"
#include "../widgets/label_widget.hpp"
#include "../widgets/preset_widget.hpp"
#include "../widgets/square_button.hpp"
#include "../widgets/tab_bar.hpp"

using namespace em_midi;
namespace pachde {

//#define VERBOSE_LOG
#include "../debug_log.hpp"
const NVGcolor& StatusColor(StatusItem led);

struct Hc1Module : IPresetHolder, ISendMidi, ISetDevice, IMidiDeviceChange, midi::Input, Module
{
    enum Params
    {
        M1_PARAM, M2_PARAM, M3_PARAM, M4_PARAM, M5_PARAM, M6_PARAM,
        R1_PARAM, R2_PARAM, R3_PARAM, R4_PARAM, RMIX_PARAM,
        VOLUME_PARAM, MUTE_PARAM,
        M1_REL_PARAM, M2_REL_PARAM, M3_REL_PARAM, M4_REL_PARAM, M5_REL_PARAM, M6_REL_PARAM,
        R1_REL_PARAM, R2_REL_PARAM, R3_REL_PARAM, R4_REL_PARAM, RMIX_REL_PARAM,
        VOLUME_REL_PARAM,
        RECIRC_EXTEND_PARAM,
        NUM_PARAMS,
        FIRST_REL_PARAM = M1_REL_PARAM,
    };
    enum Inputs
    {
        M1_INPUT, M2_INPUT, M3_INPUT, M4_INPUT, M5_INPUT, M6_INPUT,
        R1_INPUT, R2_INPUT, R3_INPUT, R4_INPUT, RMIX_INPUT,
        VOLUME_INPUT, MUTE_INPUT,
        NUM_INPUTS
    };
    enum Outputs
    {
        NUM_OUTPUTS
    };
    enum Lights
    {
        M1_REL_LIGHT, M2_REL_LIGHT, M3_REL_LIGHT, M4_REL_LIGHT, M5_REL_LIGHT, M6_REL_LIGHT,
        R1_REL_LIGHT, R2_REL_LIGHT, R3_REL_LIGHT, R4_REL_LIGHT, RMIX_REL_LIGHT,
        VOLUME_REL_LIGHT,
        HEART_LIGHT,
        MUTE_LIGHT,
        RECIRC_EXTEND_LIGHT,
        ROUND_Y_LIGHT, ROUND_INITIAL_LIGHT, ROUND_LIGHT, ROUND_RELEASE_LIGHT,
        //TRANSPOSE_UP_LIGHT, TRANSPOSE_NONE_LIGHT, TRANSPOSE_DOWN_LIGHT,
//        FILTER_LIGHT,
        NUM_LIGHTS
    };

    LivePreset preset0;
    std::vector<std::shared_ptr<Preset>> user_presets;
    std::vector<std::shared_ptr<Preset>> system_presets;
    std::vector<std::shared_ptr<Preset>> favorite_presets;

    std::vector<IHandleHcEvents*> hc_event_subscriptions;

    // ui persistence
    PresetTab tab = PresetTab::User;
    int page[3] = { 0, 0, 0 };

    struct RestoreData {
        PresetTab tab;
        int page[3] = { 0, 0, 0 };
    };
    RestoreData * restore_ui_data = nullptr;

    bool cache_system_presets = true;
    bool cache_user_presets = false;
    std::shared_ptr<Preset> current_preset = nullptr;
    std::shared_ptr<Preset> saved_preset = nullptr;
    bool restore_saved_preset = true;

#if defined PRESET_FILTERING
    PresetFilter preset_filter;
#endif
    PresetOrder preset_order = PresetOrder::Alpha;
    void setPresetOrder(PresetOrder order);

    std::string favoritesFile;
    std::string moduleFavoritesPath();
    void clearFavorites();
    void saveFavorites();
    void readFavorites();
    bool readFavoritesFile(const std::string& path, bool fresh);
    void writeFavoritesFile(const std::string& path);
    void openFavoritesFile(const std::string& path);
    void importHEGroupFile(const std::string& path);
    json_t* favoritesToJson();
    void favoritesFromPresets();

    bool bulk_favoriting = false;
    class BulkFavoritingMode {
        Hc1Module* hc1;
    public:
        explicit BulkFavoritingMode(Hc1Module* hc1) : hc1(hc1) {
            hc1->bulk_favoriting = true;
        }
        ~ BulkFavoritingMode() {
            hc1->bulk_favoriting = false;
        }
    };

    std::string userPresetsPath();
    std::string systemPresetsPath();
    void saveUserPresets();
    void saveSystemPresets();
    void savePresets();
    void loadPresets();
    void loadUserPresets();
    void loadSystemPresets();
    void userPresetsToJson(json_t * root);
    void systemPresetsToJson(json_t * root);

    void setTabPage(PresetTab tab, int tab_page) {
        page[static_cast<size_t>(tab)] = tab_page;
    }
    int getTabPage(PresetTab tab) {
        return page[static_cast<size_t>(tab)];
    }

    const std::vector<std::shared_ptr<Preset>>& getPresets(PresetTab tab) {
        switch (tab) {
            case PresetTab::User: return user_presets;
            case PresetTab::System: return system_presets;
            case PresetTab::Favorite: return favorite_presets;
            default: assert(false);
        }
    }
    
    bool is_eagan_matrix = false;
    InitState device_output_state   = InitState::Uninitialized;
    InitState device_input_state    = InitState::Uninitialized;
    InitState device_hello_state    = InitState::Uninitialized;
    InitState system_preset_state   = InitState::Uninitialized;
    InitState user_preset_state     = InitState::Uninitialized;
    InitState apply_favorite_state  = InitState::Uninitialized;
    InitState config_state          = InitState::Uninitialized;
    InitState saved_preset_state    = InitState::Uninitialized;
    InitState request_updates_state = InitState::Uninitialized;
    InitState handshake             = InitState::Uninitialized;

    bool hasSystemPresets() { return InitState::Complete == system_preset_state && !system_presets.empty(); }
    bool hasUserPresets() { return InitState::Complete == user_preset_state && !user_presets.empty(); }
    bool hasConfig() { return InitState::Complete == config_state; }
    bool deviceHelloPending() { return InitState::Pending ==  device_hello_state; }
    bool configPending() { return InitState::Pending == config_state; }
    bool savedPresetPending() { return InitState::Pending == saved_preset_state; }
    bool handshakePending() { return InitState::Pending == handshake; }

    void tryCachedPresets();

    bool checkDeviceChange();
    void initOutputDevice();
    bool initDevices();

    bool anyPending() {
        return 
               InitState::Pending == device_output_state
            || InitState::Pending == device_input_state
            || InitState::Pending == device_hello_state
            || InitState::Pending == system_preset_state
            || InitState::Pending == user_preset_state
            || InitState::Pending == apply_favorite_state
            || InitState::Pending == config_state
            || InitState::Pending == handshake;
    }
    bool ready() {
        return !broken 
            && InitState::Complete == device_output_state
            && InitState::Complete == device_input_state
            && InitState::Complete == device_hello_state
            && InitState::Complete == system_preset_state
            && InitState::Complete == user_preset_state
            && InitState::Complete == apply_favorite_state
            && InitState::Complete == config_state
            && InitState::Complete == saved_preset_state
            && InitState::Complete == request_updates_state;
    }

    bool in_preset = false;
    bool in_user_names = false;
    bool in_sys_names = false;
    bool broken = false;
    bool dupe = false;
#ifdef VERBOSE_LOG
    bool log_midi = false;
#endif
    float broken_idle = 0.f;

    // float init_step_phase = 0.f;
    // float init_step_time = 0.f;

    // void begin_init_step(float timeout) {
    //     init_step_phase = 0.f;
    //     init_step_time = timeout;
    // }

    // heart_beating
    float heart_phase = 0.f;
    float heart_time = 1.0;
    bool first_beat = false;
    bool tick_tock = true;
    NVGcolor ledColor = green_light;
    bool heart_beating = true;

    // device management
    std::shared_ptr<MidiDeviceConnection> connection = nullptr;
    std::string device_claim;

    // IMidiDeviceChange
    void onRevokeClaim(const std::string& claim) override
    {
        reboot();
    }
    // ISetDevice
    void setMidiDevice(const std::string & claim) override;

    midi::Output midi_output;
    rack::dsp::RingBuffer<uMidiMessage, 128> midi_dispatch;
    void queueMidiMessage(uMidiMessage msg);
    void dispatchMidi();

    // cc handling

    uint8_t pedal_fraction = 0;

    PedalInfo pedal1 = PedalInfo(0);
    PedalInfo pedal2 = PedalInfo(1);
    PedalInfo & getPedal(uint8_t id) {
        return id ? pedal2 : pedal1;
    }

    bool muted = false;
    int64_t notesOn = 0;
    uint8_t note = 0;
    uint8_t recirculator = 0;
    uint64_t midi_receive_count = 0;
    uint64_t midi_send_count = 0;
    uint8_t dsp[3] {0};
    int data_stream = -1;
    uint8_t ch0_cc_value[127];
    uint8_t ch15_cc_value[127];
    void clearCCValues() { 
        memset(ch0_cc_value, 0, 127); 
        memset(ch15_cc_value, 0, 127);
    }
    //std::vector<uint8_t> system_data;
    uint16_t firmware_version = 0;
    uint16_t cvc_version = 0;
    EM_Hardware hardware = EM_Hardware::Unknown;
    uint8_t middle_c = 60;
    bool reverse_surface;
    Rounding rounding;

    // cv processing
    const int CV_INTERVAL = 64;
    int check_cv = 0;
    const float MIDI_RATE = 0.05f;
    rack::dsp::Timer midi_timer;
    rack::dsp::SchmittTrigger mute_trigger;

    bool isEaganMatrix() { return is_eagan_matrix; }
    bool is_gathering_presets() { return system_preset_state == InitState::Pending || user_preset_state == InitState::Pending; }

    Hc1Module();
    virtual ~Hc1Module();

    // IHandleHcEvents subscription and notification
    void subscribeHcEvents(IHandleHcEvents* client);
    void unsubscribeHcEvents(IHandleHcEvents* client);
    void notifyPresetChanged();
    void notifyPedalsChanged() {
        notifyPedalChanged(0);
        notifyPedalChanged(1);
    }
    void notifyPedalChanged(uint8_t pedal);
    void notifyRoundingChanged();
    void notifyDeviceChanged();
    void notifyDisconnect();
    void notifyFavoritesFileChanged();

    // midi::Input
    void onMessage(const midi::Message& msg) override;

    void paramToDefault(int id) {
        auto pq = getParamQuantity(id);
        getParam(id).setValue(pq->getDefaultValue());
    }
    void paramToMin(int id) {
        auto pq = getParamQuantity(id);
        getParam(id).setValue(pq->getMinValue());
    }
    // For the moment, all knobs default to center, so we don't
    // need separate center/default, and "center" is more intuitive for users
    // void paramToCenter(int id) {
    //     auto pq = getParamQuantity(id);
    //     getParam(id).setValue(pq->getMinValue() + (pq->getMaxValue() - pq->getMinValue()) / 2.f);
    // }
    void centerKnobs();
    //void defaultKnobs();
    void zeroKnobs();
    void absoluteCV();
    void relativeCV();

    json_t *dataToJson() override;
    void dataFromJson(json_t *root) override;
    void onRandomize(const RandomizeEvent& e) override;
    void reboot();

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
    bool isRecirculatorExtend() { return recirculator & EM_Recirculator::Extend; }
    void setRecirculatorCCValue(int id, uint8_t value);

    // ISendMidi
    void sendNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) override;
    void sendNoteOff(uint8_t channel, uint8_t note, uint8_t velocity) override;
    void sendControlChange(uint8_t channel, uint8_t cc, uint8_t value) override;
    void sendProgramChange(uint8_t channel, uint8_t program) override;
    bool readyToSend() override { return ready(); }

    // IPresetHolder
    void setPreset(std::shared_ptr<Preset> preset) override;
    bool isCurrentPreset(std::shared_ptr<Preset> preset) override
    {
        if (!preset) return false;
        if (preset == current_preset) {
            return true;
        }
        if (preset0.name_empty()) {
            return false;
        }
        return preset->is_same_preset(preset0);
    }
    void addFavorite(std::shared_ptr<Preset> preset) override;
    void unFavorite(std::shared_ptr<Preset> preset) override;
    void moveFavorite(std::shared_ptr<Preset> preset, FavoriteMove motion) override;
    void numberFavorites();
    void sortFavorites(PresetOrder order = PresetOrder::Favorite);

    void sendSavedPreset();
    std::shared_ptr<Preset> findDefinedPresetByName(std::string name);
    std::shared_ptr<Preset> findDefinedPreset(std::shared_ptr<Preset> preset);

    void syncStatusLights();
    void syncParam(int paramId);
    void syncParams(float sampleTime);
    void sendResetAllreceivers();
    void transmitDeviceHello();
    void transmitRequestUpdates();
    void transmitRequestConfiguration();
    void transmitRequestSystemPresets();
    void transmitRequestUserPresets();
    void sendEditorPresent(bool init_handshake);
    void silence(bool reset);
    void beginPreset();
    void onNoteOn(uint8_t channel, uint8_t note, uint8_t velocity);
    void onNoteOff(uint8_t channel, uint8_t note, uint8_t velocity);
    void setMacroCCValue(int id, uint8_t value);
    void onSoundOff();
    void onChannelOneCC(uint8_t cc, uint8_t value);
    void onChannel16CC(uint8_t cc, uint8_t value);
    void onChannelOneMessage(const midi::Message& msg);
    void onChannel16Message(const midi::Message& msg);
    void processCV(int inputId);
    void processAllCV();
    void onSave(const SaveEvent& e) override;
    void onRemove(const RemoveEvent& e) override;
    void process(const ProcessArgs& args) override;
};

// ==== Hc1ModuleWidget ========================================

const NVGcolor& InitStateColor(InitState state);
using Hc1p = Hc1Module::Params;
using Hc1in = Hc1Module::Inputs;
using Hc1out = Hc1Module::Outputs;
using Hc1lt = Hc1Module::Lights;

struct Hc1ModuleWidget : ModuleWidget, IPresetHolder, IHandleHcEvents
{
    Hc1Module* my_module = nullptr;
    StaticTextLabel* hardware_label = nullptr;
    StaticTextLabel* device_label = nullptr;
    StaticTextLabel* firmware_label = nullptr;
    TabBarWidget* tab_bar = nullptr;
    FavoriteWidget* favorite = nullptr;
    PresetTab tab = PresetTab::Favorite;
    SquareButton* page_up = nullptr;
    SquareButton* page_down = nullptr;
    GrayModuleLightWidget * status_light = nullptr;
    EMPicker* em_picker = nullptr;
    std::vector<PresetWidget*> presets;
    int page = 0;
    bool have_preset_widgets = false;

    explicit Hc1ModuleWidget(Hc1Module *module);
    virtual ~Hc1ModuleWidget();

    // HC-1-ui.cpp: make ui
    void createPresetGrid();
    void createPresetPaging();
    void createPresetPrevNext();
    void createRoundingLEDs();
#ifdef TRANSPOSE_BUTTONS
    void createTranspose();
#endif
    void createMidiSelection();
    void createDeviceDisplay();
    void createTestNote();
    void createStatusDots();
    void createUi();

    void setTab(PresetTab tab, bool force = false);
    const std::string macroName(int m);
    void pageUp();
    void pageDown();
    void toRelativePreset(int delta);
    void toCategory(uint16_t code);
    void clearPresetWidgets();
    void populatePresetWidgets();
    void updatePresetWidgets();
    bool showCurrentPreset(bool changeTab);

    // IPresetHolder
    void setPreset(std::shared_ptr<Preset> preset) override;
    bool isCurrentPreset(std::shared_ptr<Preset> preset) override;
    void addFavorite(std::shared_ptr<Preset> preset) override;
    void unFavorite(std::shared_ptr<Preset> preset) override;
    void moveFavorite(std::shared_ptr<Preset> preset, FavoriteMove motion) override;

    // IHandleHcEvents
    void onPresetChanged(const PresetChangedEvent& e) override;
    // void onRoundingChanged(const RoundingChangedEvent& e) override {}
    void onDeviceChanged(const DeviceChangedEvent& e) override;
    void onDisconnect(const DisconnectEvent& e) override;
    void onFavoritesFileChanged(const FavoritesFileChangedEvent& e) override;

    // HC-1.draw.cpp
    void drawDSP(NVGcontext* vg);
    void drawStatusDots(NVGcontext* vg);
    void drawPedals(NVGcontext* vg, std::shared_ptr<rack::window::Font> font, bool stockPedals);

    void onHoverScroll(const HoverScrollEvent& e) override;
    void step() override;
    void drawLayer(const DrawArgs& args, int layer) override;
    void draw(const DrawArgs& args) override;

    // HC-1-menu.cpp
    void addSortBy(Menu *menu, std::string name, PresetOrder order);
    void addJumpCategory(Menu *menu, uint16_t category);
    void addRecirculator(Menu *menu, EM_Recirculator kind);
    void addFavoritesMenu(Menu *menu);
    void addSystemMenu(Menu *menu);
    void appendContextMenu(Menu *menu) override;
};

}
#endif