#include "em_device.hpp"
#include "misc.hpp"
#include <unordered_map>

namespace pachde
{

bool is_osmose(const std::string& name)
{
    if (name.empty()) { return false; }
    auto text = to_lower_case(name);
    return std::string::npos != text.find("osmose")
        && std::string::npos != text.find_first_of('2');
}

bool is_EMDevice(const std::string& name)
{
    if (name.empty()) { return false; }
    auto text = to_lower_case(name);
    // Continuum <serial> and ContinuuMini <serial>
    if (0 == text.compare(0, 8, "continuu", 0, 8)) {
        return true;
    }
    // "EaganMatrix Module"
    if (0 == text.compare(0, 11, "eaganmatrix", 0, 11)) {
        return true;
    }
    // Osmose varies depending on OS, and must be Osmose port 2
    return std::string::npos != text.find("osmose")
        && std::string::npos != text.find_first_of('2');
}

#if defined ARCH_MAC
std::string FilterDeviceName(const std::string &raw) { return raw; }
#endif

#if defined ARCH_WIN
std::string FilterDeviceName(const std::string &raw)
{
    if (raw.empty()) return raw;
    std::string text = raw;
    text.erase(text.find_last_not_of("0123456789"));
    return text;
}
#endif

#if defined ARCH_LIN
std::string FilterDeviceName(const std::string &raw)
{
    if (raw.empty()) return raw;
    std::string text = raw;
    auto pos = text.find(':');
    if (std::string::npos != pos)
    {
        return text.substr(0, pos);
    }
    return text;
}
#endif

bool ExcludeDriver(const std::string &name)
{
    auto text = to_lower_case(name);
    if (0 == text.compare(0, 7, "gamepad", 0, 7)) {
        return true;
    }
    if (0 == text.compare(0, 8, "loopback", 0, 8)) {
        return true;
    }
    return std::string::npos != text.find("keyboard");
}

bool ExcludeDevice(const std::string & name)
{
#if defined ARCH_WIN
    return (name.size() >= 28) && (0 == name.compare(0,28, "Microsoft GS Wavetable Synth", 0, 28));
#else
    return false;
#endif
}

// =============================================================

bool MidiDeviceConnectionInfo::parse(const std::string & spec)
{
    clear();
    if (spec.empty()) return false;
    int seq = 0;
    int state = 0;
    for (auto ch: spec) {
        if ('+' == ch) {
            ++state;
            continue;
        }
        switch (state) {
            case 0: input_device_name.push_back(ch); break;
            case 1: output_device_name.push_back(ch); break;
            case 2: driver_name.push_back(ch); break;
            case 3:
                if (std::isdigit(ch)) {
                    seq = (seq * 10) + (ch - '0');
                } else {
                    clear();
                    return false;
                }
                break;
            default:
                clear();
                return false;
        }
    }
    sequence = seq;
    return true;
}

std::string MidiDeviceConnectionInfo::spec() const
{
    if (claim.empty()) {
        auto s = input_device_name;
        s.push_back('+');
        s.append(output_device_name);
        s.push_back('+');
        s.append(driver_name);
        s.push_back('+');
        if (sequence < 10) {
            s.push_back('0' + sequence);
        } else {
            s.append(format_string("%d", sequence));
        }
        const_cast<MidiDeviceConnectionInfo*>(this)->claim = s;
    }
    return claim;
}

std::string MidiDeviceConnectionInfo::friendly(TextFormatLength length) const
{
    std::string result = input_device_name.empty() ? "(Unknown)" : input_device_name;
    switch (length) {
    case TextFormatLength::Compact: {
        if (0 == result.compare(0, 9, "Continuum", 0, 9)) {
            result.replace(0, 10, "Con..");
        } else if (0 == result.compare(0, 12, "ContinuuMini", 0, 12)) {
            result.erase(0, 8); // "Mini"
        } else if (0 == result.compare(0, 18, "EaganMatrix Module", 0 , 18)) {
            result.replace(0, 18, "EMM");
        }
    } break;
    case TextFormatLength::Abbreviated:{
        if (0 == result.compare(0, 9, "Continuum", 0, 9)) {
            result.erase(1, 8); // "C"
        } else if (0 == result.compare(0, 12, "ContinuuMini", 0, 12)) {
            result.replace(0, 12, "M"); // substitute "M"
        } else if (0 == result.compare(0, 18, "EaganMatrix Module", 0 , 18)) {
            result.replace(0, 18, "EMM");
        }
    } break;
    default:
        break;
    }

    if (sequence > 0) {
        result.append(format_string("#%d", sequence));
    }
    
    if (TextFormatLength::Long == length) {
        if (!output_device_name.empty()) {
            result.append(" and ");
            result.append(output_device_name);
        }
        if (!driver_name.empty()) {
            result.append(" on ");
            result.append(driver_name);
        }
    }
    return result;
}

// =============================================================

// bool MidiDeviceConnection::instantiate(const std::string &spec)
// {
//     clear_ids();
//     if (!info.parse(spec)) {
//         return false;
//     }
//
//     for (auto id_driver : midi::getDriverIds())
//     {
//         auto driver = midi::getDriver(id_driver);
//         std::string driver_name = driver->getName();
//         if (match_insensitive(info.driver(), driver_name)) {
//
//             driver_id = id_driver;
//
//             // find input device id
//             int occurrence = 0;
//             for (auto id_device : driver->getInputDeviceIds())
//             {
//                 std::string name = FilterDeviceName(driver->getInputDeviceName(id_device));
//                 if (match_insensitive(info.input(), name))
//                 {
//                     if (info.nth() <= 0 || occurrence == info.nth())
//                     {
//                         input_device_id = id_device;
//                         break;
//                     }
//                     ++occurrence;
//                 }
//             }
//
//             if (input_device_id < 0) return false;
//
//             // find output device id
//             occurrence = 0;
//             for (auto id_device : driver->getOutputDeviceIds())
//             {
//                 std::string name = FilterDeviceName(driver->getOutputDeviceName(id_device));
//                 if (match_insensitive(info.output(), name))
//                 {
//                     if (info.nth() <= 0 || occurrence == info.nth())
//                     {
//                         output_device_id = id_device;
//                         break;
//                     }
//                     ++occurrence;
//                 }
//             }
//             return identified();
//         }
//     }
//     return false;
// }

bool matchInOut(const std::string& input, const std::string& output)
{
    if (0 == input.compare(output)) return true;
#if defined ARCH_WIN   
    if (is_osmose(input) && is_osmose(output)) {
        return (0 == input.compare(0,7, "MIDIIN2", 0,7)) 
            && (0 == output.compare(0,8, "MIDIOUT2", 0,8));
    }
#endif
    return false;
}

std::vector<std::shared_ptr<MidiDeviceConnection>> EnumerateMidiConnections(bool emOnly)
{
    std::vector<std::shared_ptr<MidiDeviceConnection>> result;
    std::map<std::string, int> counts;

    for (auto id_driver : midi::getDriverIds())
    {
        auto driver = midi::getDriver(id_driver);
        if (ExcludeDriver(driver->getName())) {
            continue;
        }

        // collect inputs
        for (auto id_input: driver->getInputDeviceIds()) {
            auto input_name = FilterDeviceName(driver->getInputDeviceName(id_input));
            if (emOnly && !is_EMDevice(input_name)) {
                continue;
            }
            auto item = std::make_shared<MidiDeviceConnection>();
            item->driver_id = id_driver;
            item->input_device_id = id_input;
            item->info.driver_name = driver->getName();
            item->info.input_device_name = input_name;
            auto r = counts.insert(std::make_pair(input_name, 0));
            if (!r.second) {
                r.first->second++;
            }
            item->info.sequence = r.first->second;
            result.push_back(item);
        }

        // match outputs to inputs
        counts.clear();
        for (auto id_out: driver->getOutputDeviceIds()) {
            auto output_name = FilterDeviceName(driver->getOutputDeviceName(id_out));
            if (ExcludeDevice(output_name) || (emOnly && !is_EMDevice(output_name))) {
                continue;
            }
            auto r = counts.insert(std::make_pair(output_name, 0));
            if (!r.second) {
                r.first->second++;
            }
            int seq = r.first->second;
            auto item = std::find_if(result.begin(), result.end(), [&](std::shared_ptr<MidiDeviceConnection>& item) {
                return matchInOut(item->info.input_device_name, output_name) &&
                    item->info.sequence == seq;
            });
            if (item != result.end()) {
                (*item)->output_device_id = id_out;
                if (output_name != (*item)->info.input_device_name) {
                    (*item)->info.output_device_name = output_name;
                }
            } 
            // else {
            //     DEBUG("No match for output device %s:%s:%d", driver->getName().c_str(), output_name.c_str(), seq);
            // }
        }
    }
    return result;
}

// ==== MidiDeviceBroker::Internal =============================

struct MidiDeviceBroker::Internal
{
    std::vector<std::shared_ptr<MidiDeviceConnection>> devices;
    std::map<std::string, int64_t> claims; // spec:module_id

