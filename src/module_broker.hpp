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
    Hc1Module* getHc1(std::function<bool(Hc1Module*)> pred);
    Hc1Module* getHc1(int64_t id);
    Hc1Module* get_primary();

    // pred returns false to stop scan
    void scan_while(std::function<bool(Hc1Module*)> pred);

private:
    ModuleBroker();
};

// $CONSIDER: add hc_event subscription to partner binding
struct PartnerBinding
{
    int64_t module_id;
    std::string claim;

    PartnerBinding() : module_id(-1) {}

    static bool iCanBindToExclusive(Hc1Module* hc1, Module *me) {
        if (hc1->hc_event_subscriptions.empty()) {
            return true;
        }
        for (auto sub : hc1->hc_event_subscriptions) {
            auto client = dynamic_cast<Module*>(sub);
            if (client && (client->getModel() == me->getModel())) {
                return false;
            }
        }
        return true;
    }

    bool is_bound() {
        return (module_id != -1) || (!claim.empty());
    }

    Hc1Module* bindPartner(Module *self)
    {
        ModuleBroker* broker = ModuleBroker::get();
        auto hc1 = broker->getHc1([self](Hc1Module* hc1){ return iCanBindToExclusive(hc1, self); });
        if (hc1) {
            setModule(hc1->Module::getId());
            return hc1;
        }
        return nullptr;
    }

    void setClaim(std::string device_claim) { claim = device_claim; }
    void setModule(int64_t id) { module_id = id; }
    void forgetModule() { module_id = -1; }
    void forgetDevice() { claim = ""; }
    void onDeviceChanged(const IHandleHcEvents::DeviceChangedEvent& e) {
        setClaim(e.device ? e.device->info.spec() : "");
    }

    Hc1Module* getPartner()
    {
        auto one = ModuleBroker::get();

        // If no HC-1, forget any we remember and give up (but still remember
        // the device claim, if one comes along that matches)
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
                one->scan_while([&](Hc1Module* mod)->bool {
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

template <typename SELF>
Hc1Module* getPartnerImpl(SELF * self)
{
    Hc1Module* partner = nullptr;
    if (self->partner_binding.is_bound()) {
        partner = self->partner_binding.getPartner();
    } else {
        partner = self->partner_binding.bindPartner(self);
    }
    if (partner) {
        if (!self->partner_subscribed) {
            partner->subscribeHcEvents(self);
            self->partner_subscribed = true;
        }
    }
    return partner;
}

}
#endif