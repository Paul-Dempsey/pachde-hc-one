#include "HC-1.hpp"
#include "midi_input_worker.hpp"
#include "../module_broker.hpp"
#include "../widgets/cc_param.hpp"
#include "../em_device.hpp"

namespace pachde {

void Hc1Module::onRenewClaim()
{
    if (device_claim.empty()) return;
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
                midi_dispatch.clear();
                midi_output.reset();
                Input::reset();
                fresh_phase(InitPhase::DeviceInput);
                current_phase = InitPhase::DeviceInput;
                midi_output.setDeviceId(connection->output_device_id);
                if (connection->output_device_id != midi_output.getDeviceId()) {
                    // subscribing failed
                    midi_output.reset();
                    fresh_phase(InitPhase::DeviceOutput);
                    current_phase = InitPhase::DeviceOutput;
                    return;
                }
                midi_output.setChannel(-1);

                // these should be picked up by input device init
                //Input::setDeviceId(connection->input_device_id);
                //Input::setChannel(-1);
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

void Hc1Module::setMidiDeviceClaim(const std::string & claim)
{
    if (0 == device_claim.compare(claim)) return;
    if (!device_claim.empty()) {
        MidiDeviceBroker::get()->revoke_claim(Module::getId());
    }
    device_claim = claim;
    reboot();
}

const std::string& Hc1Module::getMidiDeviceClaim()
{
    return device_claim;
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

        case EMCC_ActionAesMenuRecirc:
            // TODO: Action, AES rate, Menu font also packed into this byte
            if (value != em.global_ActionAesMenuRecirc) {
                em.global_ActionAesMenuRecirc = value;
                em.recirculator.setDisabled(value & 0x40);
                getParamQuantity(RECIRC_ENABLE_PARAM)->setValue(static_cast<float>(em.recirculator.enabled()));
            }
            break;

        case EMCC_PedalType: {
            auto new_p1 = static_cast<PedalType>(value & 0x07);
            bool p1_change = new_p1 != em.pedal1.type;
            em.pedal1.type = new_p1;

            auto new_p2 = static_cast<PedalType>((value >> 3) & 0x07);
            bool p2_change = new_p2 != em.pedal2.type;
            em.pedal2.type = new_p2;

            if (!in_preset) {
                if (p1_change) notifyPedalChanged(0);
                if (p2_change) notifyPedalChanged(1);
            }
        } break;

        case EMCC_MiddleC:
            em.middle_c = value;
            break;

        case EMCC_TuningGrid: {
            auto new_value = static_cast<Tuning>(value);
            if (em.rounding.tuning != new_value) {
                em.rounding.tuning = new_value;
                if (!in_preset) {
                    notifyRoundingChanged();
                }
            }
        } break;

        case EMCC_Pedal1CC:
            if (value != em.pedal1.cc) {
                em.pedal1.cc = value;
                if (!in_preset) {
                    notifyPedalChanged(0);
                }
            }
            break;

        case EMCC_Pedal2CC:
            if (value != em.pedal2.cc) {
                em.pedal2.cc = value;
                if (!in_preset) {
                    notifyPedalChanged(0);
                }
            }
            break;
            
        case EMCC_CompressorThreshold:
            if (value != em.compressor.threshold) {
                em.compressor.threshold = value;
                if (!in_preset) {
                    notifyCompressorChanged();
                }
            }
            break;

        case EMCC_CompressorAttack:
            if (value != em.compressor.attack) {
                em.compressor.attack = value;
                if (!in_preset) {
                    notifyCompressorChanged();
                }
            }
            break;

        case EMCC_CompressorRatio:
            if (value != em.compressor.ratio) {
                em.compressor.ratio = value;
                if (!in_preset) {
                    notifyCompressorChanged();
                }
            }
            break;
            
        case EMCC_CompressorMix:
            if (value != em.compressor.mix) {
                em.compressor.mix = value;
                if (!in_preset) {
                    notifyCompressorChanged();
                }
            }
            break;

        case EMCC_DataStream: {
            switch (value) {
                case EM_StreamType::Name: {
                    preset0.clear_name();
                    if (data_stream != -1) {
                        broken = true;
                    } else {
                        data_stream = value;
                        if (in_sys_names || in_user_names) {
                            beginPreset();
                        }
                    }
                } break;
 
                case EM_StreamType::ConText: {
                    preset0.clear_text();
                    if (data_stream != -1) {
                        broken = true;
                    } else {
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
                            break;
                        case EM_StreamType::ConText:
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
            em.reverse_surface = static_cast<bool>(value & 1);

            auto kind = static_cast<RoundKind>((value & 0x06) >>1);
            if (kind != em.rounding.kind) {
                em.rounding.kind = kind;
                if (!in_preset) {
                    notifyRoundingChanged();
                }
            }
        } break;

        case EMCC_RecirculatorType:
            em.recirculator.setValue(value);
            getParamQuantity(RECIRC_EXTEND_PARAM)->setValue(isExtendRecirculator() * 1.f);
            break;

        case EMCC_MinPedal1:
            if (value != em.pedal1.min) {
                em.pedal1.min = value;
                if (!in_preset) {
                    notifyPedalChanged(0);
                }
            }
            break;
        case EMCC_MaxPedal1:
            if (value != em.pedal1.max) {
                em.pedal1.max = value;
                if (!in_preset) {
                    notifyPedalChanged(0);
                }
            }
            break;
        case EMCC_MinPedal2:
            if (value != em.pedal2.min) {
                em.pedal2.min = value;
                if (!in_preset) {
                    notifyPedalChanged(1);
                }
            }
            break;
        case EMCC_MaxPedal2:
            if (value != em.pedal2.max) {
                em.pedal2.max = value;
                if (!in_preset) {
                    notifyPedalChanged(1);
                }
            }
            break;

        case EMCC_VersionHigh:
            em.firmware_version = value;
            break;

        case EMCC_VersionLow:
            em.firmware_version = (em.firmware_version << 7) | value;
            break;

        case EMCC_CVCHi_Hardware:
            em.cvc_version = static_cast<uint16_t>(value & 0x03) << 14;
            em.hardware = static_cast<EM_Hardware>((value & 0x7c) >> 2);
            break;

        case EMCC_CVCMid:
            em.cvc_version |= static_cast<uint16_t>(value) << 7;
            break;

        case EMCC_CVCLo:
            em.cvc_version |= static_cast<uint16_t>(value);
            break;

        case EMCC_Download:
            //download_message_id = value;
            switch (value) {
                case EM_DownloadItem::beginUserNames:
                    in_user_names = true;
                    break;

                case EM_DownloadItem::endUserNames: {
                    in_user_names = false;
                    auto phase = get_phase(InitPhase::UserPresets);
                    phase->state = broken ? InitState::Broken : InitState::Complete;
                    if (!broken) {
                        std::sort(user_presets.begin(), user_presets.end(), preset_system_order);
                    }
                } break;

                case EM_DownloadItem::beginSysNames:
                    in_sys_names = true;
                    break;

                case EM_DownloadItem::endSysNames:
                    in_sys_names = false;
                    auto phase = get_phase(InitPhase::SystemPresets);
                    phase->state = broken ? InitState::Broken : InitState::Complete;
                    if (!broken) {
                        std::sort(system_presets.begin(), system_presets.end(), getPresetSort(preset_order));
                    }
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
            if (dsp_client) {
                dsp_client->set_dsp_value(d, pct);
            }
        } break;

        case EMCC_EditorReply: {
            if (pending(InitPhase::DeviceHello)) {
                finish_phase(InitPhase::DeviceHello);
            } else if (pending(InitPhase::Heartbeat)) {
                first_beat = true;
                finish_phase(InitPhase::Heartbeat);
            } else {
                //DebugLog("Extra Editor Reply");
            }
        } break;
    }
}

void Hc1Module::onChannel16Message(const midi::Message& msg)
{
    auto status = GetRawStatus(msg);
    assert(status & 0x80);
    switch (status) {
        case MidiStatus_CC:
            onChannel16CC(GetCC(msg), msg.getValue());
            break;

        case MidiStatus_ProgramChange:
            if (in_preset) {
                in_preset = false;
                preset0.number = msg.getNote();
                if (pending(InitPhase::SavedPreset)) {
#ifdef VERBOSE_LOG
                    if (0 != saved_preset->name.compare(preset0.name())) {
                        DEBUG("Unexpected: saved %s != set: %s", saved_preset->name.c_str(), preset0.name());
                    }
#endif
                    restore_midi_rate();
                    current_preset = findDefinedPreset(saved_preset);
                    finish_phase(InitPhase::SavedPreset);
                    finish_phase(InitPhase::PresetConfig);
                    notifyPresetChanged();
                    notifyPedalsChanged();
                    notifyCompressorChanged();
                    notifyTiltEqChanged();
                } else
                if (pending(InitPhase::DeviceConfig)) {
                    restore_midi_rate();
                    finish_phase(InitPhase::DeviceConfig);
                    notifyDeviceChanged();
                    notifyPresetChanged();
                    notifyPedalsChanged();
                    notifyCompressorChanged();
                    notifyTiltEqChanged();
                } else
                if (pending(InitPhase::PresetConfig)) {
                    restore_midi_rate();
                    if (broken) {
                        break_phase(InitPhase::PresetConfig);
                    } else {
                        finish_phase(InitPhase::PresetConfig);
                    }
                    if (!broken) {
                        if (!preset0.name_empty()) {
                            if (nullptr == current_preset || current_preset->is_same_preset(preset0)) {
                                current_preset = findDefinedPreset(nullptr);
                            }
                        }
                    }
                    notifyPresetChanged();
                    notifyPedalsChanged();
                    notifyCompressorChanged();
                    notifyTiltEqChanged();
                } else
                if (is_gathering_presets()) {
                    if (broken) {
                        restore_midi_rate();
                    }
                    ++slot_count;
                    if (!broken && !preset0.name_empty()) {
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
}

void Hc1Module::onNoteOff(uint8_t channel, uint8_t note, uint8_t velocity)
{
    if (notesOn) { notesOn--; }
}

void Hc1Module::onSoundOff()
{
    if (!in_preset) {
        current_preset = nullptr;
        beginPreset();
        if (!pending(InitPhase::SavedPreset)) {
            pend_phase(InitPhase::PresetConfig);
        }
    }
}

void Hc1Module::onChannelOneCC(uint8_t cc, uint8_t value)
{
    ch0_cc_value[cc] = value;

    // Continuum doesn't send high resolution for pedals
    if (cc == em.pedal1.cc) {
        em.pedal1.value = value;
        if (!in_preset) {
            notifyPedalChanged(0);
        }
    } else if (cc == em.pedal2.cc) {
        em.pedal2.value = value;
        if (!in_preset) {
            notifyPedalChanged(1);
        }
    }

    switch (cc) {
        // case EMCC_Jack1:
        //     DEBUG("EMCC_Jack1 %d", value);
        //     break;
        // case EMCC_Jack2:
        //     DEBUG("EMCC_Jack2 %d", value);
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
            if (em.rounding.rate != value) {
                em.rounding.rate = value;
                notifyRoundingChanged();
            }
        } break;

        case EMCC_RoundInitial: {
            bool new_initial = value; // seems to be 0 or 127
            if (em.rounding.initial != new_initial) {
                em.rounding.initial = new_initial;
                notifyRoundingChanged();
            }
        } break;

        case EMCC_RoundEqual: {
            RoundEqual rq = em.rounding.equal;
            switch (value) {
            case   0: rq = RoundEqual::Disabled; break;
            case  64: rq = RoundEqual::Enabled; break;
            case 127: rq = RoundEqual::Equal; break;
            }
            if (rq != em.rounding.equal) {
                em.rounding.equal = rq;
                notifyRoundingChanged();
            }
        } break;
        
        case EMCC_TiltEq:
            if (value != em.tilt_eq.tilt) {
                em.tilt_eq.tilt = value;
                if (!in_preset) {
                    notifyTiltEqChanged();
                }
            }
            break;

        case EMCC_TiltEqFrequency:
            if (value != em.tilt_eq.frequency) {
                em.tilt_eq.frequency = value;
                if (!in_preset) {
                    notifyTiltEqChanged();
                }
            }
            break;

        case EMCC_TiltEqMix:
            if (value != em.tilt_eq.mix) {
                em.tilt_eq.mix = value;
                if (!in_preset) {
                    notifyTiltEqChanged();
                }
            }
            break;


        case EMCC_CompressorThreshold:
            if (value != em.compressor.threshold) {
                em.compressor.threshold = value;
                if (!in_preset) {
                    notifyCompressorChanged();
                }
            }
            break;

        case EMCC_CompressorAttack:
            if (value != em.compressor.attack) {
                em.compressor.attack = value;
                if (!in_preset) {
                    notifyCompressorChanged();
                }
            }
            break;

        case EMCC_CompressorRatio:
            if (value != em.compressor.ratio) {
                em.compressor.ratio = value;
                if (!in_preset) {
                    notifyCompressorChanged();
                }
            }
            break;
            
        case EMCC_CompressorMix:
            if (value != em.compressor.mix) {
                em.compressor.mix = value;
                if (!in_preset) {
                    notifyCompressorChanged();
                }
            }
            break;

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

void Hc1Module::onMidiMessage(uMidiMessage umsg)
{
    auto msg = umsg.toMidiMessage();
#ifdef VERBOSE_LOG
    if (log_midi) {
        DebugLog("%s", ToFormattedString(msg).c_str());
    }
#endif
    bool loopbackmsg = !isLoopbackDetect(msg);
    if (broken && loopbackmsg) {
        broken_idle = 0.f;
    }

    ++midi_receive_count;

    auto channel = msg.getChannel();
    switch (channel) {
    case EM_MasterChannel: // 0 (channel 1)
        onChannelOneMessage(msg);
        break;

    case 13: // 13 (channel 14)
        break;

    case EM_MatrixChannel:  // 14 (channel 15)
        break;

    case EM_SettingsChannel: // 15 (channel 16)
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
        case MidiStatus_ChannelPressure:
        case MidiStatus_PitchBend:
            break;
        }
        break;
    }
}

void Hc1Module::onMessage(const midi::Message& msg)
{
    midi_input_worker->post_message(uMidiMessage(msg));
}

}