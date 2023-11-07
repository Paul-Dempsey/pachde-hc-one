#include "module_broker.hpp"

namespace pachde {
ModuleBroker* hcOne = nullptr;

struct ModuleBroker::Internal
{
    std::vector<Hc1Module*> hc1s;
    std::mutex mut;
};

ModuleBroker* ModuleBroker::get()
{
    if (!hcOne) {
        hcOne = new ModuleBroker;
    }
    return hcOne;
}

ModuleBroker::ModuleBroker()
: my(new Internal)
{
}

ModuleBroker::~ModuleBroker()
{
    if (my) { delete my; }
}

int ModuleBroker::Hc1count() {
    return static_cast<int>(my->hc1s.size());
}

Hc1Module*  ModuleBroker::getSoleHc1() {
    return (my->hc1s.empty() || my->hc1s.size() > 1) ? nullptr : *my->hc1s.cbegin();
}

Hc1Module* ModuleBroker::getHc1(std::function<bool (Hc1Module*)> pred)
{
    auto item = std::find_if(my->hc1s.cbegin(), my->hc1s.cend(), pred);
    return item == my->hc1s.cend() ? nullptr : *item;
}

void ModuleBroker::scan_while(std::function<bool(Hc1Module*)> pred)
{
    for (auto m: my->hc1s) {
        if (!pred(m)) break;
    }
}

Hc1Module* ModuleBroker::getHc1(int64_t id)
{
    auto item = std::find_if(my->hc1s.cbegin(), my->hc1s.cend(), [=](Hc1Module* const& m){ return m->getId() == id; });
    return item == my->hc1s.cend() ? nullptr : *item;
}

Hc1Module *ModuleBroker::get_primary()
{
    if (my->hc1s.empty()) return nullptr;
    return *my->hc1s.begin();
}

void ModuleBroker::registerHc1(Hc1Module* module)
{
    std::unique_lock<std::mutex> lock(my->mut);
    if (my->hc1s.cend() == std::find(my->hc1s.cbegin(), my->hc1s.cend(), module)) {
        my->hc1s.push_back(module);
    }
}

void ModuleBroker::unregisterHc1(Hc1Module* module)
{
    std::unique_lock<std::mutex> lock(my->mut);
    auto item = std::find(my->hc1s.cbegin(), my->hc1s.cend(), module);
    if (item != my->hc1s.cend())
    {
        my->hc1s.erase(item);
    }
}

}