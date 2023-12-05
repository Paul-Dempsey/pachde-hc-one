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

struct PartnerBinding
{
    int64_t partner_id;
    std::string claim;
    bool subscribed;
    Module* client;

    PartnerBinding() : partner_id(-1), subscribed(false), client(nullptr) {}

    bool hasClient() { return static_cast<bool>(client); }
    bool hasDevice() { return !claim.empty(); }

    void setClient(Module* the_module)
    {
        client = the_module;
        assert(hasClient());
    }

    static bool iCanBindToExclusive(Hc1Module* hc1, Module* me)
    {
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

    void setClaim(const std::string& device_claim) { claim = device_claim; }
    void setPartner(int64_t id) { partner_id = id; }
    void forgetPartner() { partner_id = -1; }
    void forgetDevice() { claim = ""; }

    void onDeviceChanged(const IHandleHcEvents::DeviceChangedEvent& e) {
        setClaim(e.device ? e.device->info.spec() : "");
    }

    void onDisconnect(const IHandleHcEvents::DisconnectEvent& e) {
        subscribed = false;
        forgetPartner();
    }

    void ensureSubscribed(Hc1Module* partner) 
    {
        if (!subscribed && partner && client) {
            auto handler = dynamic_cast<IHandleHcEvents*>(client);
            if (handler) {
                partner->subscribeHcEvents(handler);
                subscribed = true;
            }
        }
    }

    void unsubscribe()
    {
        if (!subscribed) return;

        auto handler = dynamic_cast<IHandleHcEvents*>(client);
        if (handler) {
            auto partner = getPartner(false);
            if (partner) {
                partner->unsubscribeHcEvents(handler);
                subscribed = false;
            }
        }
    }

    Hc1Module* getPartner(bool subscribe = true)
    {
        auto one = ModuleBroker::get();

        // If no HC-1, forget any we remember and give up 
        // but still remember the device claim, if one comes along that matches
        if (0 == one->Hc1count()) {
            subscribed = false;
            forgetPartner();
            return nullptr;
        }

        Hc1Module* partner = nullptr;

        // bind by retained id
        if (partner_id != -1)
        {
            partner = one->getHc1(partner_id);
            if (partner) { 
                claim = partner->getMidiDeviceClaim();
                ensureSubscribed(partner);
            } else {
                subscribed = false;
                forgetPartner();
            }
            return partner;
        }

        if (claim.empty()) {
            // if only one HC-1, grab it
            partner = one->getSoleHc1();
            if (!partner) {
                // otherwise grab the first 
                one->scan_while([this, &partner](Hc1Module* mod)->bool {
                    if (!mod || !iCanBindToExclusive(mod, client)) return true;
                    partner_id = mod->getId();
                    partner = mod;
                    return false;
                });
            }
        } else {
            // bind by device claim
            one->scan_while([this, &partner](Hc1Module* const& mod)->bool {
                if (mod
                    && iCanBindToExclusive(mod, client)
                    && mod->connection
                    && mod->connection->info.spec() == claim) 
                {
                    partner_id = mod->getId();
                    partner = mod;
                    return false;
                }
                return true;
            });
        }

        if (partner) {
            claim = partner->getMidiDeviceClaim();
            ensureSubscribed(partner);
        }
        return partner;
    }

    void appendContextMenu(Menu *menu) {
        menu->addChild(createMenuItem(
            "Reset partner module", "",
            [&](){
                forgetDevice();
                unsubscribe();
                forgetPartner();
            },
            !(hasClient() || hasDevice())
        ));
    }
};

}
#endif