    void clear() {
        devices.clear();
        claims.clear();
    }
    bool empty() { return devices.empty(); }

    void scan_while(std::function<bool(const std::shared_ptr<MidiDeviceConnection>)> pred) {
        for (auto d: devices) {
            if (!pred(d)) break;
        }
    }

    std::shared_ptr<MidiDeviceConnection> get_connection(const std::string& claim) const
    {
        auto it = std::find_if(devices.cbegin(), devices.cend(),
            [claim](const std::shared_ptr<MidiDeviceConnection> p) {
                return 0 == claim.compare(p->info.spec());
            });
        return it == devices.cend() ? nullptr : *it;
    }

    void ensureDevices() {
        if (devices.empty()) {
            devices = EnumerateMidiConnections(false);
        }
    }

    int64_t get_claimant(const std::string& claim) const {
        auto r = claims.find(claim);
        return r != claims.cend() ? r->second : -1;
    }

    ClaimResult claim(int64_t claimant, const std::string& claim)
    {
        if (claim.empty()) return ClaimResult::ArgumentError;
        ensureDevices();
        if (empty()) return ClaimResult::NoMidiDevices;
        if (!get_connection(claim)) {
            return ClaimResult::NotAvailable;
        }
        auto r = claims.insert(std::make_pair(claim, claimant));
        if (r.second) return ClaimResult::Ok; // inserted, so successfully claimed
        return r.first->second == claimant 
            ? ClaimResult::Ok // already claimed by the module, which is fine
            : ClaimResult::AlreadyClaimed; // claimed by another module
    }

