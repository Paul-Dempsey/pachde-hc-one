#include "em_device.hpp"
#include "misc.hpp"

namespace pachde {

// make sure to pass filtered name
bool is_EMDevice(const std::string& name)
{
    auto text = to_lower_case(name);
    if (0 == text.compare(0, 8, "continuu", 0, 8)) { return true; } //Continuum <serial> and ContinuuMini <serial>
    if (0 == text.compare(0, 11, "eaganmatrix", 0, 11)) { return true; } // "EaganMatrix Module" according to the user guide

    // osmose varies depending on OS, and must be Osmose port 2
    if (std::string::npos != text.find("osmose")) {
// #if defined ARCH_MAC
//         // osmose port 2
//         if (0 == text.compare(8, 14, "port 2", 0, 6)) return true;
// #endif
// #if defined ARCH_WIN
//         if (0 == text.compare(0, 8, "midiout2", 0, 8)) return true;
//         if (0 == text.compare(0, 7, "midiin2", 0, 7)) return true;
// #endif
        // should be good enough for both input and output
        if (std::string::npos != text.find_first_of('2')) return true;
    }
    return false;
}

std::string FilterDeviceName(const std::string& raw)
{
    std::string text = raw;

    #if defined ARCH_WIN
    if (!text.empty()) {
        text.erase(text.find_last_not_of("0123456789"));
    }
    #endif

    #if defined ARCH_LIN
    if (!text.empty()) {
        auto pos = text.find(':');
        if (std::string::npos != pos) {
            return text.substr(0, pos);
        }
    }
    #endif

    return text;
}

bool ExcludeDriver(const std::string& name)
{
    auto text = to_lower_case(name);
    if (0 == text.compare(0, 7, "gamepad", 0, 7)) { return true; }
    if (0 == text.compare(0, 8, "loopback", 0, 8)) { return true; }
    if (std::string::npos != text.find("keyboard")) { return true; }
    return false;
}

std::string MidiDeviceIdentity::toSpec()
{
    if (!ok()) return "";
    return format_string("%s:%s:%s:%d",
        MidiDeviceKind::Input == kind ? "in" : "out",
        driver_name.c_str(),
        device_name.c_str(),
        sequence
    );
}

// spec:
// (i[n]|o[ut]):<driver>:<device>[:<sequence#>]
// no spaces allowed except within <driver> or <device>.
// no leading or trailing spaces allowed around <driver> or <device>.
bool MidiDeviceIdentity::parse(const std::string& spec)
{
    enum class PS { start, nc, uc, t, colon, driver, device, seq, error, end };
    clear();
    if (spec.empty()) return false;

    sequence = 0;
    PS state = PS::start;
    for (auto ch: spec) {
        if (PS::end == state) break;
        switch (state) {
        case PS::error:
            clear();
            return false;
        case PS::start:
            switch (ch) {
            case 'i': case 'I': kind = MidiDeviceKind::Input; state = PS::nc; break;
            case 'o': case 'O': kind = MidiDeviceKind::Output; state = PS::uc; break;
            default: state = PS::error; break;
            }
            break;
        case PS::nc: 
            switch (ch){
            case 'n': case 'N': state= PS::colon; break;
            case ':': state = PS::driver; break;
            default: state = PS::error; break;
            }
            break;
        case PS::uc:
            switch (ch) {
            case 'u': case 'U': state= PS::t; break;
            case ':': state = PS::driver; break;
            default: state = PS::error; break;
            }
            break;
        case PS::t:
            switch (ch) {
            case 't': case 'T': state= PS::colon; break;
            default: state = PS::error; break;
            }
            break;
        case PS::colon: 
            state = ':' == ch ? PS::driver : PS::error;
            break;
        case PS::driver:
            if (':' == ch) {
                state = PS::device;
            } else {
                driver_name.push_back(ch);
            }
            break;
        case PS::device:
            if (':' == ch) {
                state = PS::seq;
            } else {
                device_name.push_back(ch);
            }
            break;
        case PS::seq:
            if ('*' == ch) {
                sequence = -1;
                state = PS::end;
            } else
            if (std::isdigit(ch)) {
                sequence = (10 * sequence) + (ch - '0');
            } else {
                state = PS::error;
            }
            break;
        }
    }
    if (ok()) {
        return true;
    } else {
        clear();
        return false;
    }
}

bool MidiDevice::instantiate(const std::string& spec)
{
    if (!identity.parse(spec)) return false;
    int occurrence = 0;

    for (auto id_driver : midi::getDriverIds()) {
        auto driver = midi::getDriver(id_driver);
        std::string name = driver->getName();
        if (match_insensitive(identity.driver(), name)) {
            bool out = identity.getKind() == MidiDeviceKind::Output;
            for (auto id_device: out ? driver->getOutputDeviceIds() : driver->getInputDeviceIds()) {
                std::string name = out ? driver->getOutputDeviceName(id_device) : driver->getInputDeviceName();
                if (match_insensitive(identity.device(), name)) {
                    if (identity.nth() == -1 || occurrence == identity.nth()) {
                        driver_id = id_driver;
                        device_id = id_device;
                        return true;
                    }
                    ++occurrence;
                }
            }
        }
    }
    return false;
}


}