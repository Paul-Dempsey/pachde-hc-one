#include "HC-1.hpp"
#include "midi_input_worker.hpp"
#include "../module_broker.hpp"
#include "../widgets/cc_param.hpp"
#include "../em_device.hpp"

namespace pachde {

void Hc1Module::dispatchMidi()
{
#ifdef VERBOSE_LOG
    if (midi_dispatch.size() > 50) {
        DEBUG("Large queue: %lld", midi_dispatch.size());
    }
#endif
    while (!midi_dispatch.empty()) {
        midi::Message msg;
        auto um = midi_dispatch.shift();
        um.toMidiMessage(msg);

        ++midi_send_count;
        midi_output.sendMessage(msg);
    }
}

void Hc1Module::queueMidiOutMessage(uMidiMessage msg)
{
    if (midi_dispatch.full()) {
        reboot();
    } else {
        midi_dispatch.push(msg);
    }
}

void Hc1Module::sendControlChange(uint8_t channel, uint8_t cc, uint8_t value)
{
    queueMidiOutMessage(uMidiMessage(MidiStatus_CC|channel, cc, value));
}

void Hc1Module::sendProgramChange(uint8_t channel, uint8_t program)
{
    queueMidiOutMessage(uMidiMessage(MidiStatus_ProgramChange|channel, program));
}

void Hc1Module::sendResetAllreceivers()
{
    queueMidiOutMessage(uMidiMessage(0xff));
}

void Hc1Module::send_init_midi_rate(EMMidiRate rate)
{
    if (init_midi_rate == rate) return;
    switch (rate) {
    case EMMidiRate::Full: sendControlChange(EM_SettingsChannel, EMCC_Download, EM_DownloadItem::midiTxFull); break;
    case EMMidiRate::Third: sendControlChange(EM_SettingsChannel, EMCC_Download, EM_DownloadItem::midiTxThird); break;
    case EMMidiRate::Twentieth: sendControlChange(EM_SettingsChannel, EMCC_Download, EM_DownloadItem::midiTxTweenth); break;
    }
    init_midi_rate = rate;
}

void Hc1Module::restore_midi_rate() {
    if (EMMidiRate::Full != init_midi_rate) {
        sendControlChange(EM_SettingsChannel, EMCC_Download, EM_DownloadItem::midiTxFull);
        init_midi_rate = EMMidiRate::Full;
    }
}

void Hc1Module::transmitDeviceConfig()
{
    beginPreset(); // force preset
    sendControlChange(EM_SettingsChannel, EMCC_Download, EM_DownloadItem::configToMidi);
}

void Hc1Module::transmitRequestConfiguration()
{
    beginPreset(); // force preset
    sendControlChange(EM_SettingsChannel, EMCC_Download, EM_DownloadItem::gridToFlash);
    sendControlChange(EM_SettingsChannel, EMCC_Download, EM_DownloadItem::configToMidi);
}

void Hc1Module::transmitRequestSystemPresets()
{
    clearCCValues();
    slot_count = 0;
    system_presets.clear();
    // consider: save/restore EM MIDI routing to disable surface > midi/cvc to avoid interruption while loading
    sendControlChange(EM_SettingsChannel, EMCC_Download, EM_DownloadItem::sysToMidi);
}

void Hc1Module::transmitRequestUserPresets()
{
    clearCCValues();
    slot_count = 0;
    user_presets.clear();
    // consider: save/restore EM MIDI routing to disable surface > midi/cvc to avoid interruption while loading
    sendControlChange(EM_SettingsChannel, EMCC_Download, EM_DownloadItem::userToMidi);
}

void Hc1Module::setPreset(std::shared_ptr<Preset> preset)
{
    beginPreset(); // force preset
    notifyPresetChanged();
    notifyPedalsChanged();

    current_preset = preset;
    if (!preset) return;

    auto phase = get_phase(InitPhase::PresetConfig);
    phase->pend();
    send_init_midi_rate(phase->midi_rate);
    sendEditorPresent();
    sendControlChange(EM_SettingsChannel, EMCC_Download, EM_DownloadItem::gridToFlash);
    sendControlChange(EM_SettingsChannel, MidiCC_BankSelect, preset->bank_hi);
    sendControlChange(EM_SettingsChannel, EMCC_Category, preset->bank_lo);
    sendProgramChange(EM_SettingsChannel, preset->number);
}

void Hc1Module::sendSavedPreset()
{
    sendEditorPresent();
    sendControlChange(EM_SettingsChannel, EMCC_Download, EM_DownloadItem::gridToFlash);
    sendControlChange(EM_SettingsChannel, MidiCC_BankSelect, saved_preset->bank_hi);
    sendControlChange(EM_SettingsChannel, EMCC_Category, saved_preset->bank_lo);
    sendProgramChange(EM_SettingsChannel, saved_preset->number);
}

void Hc1Module::silence(bool reset)
{
    for (uint8_t channel = 0; channel < 12; ++channel) {
        if (0 == channel || 1 == channel) {
            sendControlChange(channel, MidiCC_Sustain, 0);
        }
        sendControlChange(channel, MidiCC_AllNotesOff, 127);
        sendControlChange(channel, MidiCC_AllSoundOff, 127);
        if (reset) {
            sendControlChange(channel, MidiCC_Reset, 127);
        }
    }
}

void Hc1Module::sendSurfaceDirection(bool reverse)
{
    em.reverse_surface = reverse; // doesn't get echoed back, so track here
    sendControlChange(EM_SettingsChannel, EMCC_Reverse_Rounding, (static_cast<uint8_t>(em.rounding.kind) << 1) | (reverse ? 1 : 0));
}

void Hc1Module::sendEditorPresent()
{
    sendControlChange(EM_SettingsChannel, EMCC_EditorPresent, tick_tock ? 85 : 42);
    tick_tock = !tick_tock;
}

void Hc1Module::sendNoteOn(uint8_t channel, uint8_t note, uint8_t velocity)
{
    queueMidiOutMessage(uMidiMessage(MidiStatus_NoteOn|channel, note, velocity));
}

void Hc1Module::sendNoteOff(uint8_t channel, uint8_t note, uint8_t velocity)
{
    queueMidiOutMessage(uMidiMessage(MidiStatus_NoteOff|channel, note, velocity));
}

}