#pragma once
#ifndef EM_DEVICE_HPP_INCLUDED
#define EM_DEVICE_HPP_INCLUDED
#include <rack.hpp>
using namespace ::rack;

namespace pachde {

bool ExcludeDriver(const std::string& driver_name);
bool is_EMDevice(const std::string& device_name);
std::string FilterDeviceName(const std::string& text);

enum class MidiDeviceKind : uint8_t {
    Unknown,
    Input,
    Output
};

struct MidiDeviceIdentity
{
    MidiDeviceKind kind;
    std::string driver_name;
    std::string device_name;
    int sequence;

    explicit MidiDeviceIdentity() : sequence(-2) {}

    MidiDeviceIdentity(const std::string & driver, const std::string & device, int nth = 0)
    :   kind(MidiDeviceKind::Unknown),
        driver_name(driver),
        device_name(device),
        sequence(nth)
    {}
    
    bool ok()
    {
        return kind != MidiDeviceKind::Unknown
            && sequence >= -1
            && !driver_name.empty()
            && !device_name.empty();
    }

    void clear()
    {
        kind = MidiDeviceKind::Unknown;
        sequence = -1;
        driver_name = "";
        device_name = "";
    }

    std::string toSpec();
    bool parse(const std::string& spec);
    const std::string& driver() const { return driver_name; }
    const std::string& device() const { return device_name; }
    int nth() { return sequence; }
    MidiDeviceKind getKind() { return kind; }
};

struct MidiDevice
{
    int driver_id;
    int device_id;
    MidiDeviceIdentity identity;

    MidiDevice()
    :   driver_id(-1),
        device_id(-1),
    {}

    MidiDevice(int driver, int device, MidiDeviceIdentity ident)
    :   device_id(device),
        driver_id(driver),
        identity(ident)
    {}

    bool instantiate(const std::string& spec);

    bool ok()
    {
        return identity.ok()
            && driver_id >= 0
            && device_id >= 0;
    }
};

}
#endif