    std::string claim_new_device(int64_t claimant)
    {
        ensureDevices();
        revoke_claim(claimant); // you only get one at a time
        if (devices.empty()) return "";
        
        for (auto it = devices.cbegin(); it != devices.cend(); ++it) {
            auto p = *it;
            if (is_EMDevice(p->info.input_device_name)) {
                auto claim = p->info.spec();
                if (-1 == get_claimant(claim)) {
                    claims.insert(std::make_pair(claim, claimant));
                    return claim;
                }
            }
        };
        return "";
    }

    void sync()
    {
        auto new_devices = EnumerateMidiConnections(false);
        bool renew = false;
        if (new_devices.size() == devices.size()) {
            for (auto incoming : new_devices) {
                auto it = std::find_if(devices.begin(), devices.end(), [incoming](std::shared_ptr<MidiDeviceConnection> old) {
                    return old->is_same_connection(incoming);
                });
                if (it == devices.cend()) {
                    renew = true;
                    break;
                }
            }
        } else {
            renew = true;
        }
        if (!renew) return;

        std::vector<int64_t> claimants;
        auto bit = std::back_inserter(claimants);
        for (auto p : claims) {
            *bit++ = p.second;
        }
        claims.clear();
        devices = new_devices;
        for (auto mod : claimants) {
            auto client = dynamic_cast<IMidiDeviceChange*>(APP->engine->getModule(mod));
            if (client) {
                client->onRenewClaim();
            }
        }
    }

    void revoke_claim(const std::string& claim)
    {
        claims.erase(claim);
    }

    void revoke_claim(int64_t claimant)
    {
        auto it = std::find_if(claims.begin(), claims.end(), [claimant](const std::pair<std::string, int64_t>& claim){ return claim.second == claimant; });
        if (it != claims.end()) {
            claims.erase(it);
        }
    }

};

// ==== MidiDeviceBroker =======================================

MidiDeviceBroker * brokerInstance = nullptr;

MidiDeviceBroker::MidiDeviceBroker() : me(new Internal) {}

MidiDeviceBroker* MidiDeviceBroker::get()
{
    if (!brokerInstance) {
        brokerInstance = new MidiDeviceBroker();
    }
    return brokerInstance;
}

MidiDeviceBroker::~MidiDeviceBroker() {
    if (me) delete me;
}

bool MidiDeviceBroker::is_available(const std::string& claim)
{
    return  me->get_claimant(claim) == -1;
}


// everything else is a forwarder
void MidiDeviceBroker::sync() { me->sync(); }

MidiDeviceBroker::ClaimResult MidiDeviceBroker::claim_device(int64_t claimant, const std::string& claim) {
    return me->claim(claimant, claim);
}
std::string MidiDeviceBroker::claim_new_device(int64_t claimant) {
    return me->claim_new_device(claimant);
}
void MidiDeviceBroker::revoke_claim(const std::string& claim) {
    return me->revoke_claim(claim);
}
void MidiDeviceBroker::revoke_claim(int64_t claimant) {
    return me->revoke_claim(claimant);
}
std::shared_ptr<MidiDeviceConnection> MidiDeviceBroker::get_connection(const std::string& claim) const {
    return me->get_connection(claim);
}
void MidiDeviceBroker::scan_while(std::function<bool(const std::shared_ptr<MidiDeviceConnection>)> pred) const {
    me->scan_while(pred);
}
}