#include "HcOne.hpp"

namespace pachde {
HcOne* hcOne = nullptr;

struct HcOne::Internal
{
    std::vector<Hc1Module*> hc1s;
};

HcOne* HcOne::get()
{
    if (!hcOne) {
        hcOne = new HcOne;
    }
    return hcOne;
}

HcOne::HcOne()
: my(new Internal)
{
}

int HcOne::Hc1count() {
    return static_cast<int>(my->hc1s.size());
}

Hc1Module*  HcOne::getSoleHc1() {
    if (my->hc1s.empty()) return nullptr;
    if (1 == my->hc1s.size()) return *my->hc1s.cbegin();
    return nullptr;
}

Hc1Module* HcOne::getHc1(std::function<bool (Hc1Module* const&)> pred)
{
    auto item = std::find_if(my->hc1s.cbegin(), my->hc1s.cend(), pred);
    return item == my->hc1s.cend() ? nullptr : *item;
}

void HcOne::scan_while(std::function<bool(Hc1Module* const&)> pred)
{
    for (auto m: my->hc1s) {
        if (!pred(m)) break;
    }
}

Hc1Module* HcOne::getHc1(int64_t id)
{
    auto item = std::find_if(my->hc1s.cbegin(), my->hc1s.cend(), [=](Hc1Module* const& m){ return m->getId() == id; });
    return item == my->hc1s.cend() ? nullptr : *item;
}

void HcOne::registerHc1(Hc1Module* module)
{
    if (my->hc1s.cend() == std::find(my->hc1s.cbegin(), my->hc1s.cend(), module)) {
        my->hc1s.push_back(module);
    }
}

void HcOne::unregisterHc1(Hc1Module* module)
{
    auto item = std::find(my->hc1s.cbegin(), my->hc1s.cend(), module);
    if (item != my->hc1s.cend())
    {
        my->hc1s.erase(item);
    }
}

}