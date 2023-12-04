#pragma once
#ifndef EM_DEVICE_HPP_INCLUDED
#define EM_DEVICE_HPP_INCLUDED
#include <rack.hpp>
#include <string>
#include "misc.hpp"

using namespace ::rack;
namespace pachde {

bool ExcludeDriver(const std::string& driver_name);
bool is_EMDevice(const std::string& device_name);
std::string FilterDeviceName(const std::string& text);

enum class TextFormatLength
{
    Short,
    Long,
    Compact,
    Abbreviated
};

struct MidiDeviceConnectionInfo
{
    std::string driver_name;
    std::string input_device_name;
    std::string output_device_name;
    int sequence;
    std::string claim;

    explicit MidiDeviceConnectionInfo()
    : sequence(-1)
    {}

    const std::string & input() const {
        return input_device_name;
    }
    const std::string & output() const {
        if (output_device_name.empty()) {
            return input_device_name;
        }
        return output_device_name;
    }
    const std::string & driver() const {
        return driver_name;
    }
    int nth() const { return sequence; }

    void clear() {
        claim.clear();
        driver_name.clear();
        input_device_name.clear();
        output_device_name.clear();
        sequence = -1;
    }
    bool ok() const {
        return sequence >= 0
            && !input_device_name.empty()
            && !driver_name.empty();
    }
    bool parse(const std::string & spec);
    std::string spec() const;
    std::string friendly(TextFormatLength length) const;
};

struct MidiDeviceConnection
{
    int driver_id;
    int input_device_id;
    int output_device_id;
    MidiDeviceConnectionInfo info;

    explicit MidiDeviceConnection()
    : driver_id(-1), input_device_id(-1), output_device_id(-1)
    {}

    void clear_ids() {
        driver_id = input_device_id = output_device_id = -1;
    }

    bool identified() const {
        return driver_id >= 0
            && input_device_id >= 0
            && output_device_id >= 0;
    }

    bool is_same_connection(std::shared_ptr<MidiDeviceConnection> other) const {
        return 0 == info.spec().compare(other->info.spec());
    }
    //bool instantiate(const std::string &spec);
};

std::vector<std::shared_ptr<MidiDeviceConnection>> EnumerateMidiConnections(bool emOnly);

// ==== MidiDeviceBroker =======================================

struct IMidiDeviceChange {
    virtual void onRenewClaim() = 0;
};

struct MidiDeviceBroker {
    struct Internal;
    Internal * me;

    MidiDeviceBroker & operator=(const MidiDeviceBroker &) = delete;
    MidiDeviceBroker(const MidiDeviceBroker&) = delete;

    static MidiDeviceBroker* get();
    ~MidiDeviceBroker();

    enum class ClaimResult {
        ArgumentError = -1,
        AlreadyClaimed = -2,
        NotAvailable = -3,
        NoMidiDevices = -4,
        Ok = 0
    };

    // can claim any known bidirectional midi connection
    ClaimResult claim_device(int64_t claimant, const std::string& claim);

    // always looks for EM device
    std::string claim_new_device(int64_t claimant_module_id);

    bool is_available(const std::string& claim);

    void revoke_claim(const std::string& claim);
    void revoke_claim(int64_t claimant_module_id);
    std::shared_ptr<MidiDeviceConnection> get_connection(const std::string & claim) const;
    void sync();

    // pred returns false to stop scan
    void scan_while(std::function<bool(const std::shared_ptr<MidiDeviceConnection>)> pred) const;

private:
    MidiDeviceBroker();
};

}
#endif