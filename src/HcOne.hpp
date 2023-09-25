#pragma once
#ifndef HCONE_HPP_INCLUDED
#define HCONE_HPP_INCLUDED
#include "./HC-1/HC-1.hpp"
// #include "./HC-2/HC-2.hpp"
// #include "./HC-3/HC-3.hpp"

namespace pachde {

struct HcOne
{
    struct Internal;
    Internal * my;

    static HcOne* get();
    void registerHc1(Hc1Module * module);
    void unregisterHc1(Hc1Module * module);
    int Hc1count();
    Hc1Module* getSoleHc1();
    Hc1Module* getHc1(std::function<bool(Hc1Module* const&)> pred);
    Hc1Module* getHc1(int64_t id);

    // pred returns false to stop scan
    void scan(std::function<bool(Hc1Module* const&)> pred);

private:
    HcOne();
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
        auto one = HcOne::get();

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
                // otherwise grab the first one
                one->scan([&](Hc1Module* const& mod)->bool {
                    if (mod) {
                        this->module_id = mod->getId();
                        partner = mod;
                        return false;
                    }
                    return true;
                });
            }
        } else {
            // bind by device name
            one->scan([&](Hc1Module* const& mod)->bool {
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