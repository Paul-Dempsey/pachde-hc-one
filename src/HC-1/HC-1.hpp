#pragma once
#ifndef HC_ONE_HPP_INCLUDED
#define HC_ONE_HPP_INCLUDED
#include <stdint.h>
#include "../colors.hpp"
#include "../em_midi.hpp"
#include "../em.hpp"
#include "../hc_events.hpp"
#include "../plugin.hpp"
#include "../preset_meta.hpp"
#include "../presets.hpp"
#include "../text.hpp"
#include "../widgets/dsp_widget.hpp"
#include "../widgets/em_picker.hpp"
#include "../widgets/favorite_widget.hpp"
#include "../widgets/label_widget.hpp"
#include "../widgets/preset_widget.hpp"
#include "../widgets/square_button.hpp"
#include "../widgets/tab_bar.hpp"
#include "init_phase.hpp"

using namespace eagan_matrix;
namespace pachde {

//#define VERBOSE_LOG
#include "../debug_log.hpp"
const NVGcolor& StatusColor(StatusItem led);
struct MidiInputWorker;

struct Hc1Module : IPresetHolder, ISendMidi, IMidiDeviceHolder, IMidiDeviceChange, midi::Input, Module
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
        RECIRC_ENABLE_PARAM,

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
        READY_TRIGGER,
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
        RECIRC_ENABLE_LIGHT,
        ROUND_Y_LIGHT, ROUND_INITIAL_LIGHT, ROUND_LIGHT, ROUND_RELEASE_LIGHT,
        //TRANSPOSE_UP_LIGHT, TRANSPOSE_NONE_LIGHT, TRANSPOSE_DOWN_LIGHT,
//        FILTER_LIGHT,
        NUM_LIGHTS
    };

    LivePreset preset0;
    int slot_count;
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
    bool requireFirmwareVersionMatch(const std::string &path, json_t* root);
    void clearFavorites();
    void saveFavorites();
    bool readFavoritesFile(const std::string& path, bool fresh);
    void writeFavoritesFile(const std::string& path);
    void openFavoritesFile(const std::string& path);
    void importHEGroupFile(const std::string& path);
    json_t* favoritesToJson();

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
    std::string startupConfigPath();
    std::string userPresetsPath();
    std::string systemPresetsResPath();
    std::string systemPresetsPath();
    void saveStartupConfig();
    void loadStartupConfig();
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
            default:
                assert(false);
                return system_presets;
        }
    }
    
    bool is_eagan_matrix = false;

    //
    // initialization phases
    //

    InitPhase current_phase{InitPhase::None};
    float phase_time = 0.f;
    bool phase_pause = false;
    int phase_attempt = 0;
    void process_init_phase(const ProcessArgs& args);

    std::vector<InitPhaseInfo> init_phase = {
        //            Phase                      State                     MIDI rate              Post delay  Budget
        InitPhaseInfo{InitPhase::DeviceOutput,   InitState::Uninitialized, EMMidiRate::Full,      4.0f,       0.f },
        InitPhaseInfo{InitPhase::DeviceInput,    InitState::Uninitialized, EMMidiRate::Full,      3.0f,       0.f },
        InitPhaseInfo{InitPhase::DeviceHello,    InitState::Uninitialized, EMMidiRate::Third,     2.0f,       4.f },
        InitPhaseInfo{InitPhase::DeviceConfig,   InitState::Uninitialized, EMMidiRate::Third,     1.0f,       4.f },
        InitPhaseInfo{InitPhase::CachedPresets,  InitState::Uninitialized, EMMidiRate::Full,      0.f,        0.f },
        InitPhaseInfo{InitPhase::UserPresets,    InitState::Uninitialized, EMMidiRate::Third,     1.0f,      12.f },
        InitPhaseInfo{InitPhase::SystemPresets,  InitState::Uninitialized, EMMidiRate::Twentieth, 1.0f,      25.f },
        InitPhaseInfo{InitPhase::Favorites,      InitState::Uninitialized, EMMidiRate::Full,      0.f,        0.f },
        InitPhaseInfo{InitPhase::SavedPreset,    InitState::Uninitialized, EMMidiRate::Full,      1.0f,       4.f },
        InitPhaseInfo{InitPhase::PresetConfig,   InitState::Uninitialized, EMMidiRate::Full,      1.0f,       4.f },
        InitPhaseInfo{InitPhase::RequestUpdates, InitState::Uninitialized, EMMidiRate::Full,      0.f,        0.f },
        InitPhaseInfo{InitPhase::Heartbeat,      InitState::Uninitialized, EMMidiRate::Full,      0.0f,       2.f },
        InitPhaseInfo{InitPhase::Done,           InitState::Uninitialized, EMMidiRate::Full,      0.f,        0.f }
    };

    InitPhaseInfo* get_phase(InitPhase phase) { return phase == InitPhase::None ? nullptr : &init_phase[PhaseIndex(phase)]; }
    InitState phaseState(InitPhase phase) { return get_phase(phase)->state; }
    float phase_post_delay(InitPhase phase) { return get_phase(phase)->post_delay; }

    void fresh_phase(InitPhase phase)  { get_phase(phase)->state = InitState::Uninitialized; }
    void pend_phase(InitPhase phase)   { get_phase(phase)->state = InitState::Pending; }
    void finish_phase(InitPhase phase) { get_phase(phase)->state = InitState::Complete; }
    void break_phase(InitPhase phase)  { get_phase(phase)->state = InitState::Broken; }

    bool fresh(InitPhase phase) { return get_phase(phase)->fresh(); }
    bool pending(InitPhase phase) { return get_phase(phase)->pending(); }
    bool finished(InitPhase phase) { return get_phase(phase)->finished(); }
    bool broken_phase(InitPhase phase) { return get_phase(phase)->broken(); }

    bool anyPending() { return init_phase.cend() != std::find_if(init_phase.cbegin(), init_phase.cend(), [](const InitPhaseInfo& info){ return InitState::Pending == info.state; }); }
    bool anyBroken() { return init_phase.cend() != std::find_if(init_phase.cbegin(), init_phase.cend(), [](const InitPhaseInfo& info){ return InitState::Broken == info.state; }); }

    bool allComplete() { return init_phase.cend() == std::find_if(init_phase.cbegin(), init_phase.cend(), [](const InitPhaseInfo& info){ return InitState::Complete != info.state; }); }

    bool is_gathering_presets() { return pending(InitPhase::SystemPresets) || pending(InitPhase::UserPresets); }
    bool hasSystemPresets() { return finished(InitPhase::SystemPresets) && !system_presets.empty(); }
    bool hasUserPresets() { return finished(InitPhase::UserPresets) && !user_presets.empty(); }
    bool hasFavorites()  { return finished(InitPhase::Favorites) && !favorite_presets.empty(); }

    EMMidiRate init_midi_rate = EMMidiRate::Full;
    void send_init_midi_rate(EMMidiRate rate);
    void restore_midi_rate();

    void tryCachedPresets();

    void initOutputDevice();

    bool ready() { return !broken && allComplete(); }

    bool in_preset = false;
    bool in_user_names = false;
    bool in_sys_names = false;
    bool broken = false;
    float broken_idle = 0.f;

    rack::dsp::Timer device_sync;
    const float DEVICE_SYNC_PERIOD = 5.f;

    bool first_beat = false;
    bool ready_sent = false;
    bool tick_tock = true;
    NVGcolor ledColor = green_light;

    // device management
    std::shared_ptr<MidiDeviceConnection> connection = nullptr;
    std::string device_claim;

    // IMidiDeviceChange
    void onRenewClaim() override;

    // IMidiDeviceHolder
    void setMidiDeviceClaim(const std::string & claim) override;
    const std::string& getMidiDeviceClaim() override;
    bool in_reboot = false;

    MidiInputWorker* midi_input_worker{nullptr};
    midi::Output midi_output;
    rack::dsp::RingBuffer<uMidiMessage, 128> midi_dispatch;
    void queueMidiOutMessage(uMidiMessage msg);
    void dispatchMidi();
    const float MIDI_RATE = 0.05f;
    rack::dsp::Timer midi_timer;

    // cc handling

    uint8_t pedal_fraction = 0;
    PedalInfo & getPedal(uint8_t id) {
        return id ? em.pedal2 : em.pedal1;
    }

    bool muted = false;
    int64_t notesOn = 0;
    uint8_t note = 0;
    uint64_t midi_receive_count = 0;
    uint64_t midi_send_count = 0;
    int data_stream = -1;
    uint8_t ch0_cc_value[127];
    uint8_t ch15_cc_value[127];
    void clearCCValues() { 
        memset(ch0_cc_value, 0, 127); 
        memset(ch15_cc_value, 0, 127);
    }

    IDsp* dsp_client = nullptr;

    EaganMatrix em;

    // cv processing
    const int CV_INTERVAL = 64;

    rack::dsp::SchmittTrigger mute_trigger;
    rack::dsp::PulseGenerator ready_trigger;

    bool isEaganMatrix() { return is_eagan_matrix; }

    Hc1Module();
    virtual ~Hc1Module();
    void Uninit();
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
    void notifyCompressorChanged();
    void notifyTiltEqChanged();
    void notifyDeviceChanged();
    void notifyDisconnect();
    void notifyFavoritesFileChanged();

    // midi::Input
    void onMessage(const midi::Message& msg) override;
    void onMidiMessage(uMidiMessage msg);

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
    void centerMacroKnobs();
    //void defaultKnobs();
    void zeroKnobs();
    void absoluteCV();
    void relativeCV();

    json_t *dataToJson() override;
    void dataFromJson(json_t *root) override;
    void onRandomize(const RandomizeEvent& e) override;
    void reboot();

    EM_Recirculator recirculatorType() { return em.recirculator.kind(); }
    bool isExtendRecirculator() { return em.recirculator.extended(); }
    bool isRecirculatorDisabled() { return em.recirculator.disabled(); }
    const std::string recirculatorName() { return em.recirculator.name(); }
    const std::string recirculatorParameterName(int r) { return em.recirculator.parameter_name(r); }
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
    void transmitDeviceConfig();
    void transmitRequestConfiguration();
    void transmitRequestSystemPresets();
    void transmitRequestUserPresets();
    void sendSurfaceDirection(bool reverse);
    void sendEditorPresent();
    void silence(bool reset);
    void beginPreset();
    void clearPreset();
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
    void processReadyTrigger(bool ready, const ProcessArgs& args);
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

struct Hc1ModuleWidget : ModuleWidget, IPresetHolder, IHandleHcEvents, IDsp
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
    DspWidget* dsp_widget = nullptr;
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

    // IDsp
    void set_dsp_ready(bool ready) override;
    void set_dsp_value(int index, uint8_t value) override;

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
    void drawStatusDots(NVGcontext* vg);
    void drawPedalAssignment(NVGcontext* vg, float x, float y, char ped_char, uint8_t ped, uint8_t ped_value);
    void drawPedals(NVGcontext* vg, std::shared_ptr<rack::window::Font> font, bool stockPedals);
    std::string getBannerText(NVGcontext* vg, std::shared_ptr<rack::window::Font> normal, std::shared_ptr<rack::window::Font> bold);

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