#pragma once
#ifndef EM_MPE_HPP_INCLUDED
#define EM_MPE_HPP_INCLUDED
#include <rack.hpp>
#include "../misc.hpp"
#include "../em_midi.hpp"
using namespace ::rack;
using namespace ::em_midi;
namespace eagan_matrix {

enum class MidiMode : uint8_t {
    Midi,
    Mpe,
    MpePlus
};

inline bool is_z_mpe(EMZ the_z) { return the_z > EMZ::Midi_ChannelPressure; }

struct Mpe
{
    Mpe()
    :   x_bend(96),
        y(EMY::Default_Y),
        z(EMZ::MpePlus_ChannelPressure)
    {
    }

    void clear()
    {
        x_bend = 96;
        y = EMY::Default_Y;
        z = EMZ::MpePlus_ChannelPressure;
    }

    bool operator ==(const Mpe& rhs) const {
        return x_bend == rhs.x_bend
            && y == rhs.y
            && z == rhs.z;
    }
    bool operator !=(const Mpe& rhs) const {
        return x_bend != rhs.x_bend
            || y != rhs.y
            || z != rhs.z;
    }

    MidiMode mode() const {
        switch (z) {
        case EMZ::Mpe_ChannelPressure: return MidiMode::Mpe;
        case EMZ::MpePlus_ChannelPressure: return MidiMode::MpePlus;
        default: return MidiMode::Midi;
        }
    }
    bool is_any_mpe() const { return is_z_mpe(z); }
    bool is_mpe() const { return z == EMZ::Mpe_ChannelPressure; }
    bool is_mpe_plus() const { return z == EMZ::MpePlus_ChannelPressure; }
    bool is_channel_pressure() const { return z >= EMZ::Midi_ChannelPressure; }
    uint8_t get_bend() const { return x_bend; }
    EMY get_y() const { return y; }
    EMZ get_z() const { return z; }

    std::string bend_display()
    {
        if (x_bend <= 96) {
            return pachde::format_string("%d", x_bend);
        } else {
            int ch1Bend = std::max(1, x_bend - 96);
            return pachde::format_string("96:%d", ch1Bend);
        }
    }

    void set_bend_checked(uint8_t bend)
    {
        x_bend = is_any_mpe() ? std::max(U8(12), bend) : bend;
    }

    void set_bend_with_side_effects(uint8_t bend)
    {
        if (bend < 12 && is_any_mpe()) {
            z = EMZ::Midi_ChannelPressure;
        }
        x_bend = bend;
    }

    void set_y_checked(EMY new_y)
    {
        y = is_any_mpe() ? EMY::Default_Y : new_y;
    }

    void set_y_with_side_effects(EMY new_y)
    {
        if (new_y != EMY::Default_Y && is_any_mpe()) {
            z = EMZ::Midi_ChannelPressure;
        }
        y = new_y;
    }

    void set_z_checked(EMZ new_z)
    {
        if ((y == EMY::Default_Y) && (x_bend >= 12) && is_z_mpe(new_z)) {
            z = new_z;
        } else {
            z = new_z;
        }
    }

    void set_z_with_side_effects(EMZ new_z)
    {
        switch (new_z) {
        case EMZ::Midi_ChannelPressure:    set_mode(MidiMode::Midi); break;
        case EMZ::MpePlus_ChannelPressure: set_mode(MidiMode::MpePlus); break;
        case EMZ::Mpe_ChannelPressure:     set_mode(MidiMode::Mpe); break;
        default:
            z = new_z;
            break;
        }
    }

    void set_mode(MidiMode new_mode)
    {
        if (new_mode == mode()) return;
        switch (new_mode)
        {
        case MidiMode::Midi:
            if (is_any_mpe()) {
                z = EMZ::Midi_ChannelPressure;
            }
            break;

        case MidiMode::Mpe:
            x_bend = std::max(U8(12), x_bend);
            y = EMY::Default_Y;
            z = EMZ::Mpe_ChannelPressure;
            break;

        case MidiMode::MpePlus:
            x_bend = std::max(U8(12), x_bend);
            y = EMY::Default_Y;
            z = EMZ::MpePlus_ChannelPressure;
            break;
        }
    }

private:
    uint8_t x_bend;
    EMY y;
    EMZ z;
};

}
#endif