#pragma once
#ifndef MODULE_BROKER_HPP_INCLUDED
#define MODULE_BROKER_HPP_INCLUDED
#include "./HC-1/HC-1.hpp"
#include "hc_events.hpp"
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

struct PartnerBinding
{
    int64_t module_id;
    std::string claim;

    PartnerBinding() : module_id(-1) {}

    void setDevice(std::string device_claim) { claim = device_claim; }
    void forgetModule() { module_id = -1; }
    void forgetDevice() { claim = ""; }
    void onDeviceChanged(const IHandleHcEvents::DeviceChangedEvent& e) {
        setDevice(e.device ? e.device->info.spec() : "");
    }

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

        if (claim.empty()) {
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
            // bind by device claim
            one->scan_while([&](Hc1Module* const& mod)->bool {
                if (mod 
                    && mod->connection
                    && mod->connection->info.spec() == claim) 
                {
                    module_id = mod->getId();
                    partner = mod;
                    return false;
                }
                return true;
            });
        }
        if (partner && partner->connection) {
            claim = partner->connection->info.spec();
        }
        return partner;
    }
};

}
#endif