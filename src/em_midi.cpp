#include "em_midi.hpp"
#include "misc.hpp"

namespace em_midi
{

const char * StatusName(uint8_t status) {
    switch (status) {
        case MidiStatus_NoteOff        : return "Note-";
        case MidiStatus_NoteOn         : return "Note+";
        case MidiStatus_PolyKeyPressure: return "KeyPressure";
        case MidiStatus_CC             : return "CC";
        case MidiStatus_ProgramChange  : return "ProgChange";
        case MidiStatus_ChannelPressure: return "ChPressure";
        case MidiStatus_PitchBend      : return "PitchBend";
        case MidiStatus_SysEx          : return "System";
        default: return "(\?\?\?\?)";
    }
}

std::string Printable(char c) {
    if (c >= ' ') return pachde::format_string("%c", c);
    return pachde::format_string("0x%02x", c);
}

const char * midiCC(uint8_t cc) {
    switch (cc) {
        case 0: return "Bank Select MSB";
        case 1: return "ModWheel";
        case 2: return "Breath";
        case 4: return "Foot";
        case 5: return "Portamento time";
        case 6: return "Data MSB";
        case 7: return "Volume";
        case 8: return "Balance";
        case 9: return "(Mono switch)";
        case 10: return "Pan (fine tune)";
        case 11: return "Expression";
        case 12: return "(i)";
        case 13: return "(ii)";
        case 14: return "(iii)";
        case 15: return "(iv)";
        case 16: return "(v)";
        case 17: return "(vi)";
        case 18: return "(Post Master level)";
        case 19: return "(Audio input level)";
        case 20: return "(R-1)";
        case 21: return "(R-2)";
        case 22: return "(R-3)";
        case 23: return "(R-4)";
        case 24: return "(R-Mix)";
        case 25: return "(Round rate)";
        case 26: return "(Pre Master level)";
        case 27: return "(Output attenuation)";
        case 28: return "(Round initial)";
        case 29: return "(Pedal Jack 1)";
        case 30: return "(Pedal Jack 2)";
        case 31: return "(Preset advance)";
//LSBs
        case 32: return "Bank LSB";
        case 33: return "ModWheel LSB";
        case 34: return "Breath LSB";
        case 35: return "cc03 LSB";
        case 36: return "Foot LSB";
        case 37: return "Portamento time LSB";
        case 38: return "Data LSB";
        case 39: return "Volume LSB";
        case 40: return "Balance LSB";
        case 41: return "cc09 LSB";
        case 42: return "cc10 LSB";
        case 43: return "cc11 LSB";
        case 44: return "cc12 LSB";
        case 45: return "cc13 LSB";
        case 46: return "cc14 LSB";
        case 47: return "cc15 SB";
        case 48: return "cc16 LSB";
        case 49: return "cc17 LSB";
        case 50: return "cc18 LSB";
        case 51: return "cc19 LSB";
        case 52: return "cc20 LSB";
        case 53: return "cc21 LSB";
        case 54: return "cc22 LSB";
        case 55: return "cc23 LSB";
        case 56: return "cc24 LSB";
        case 57: return "cc25 LSB";
        case 58: return "cc26 LSB";
        case 59: return "cc27 LSB";
        case 60: return "cc28 LSB";
        case 61: return "cc29 LSB";
        case 62: return "cc30 LSB";
        case 63: return "cc31 LSB";
        case 64: return "Sustain";
        case 65: return "PortamentoSwitch (rounding)";
        case 66: return "Sostenuto";
        case 67: return "Soft";
        case 68: return "Legato";
        case 69: return "Hold (Sos2)";
        case 70: return "SoundVariation (MPEZ MSB)";
        case 71: return "Intensity";
        case 72: return "Release";
        case 73: return "Attack";
        case 74: return "Brightness (Y)";
        case 75: return "Sound6";
        case 76: return "Sound7";
        case 77: return "Sound8";
        case 78: return "Sound9";
        case 79: return "Sound10";
        case 80: return "GP5";
        case 81: return "GP6 (Compression rate)";
        case 82: return "GP7 (Compression time)";
        case 83: return "GP8 (Tilt EQ)";
        case 84: return "Portamento control (Eq Freq)";
        case 85: return "(Eq Mix)";
        case 86: return "(Macro/Pedal fraction)";
        case 87: return "(Bend/Y/Z fraction)";
        case 88: return "Multisplit low note for channel";
        case 89: return "Multisplit high note for channel";
        case 91: case 92: case 93: case 94: case 95: return "Effect depth";
        case 96: return "DataIncrement";
        case 97: return "DataDecrement";
        case 98: return "NRPN_LSB";
        case 99: return "NRPN_MSB";
        case 100: return "RPN_LSB";
        case 101: return "RPN_MSB";
        case 102: return "(MPE Z LSB)";
        case 106: return "(MPE Y LSB)";
        case 117: return "(Loopback detect)"; break;
        case 120: return "AllSoundOff";
        case 121: return "Reset";
        case 122: return "LocalControl";
        case 123: return "AllNotesOff";
        case 124: return "OmniOff";
        case 125: return "OmniOn";
        case 126: return "Mono";
        case 127: return "Poly";

        default: return "(undefined)";
    }
}

const char * continuumCC(uint8_t cc) {
    switch (cc) {
        case 0: return "Bank Select MSB";
        case 6: return "Data MSB";
        case 8: return "Octave shift";
        case 9: return "Mono switch";
        case 10: return "Fine tune";
        case 11: return "Expression?";
        case 12: return "i";
        case 13: return "ii";
        case 14: return "iii";
        case 15: return "iv";
        case 16: return "v";
        case 17: return "vi";
        case 18: return "Post Master level";
        case 19: return "Audio input level";
        case 20: return "R-1";
        case 21: return "R-2";
        case 22: return "R-3";
        case 23: return "R-4";
        case 24: return "R-Mix";
        case 25: return "Round rate";
        case 26: return "Pre Master level";
        case 27: return "Output attenuation";
        case 28: return "Round initial";
        case 29: return "Pedal Jack 1";
        case 30: return "Pedal Jack 2";
        case 31: return "Preset advance";
        case 32: return "Bank LSB";
        case 33: return "Action/AES";
        case 34: return "Algorithm";
        case 35: return "Program #";
        case 36: return "Routing";
        case 37: return "Pedal type";
        case 38: return "Data LSB";
        case 39: return "Polyphony";
        case 40: return "Pitch bend range (semitones)";
        case 41: return "Y cc";
        case 42: return "Z cc";
        case 43: return "Note handliing";
        case 44: return "Middle C position";
        case 45: return "Split point (note number)";
        case 46: return "Mono function";
        case 47: return "Recirculator column";
        case 48: return "Mono Interval";
        case 49: return "Note Priority";
        //
        case 51: return "Tuning: 0 default, 1-50 n-tone equal, 60-71 just";
        case 52: return "Pedal 1 cc";
        case 53: return "Pedal 2 cc";
        case 54: return "Pedal octave shift amount";
        case 55: return "Setting Preservation";
        case 56: return "Data Stream";
        //
        case 59: return "Dim menu";
        //
        case 60: return "Touch center";
        case 61: return "Reverse pitch";
        case 62: return "Recirculator type";
        case 63: return "CVC configuration";
        case 64: return "Sustain";
        case 65: return "Rounding override";
        case 66: return "Sos 1";
        case 67: return "Headphone level";
        case 68: return "Line level";
        case 69: return "Sos 2";
        case 70: return "Actuation";
        case 71: return "Total traditional polyphony";
        case 72: return "Total DSP polyphony";
        case 73: return "Total CVC polyphony";
        //
        case 75: return "Stress test";
        case 76: return "Pedal 1 min";
        case 77: return "Pedal 1 max";
        case 78: return "Pedal 2 min";
        case 79: return "Pedal 2 max";
        case 80: return "Q Bias (obsolete)";
        case 81: return "(old) Compression rate";
        case 82: return "(old) Compression time";
        case 83: return "Tilt EQ";
        case 84: return "EQ Freq";
        case 85: return "EQ Mix";
        case 86: return "Macro/Pedal fraction";
        case 87: return "Bend/Y/Z fraction";
        case 90: return "Compressor Threshhold";
        case 91: return "Compressor Attack";
        case 92: return "Compressor Ratio";
        case 93: return "Compressor Mix";
        //
        case 98 : return "MPE+ lo NRPN select";
        case 99 : return "MPE+ hi NRPN select";
        case 100: return "MPE lo RPN select";
        case 101: return "MPE hi RPN select";
        case 102: return "Firmware version hi";
        case 103: return "Firmware version lo";
        case 104: return "Hardware/CVC hi";
        case 105: return "CVC mid";
        case 106: return "CVC lo";
        case 107: return "SNBN a";
        case 109: return "Editor message";
        case 110: return "HE<>Device info";
        case 111: return "Device status";
        //
        case 113: return "SNBN b";
        case 114: return "DSP %";
        case 115: return "Log dump";
        case 116: return "Editor present";
        case 117: return "Loopback detect";
        case 118: return "Editor reply";
        case 119: return "archive no-op";
        case 120: return "All sound off";
        case 122: return "CRC 0 7'";
        case 123: return "CRC 1 7'";
        case 124: return "CRC 2 7'";
        case 125: return "CRC 3 7'";
        case 126: return "CRC 5 4'";
        case 127: return "MPE Polyphony";
        default: return "(available)";
    }

}

std::string ToFormattedString(const midi::Message& msg) {
    auto status = GetRawStatus(msg);
    auto status_name = StatusName(status);
    switch (status) {
        case MidiStatus_NoteOff:
        case MidiStatus_NoteOn:
        case MidiStatus_PolyKeyPressure:
        case MidiStatus_PitchBend:
            return pachde::format_string("ch%2d %s %3d %3d",
                msg.getChannel(),
                status_name,
                msg.bytes[1],
                msg.bytes[2]
                );

        case MidiStatus_CC: {
            auto channel = msg.getChannel();
            return pachde::format_string("ch%2d %s %3d %3d %s",
                channel,
                status_name,
                msg.bytes[1],
                msg.bytes[2],
                (channel == 15) 
                    ? continuumCC(msg.bytes[1])
                    : (channel == 14) ? "EM data" : midiCC(msg.bytes[1])
                );
        } break;

        case MidiStatus_ProgramChange:
            return pachde::format_string("ch%2d %s %3d",
                msg.getChannel(),
                status_name,
                msg.bytes[1]
                );

        case MidiStatus_ChannelPressure:
            return pachde::format_string("ch%2d %s %3d (%s)",
                msg.getChannel(),
                status_name,
                msg.bytes[1],
                Printable(msg.bytes[1]).c_str()
                );

        case MidiStatus_SysEx:
            return status_name;

        default:
            return msg.toString();
    }
}


const char * RecirculatorName(EM_Recirculator r)
{
    switch (r) {
        case EM_Recirculator::Reverb:         return "Reverb";
        case EM_Recirculator::ModDelay:       return "Mod Delay";
        case EM_Recirculator::SweptEcho:      return "Swept Echo";
        case EM_Recirculator::AnalogEcho:     return "Analog Echo";
        case EM_Recirculator::DigitalEchoLPF: return "Delay LPF";
        case EM_Recirculator::DigitalEchoHPF: return "Delay HPF";
        default: return "";
    }
}

const char * RecirculatorParameterName(EM_Recirculator r_type, int r)
{
    if (r < 1 || r > 5) return "";
    if (r == 5) return "Mix";
    switch (r_type) {
        case EM_Recirculator::Reverb:
            switch (r) {
                case 1: return "Diffuse";
                case 2: return "Darkness";
                case 3: return "Damping";
                case 4: return "Decay";
            };
            break;
        case EM_Recirculator::ModDelay:
        case EM_Recirculator::SweptEcho:
            switch (r) {
                case 1: return "Depth";
                case 2: return "Rate";
                case 3: return "Feedback";
                case 4: return "Time";
            };
            break;
        case EM_Recirculator::AnalogEcho:
            switch (r) {
                case 1: return "Noise";
                case 2: return "Offset";
                case 3: return "Feedback";
                case 4: return "Time";
            };
            break;
        case EM_Recirculator::DigitalEchoLPF:
            switch (r) {
                case 1: return "LPF";
                case 2: return "Offset";
                case 3: return "Feedback";
                case 4: return "Time";
            };
            break;
        case EM_Recirculator::DigitalEchoHPF:
            switch (r) {
                case 1: return "HPF";
                case 2: return "Offset";
                case 3: return "Feedback";
                case 4: return "Time";
            };
            break;
        default:
            break;
    }
    return "?!!?";
}

const char * DownloadMessage(int id)
{
    if (id < 0) return "";
    switch (id) {
        case EM_DownloadItem::reduceGain: return "Reduce gain";
        case EM_DownloadItem::reducePoly: return "Reduce polyphony";
        case EM_DownloadItem::remakeSRMahl: return "Remake QSPI Data";
        case EM_DownloadItem::inFactCalib: return "Recalibrating...";
        case EM_DownloadItem::midiLoopback: return "MIDI loppback deteced";
        case EM_DownloadItem::eraseMessage: return "";
        case EM_DownloadItem::doFactCalib: return "Factory recalibrartion required";
        case EM_DownloadItem::usbTxNoAck: return "mini midi no ack";
        case EM_DownloadItem::rxOver: return "Midi rx overflow";
        case EM_DownloadItem::txOver: return "Midi tx overflow";
        case EM_DownloadItem::rxSynErr: return "Midi rx syntax error";
        case EM_DownloadItem::rxBitErr: return "Midi rx bad bit widths";
        case EM_DownloadItem::sensComm: return "serial sensors error";
        case EM_DownloadItem::nanErr: return "output has NaN";
        case EM_DownloadItem::ceeSeq: return "CEE Communications error";
    }
    return nullptr;
}


} // em_midi
