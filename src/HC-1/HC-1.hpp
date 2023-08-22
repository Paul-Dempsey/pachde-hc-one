#pragma once
#ifndef HC_ONE_HPP_INCLUDED
#define HC_ONE_HPP_INCLUDED
#include <stdint.h>
#include "../plugin.hpp"
#include "../colors.hpp"
#include "../em_midi.hpp"
#include "../favorite_widget.hpp"
#include "../presets.hpp"
#include "../preset_widget.hpp"
#include "../tab_bar.hpp"
#include "../text.hpp"
#include "../up_down_widget.hpp"
#include "../preset_meta.hpp"

using namespace em_midi;
namespace pachde {

#define VERBOSE_LOG
#include "../debug_log.hpp"

int randomZeroTo(int size);
const NVGcolor& StatusColor(StatusItem status);
bool preset_order(const std::shared_ptr<MinPreset>& p1, const std::shared_ptr<MinPreset>& p2);
bool favorite_order(const std::shared_ptr<MinPreset>& p1, const std::shared_ptr<MinPreset>& p2);

struct Hc1Module : IPresetHolder, ISendMidi, midi::Input, Module
{
    enum Params
    {
        M1_PARAM, M2_PARAM, M3_PARAM, M4_PARAM, M5_PARAM, M6_PARAM,
        R1_PARAM, R2_PARAM, R3_PARAM, R4_PARAM, RMIX_PARAM,
        M1_REL_PARAM, M2_REL_PARAM, M3_REL_PARAM, M4_REL_PARAM, M5_REL_PARAM, M6_REL_PARAM,
        R1_REL_PARAM, R2_REL_PARAM, R3_REL_PARAM, R4_REL_PARAM, RMIX_REL_PARAM,
        NUM_PARAMS,
        NUM_KNOBS = M1_REL_PARAM,
    };
    enum Inputs
    {
        M1_INPUT, M2_INPUT, M3_INPUT, M4_INPUT, M5_INPUT, M6_INPUT,
        R1_INPUT, R2_INPUT, R3_INPUT, R4_INPUT, RMIX_INPUT,
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
        HEART_LIGHT,
        NUM_LIGHTS
    };
    bool relative_param[NUM_KNOBS] {false};

    Preset preset0;
    std::vector<std::shared_ptr<MinPreset>> user_presets;
    std::vector<std::shared_ptr<MinPreset>> system_presets;
    std::vector<std::shared_ptr<MinPreset>> favorite_presets;
    // persistence
    PresetTab tab = PresetTab::User;
    int page[3] = { 0, 0, 0 };
    bool cache_presets = false;
    std::shared_ptr<MinPreset> current_preset = nullptr;
    std::shared_ptr<MinPreset> saved_preset = nullptr;

    std::string favoritesPath();
    void clearFavorites();
    void saveFavorites();
    void readFavorites();
    void readFavoritesFile(const std::string& path);
    void writeFavoritesFile(const std::string& path);
    json_t* favoritesToJson();
    bool bulk_favoriting = false;

    class BulkFavoritingMode {
        Hc1Module* hc1;
    public:
        BulkFavoritingMode(Hc1Module* hc1) : hc1(hc1) {
            assert(hc1);
            hc1->bulk_favoriting = true;
        }
        ~ BulkFavoritingMode() {
            hc1->bulk_favoriting = false;
        }
    };

    std::string presetsPath();
    void savePresets();
    void loadPresets();
    json_t* presetsToJson();

    void setTabPage(PresetTab tab, int tab_page) {
        page[tab]= tab_page;
    }
    int getTabPage(PresetTab tab) {
        return page[tab];
    }

    const std::vector< std::shared_ptr<MinPreset> >& getPresets(PresetTab tab) {
        switch (tab) {
            case PresetTab::User: return user_presets;
            case PresetTab::System: return system_presets;
            case PresetTab::Favorite: return favorite_presets;
            default: assert(false);
        }
    }
    
    bool is_eagan_matrix = false;

    InitState preset_state = InitState::Uninitialized;
    bool hasPresets() { return InitState::Complete == preset_state; }

    InitState config_state = InitState::Uninitialized;
    bool configPending() { return InitState::Pending == config_state; }
    bool hasConfig() { return InitState::Complete == config_state; }

    InitState device_state = InitState::Uninitialized;
    bool hasDevice() { return InitState::Complete == device_state; }

    InitState saved_preset_state = InitState::Uninitialized;
    bool savedPresetPending() { return InitState::Pending == saved_preset_state; }

    InitState handshake = InitState::Uninitialized;
    bool handshakePending() { return InitState::Pending == handshake; }

    InitState requested_updates = InitState::Uninitialized;

    bool stateError() {
        return InitState::Broken == preset_state
            || InitState::Broken == config_state
            || InitState::Broken == device_state
            || InitState::Broken == handshake;
    }
    bool anyPending() {
        return InitState::Pending == preset_state
            || InitState::Pending == config_state
            || InitState::Pending == device_state
            || InitState::Pending == handshake;
    }
    bool ready() {
        return !broken 
            && InitState::Complete == device_state
            && InitState::Complete == preset_state
            && InitState::Complete == config_state
            && InitState::Complete == saved_preset_state
            && InitState::Complete == requested_updates
            && is_eagan_matrix;
    }

