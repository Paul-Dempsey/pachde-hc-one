#include "HC-2.hpp"
#include "../text.hpp"
#include "../colors.hpp"
#include "../misc.hpp"

namespace pachde {

Hc2Module::Hc2Module()
{
}


Hc1Module* Hc2Module::getPartner()
{
    if (partner_side.left()) {
        return dynamic_cast<Hc1Module*>(getLeftExpander().module);
    }
    if (partner_side.right()) {
        return dynamic_cast<Hc1Module*>(getRightExpander().module);
    }
    return nullptr;
}

const char * Hc2Module::getDeviceName()
{
    if (auto partner = getPartner()) {
        return partner->device_name.c_str();
    }
    return "<Eagan Matrix Device>";
}

void Hc2Module::onExpanderChange(const ExpanderChangeEvent& e)
{
    partner_side.clear();
    auto left = dynamic_cast<Hc1Module*>(getLeftExpander().module);
    auto right = dynamic_cast<Hc1Module*>(getRightExpander().module);
    if (left) {
        partner_side.addLeft();
        left->expanderAdded(Expansion::Right);
    }
    if (right) {
        partner_side.addRight();
        right->expanderAdded(Expansion::Left);
    }
}

// void Hc2Module::process(const ProcessArgs& args)
// {
// }

}

Model *modelHc2 = createModel<pachde::Hc2Module, pachde::Hc2ModuleWidget>("pachde-hc-2");