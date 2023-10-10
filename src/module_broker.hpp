#pragma once
#ifndef MODULE_BROKER_HPP_INCLUDED
#define MODULE_BROKER_HPP_INCLUDED
#include "./HC-1/HC-1.hpp"
// #include "./HC-2/HC-2.hpp"
// #include "./HC-3/HC-3.hpp"

namespace pachde {

struct ModuleBroker
{
    struct Internal;
    Internal * my;

    ~ModuleBroker();
    ModuleBroker & operator=(const ModuleBroker &) = delete;
    ModuleBroker(const ModuleBroker&) = delete;

    static ModuleBroker* get();

    void registerHc1(Hc1Module * module);
    void unregisterHc1(Hc1Module * module);
    int Hc1count();
    Hc1Module* getSoleHc1();
    Hc1Module* getHc1(std::function<bool(Hc1Module* const&)> pred);
    Hc1Module* getHc1(int64_t id);

    // pred returns false to stop scan
    void scan_while(std::function<bool(Hc1Module* const&)> pred);

private:
    ModuleBroker();
};

struct DeviceAssociation {
    const std::string device_name;
    int64_t module_id;

    DeviceAssociation(std::string device, int64_t id)
    : device_name(device), module_id(id)
    {}

    static std::vector<DeviceAssociation> getList()
    {
        std::vector<DeviceAssociation> list;
        auto one = ModuleBroker::get();
        one->scan_while([&](Hc1Module* const& mod){
            list.push_back(DeviceAssociation{mod->deviceName(), mod->getId()});
            return true;
        });
        return list;
    }
};

struct PartnerBinding
{
    int64_t module_id;
    std::string device_name;

    PartnerBinding() : module_id(-1) {}

    void setDevice(std::string device) { device_name = device; }
    void forgetModule() { module_id = -1; }
    void forgetDevice() { device_name = ""; }

    Hc1Module* getPartner()
    {
        auto one = ModuleBroker::get();

        // If no HC-1, forget any we remember and give up (but still remember
        // the device name, if one comes along that matches)
        if (0 == one->Hc1count()) {
            forgetModule();
            return nullptr;
        }

        Hc1Module* partner = nullptr;

        // bind by retained id
        if (module_id != -1)
        {
            partner = one->getHc1(module_id);
            if (partner) { 
                return partner;
            } else {
                forgetModule();
            }
        }

        if (device_name.empty()) {
            // if only one HC-1, grab it
            partner = one->getSoleHc1();
            if (!partner) {
                // otherwise grab the first 
                one->scan_while([&](Hc1Module* const& mod)->bool {
                    if (!mod) return true;
                    this->module_id = mod->getId();
                    partner = mod;
                    return false;
                });
            }
        } else {
            // bind by device name
            one->scan_while([&](Hc1Module* const& mod)->bool {
                if (mod && mod->deviceName() == this->device_name) {
                    this->module_id = mod->getId();
                    partner = mod;
                    return false;
                }
                return true;
            });
        }
        if (partner) {
            device_name = partner->deviceName();
        }
        return partner;
    }
};

}
#endif