    bool in_preset = false;
    bool in_user_names = false;
    bool in_sys_names = false;
    bool broken = false;
#ifdef VERBOSE_LOG
    bool log_midi = false;
#endif
    float broken_idle = 0.f;
    
    // heartbeat
    float heart_phase = 0.f;
    float heart_time = 1.0;
    bool tick_tock = true;
    NVGcolor ledColor = green_light;
    bool heartbeat = true;
	//float blinkPhase = 0.f;

    // device management
    int inputDeviceId = -1;
    std::string device_name;

    // cc handling
    uint8_t pedal_fraction = 0;
    int64_t notesOn = 0;
    uint8_t recirculator = 0;
    int download_message_id = -1; // CC109
    uint64_t midi_receive_count = 0;
    uint16_t firmware_version = 0;
    uint8_t dsp[3] {0};
    int data_stream = -1;

    uint8_t ch0_cc_value[127];
    uint8_t ch15_cc_value[127];
    void clearCCValues() { 
        memset(ch0_cc_value, 0, 127); 
        memset(ch15_cc_value, 0, 127);
    }

    midi::Output midiOutput;

    // cv processing
    const int CV_INTERVAL = 64;
    int check_cv = 0;
    const float MIDI_RATE = 0.01f;
    rack::dsp::Timer midi_timer;

    const std::string deviceName() { return device_name; }
    bool isEaganMatrix() { return is_eagan_matrix; }
    bool is_gathering_presets() { return preset_state == InitState::Pending; }

    Hc1Module();

    // midi::Input
    void onMessage(const midi::Message& msg) override;

    void paramToDefault(int id) {
        auto pq = getParamQuantity(id);
        getParam(id).setValue(pq->getDefaultValue());
    }
    void centerKnobs();

    json_t *dataToJson() override;
    void dataFromJson(json_t *root) override;
    void onRandomize(const RandomizeEvent& e) override;
    void reboot();

    void findEMOut();
    void findEM();

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

    // IPresetHolder
    void setPreset(std::shared_ptr<MinPreset> preset) override;
    bool isCurrentPreset(std::shared_ptr<MinPreset> preset) override
    {
        if (!preset) return false;
        if (preset == current_preset) {
            return true;
        }
        if (preset0.name_empty()) {
            return false;
        }
        return preset->isSamePreset(preset0);
    }
    void addFavorite(std::shared_ptr<MinPreset> preset) override;
    void unFavorite(std::shared_ptr<MinPreset> preset) override;

    void sendSavedPreset();
    std::shared_ptr<MinPreset> findSysUserPreset(std::shared_ptr<MinPreset> preset);

    void orderFavorites(bool sort);

    void sendResetAllreceivers();
    void transmitInitDevice();
    void transmitRequestUpdates();
    void transmitRequestConfiguration();
    void transmitRequestPresets();
    void sendEditorPresent();
    void silence(bool reset);
    void beginPreset();
    void handle_ch16_cc(uint8_t cc, uint8_t value);
    void handle_ch16_message(const midi::Message& msg);
    void onNoteOn(uint8_t channel, uint8_t note, uint8_t velocity);
    void onNoteOff(uint8_t channel, uint8_t note, uint8_t velocity);
    void setMacroCCValue(int id, uint8_t value);
    void onSoundOff();
    void onChannel0CC(uint8_t cc, uint8_t value);
    void handle_ch0_message(const midi::Message& msg);
    void processCV(int inputId);
    void processAllCV();
    void onSave(const SaveEvent& e) override;
    void process(const ProcessArgs& args) override;
};

struct Hc1ModuleWidget : IPresetHolder, ModuleWidget
{
    Hc1Module *my_module = nullptr;

    GrayModuleLightWidget * status_light;
    bool have_preset_widgets = false;
    std::vector<PresetWidget*> presets;
    TabBarWidget* tab_bar;
    PresetTab tab = PresetTab::Favorite;
    int page = 0;
    FavoriteWidget* fave;
    UpDown* page_up;
    UpDown* page_down;

    explicit Hc1ModuleWidget(Hc1Module *module);
    void setTab(PresetTab tab);
    void setPage(PresetTab tab, int page);
    void pageUp();
    void pageDown();
    void clearPresetWidgets();
    void populatePresetWidgets();
    void updatePresetWidgets();

    // IPresetHolder
    void setPreset(std::shared_ptr<MinPreset> preset) override
    {
        if (my_module) my_module->setPreset(preset);
    }
    bool isCurrentPreset(std::shared_ptr<MinPreset> preset) override
    {
        if (my_module) return my_module->isCurrentPreset(preset);
        return false;
    }
    void addFavorite(std::shared_ptr<MinPreset> preset) override
    {
        if (my_module) {
            my_module->addFavorite(preset);
            populatePresetWidgets();
        }
    }
    void unFavorite(std::shared_ptr<MinPreset> preset) override
    {
        if (my_module) {
            my_module->unFavorite(preset);
            populatePresetWidgets();
        }
    }

    void step() override;
    void drawLayer(const DrawArgs& args, int layer) override;
    void draw(const DrawArgs& args) override;
    void appendContextMenu(Menu *menu) override;
};

}
#endif