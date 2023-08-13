#include "HC-2.hpp"
#include "../text.hpp"
#include "../colors.hpp"
#include "../misc.hpp"

namespace pachde {

Hc2Module::Hc2Module()
{
}

std::string Hc2Module::getDeviceName()
{
    return partner ? FilterDeviceName(partner->device_name) : "<Eagan Matrix Device>";
}

void Hc2Module::onExpanderChange(const ExpanderChangeEvent& e)
{
    auto ex = e.side ? getRightExpander() : getLeftExpander();
    if (!partner) {
        if (ex.module) {
            partner = dynamic_cast<Hc1Module*>(ex.module);
            partner_side = e.side;
            refreshPartner();
        }
    } else {
        if (e.side == partner_side) {
            partner = dynamic_cast<Hc1Module*>(ex.module);
            if (!partner) {
                // get partner from opposite side (if any)
                ex = (!e.side) ? getRightExpander() : getLeftExpander();
                partner = dynamic_cast<Hc1Module*>(ex.module);
                partner_side = !e.side;
            }
            refreshPartner();
        }
    }
}

void Hc2Module::refreshPartner()
{
    if (partner) {

    } else {

    }
}

// void Hc2Module::process(const ProcessArgs& args)
// {
// }

}

Model *modelHc2 = createModel<pachde::Hc2Module, pachde::Hc2ModuleWidget>("pachde-hc-2");