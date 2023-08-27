#include "HC-2.hpp"
#include "../text.hpp"
#include "../colors.hpp"
#include "../misc.hpp"

namespace pachde {

Hc2Module::Hc2Module()
{
}

const char * Hc2Module::getDeviceName()
{
    return partner ? partner->device_name.c_str() : "<Eagan Matrix Device>";
}

void Hc2Module::onExpanderChange(const ExpanderChangeEvent& e)
{
    ExpanderPresence where = ExpanderPresence::fromRackSide(e.side);
    auto expander = where.right() ? getRightExpander() : getLeftExpander();
    bool new_partner = false;

    if (!partner) {
        partner = dynamic_cast<Hc1Module*>(expander.module);
        new_partner = true;
    } else {
        if (where == partner_side) {
            partner = dynamic_cast<Hc1Module*>(expander.module);
            if (!partner) {
                // get partner from opposite side (if any)
                where = where.right() ? Expansion::Left : Expansion::Right;
                expander = expander = where.right() ? getRightExpander() : getLeftExpander();
                partner = dynamic_cast<Hc1Module*>(expander.module);
            }
            new_partner = true;
        } else {
            //ignore: already have a partner
        }
    }
    if (new_partner) {
        if (partner) {
            partner_side = where;
            partner->expanderAdded(where.right() ? Expansion::Left : Expansion::Right);
        } else {
            partner_side = Expansion::None;
        }
    }
}

// void Hc2Module::process(const ProcessArgs& args)
// {
// }

}

Model *modelHc2 = createModel<pachde::Hc2Module, pachde::Hc2ModuleWidget>("pachde-hc-2");