#pragma once
#ifndef TUNING_HAMURGER_HPP_INCLUDED
#define TUNING_HAMURGER_HPP_INCLUDED
#include "HC-2.hpp"
#include "../hamburger.hpp"
namespace pachde {

struct TuningMenu : Hamburger
{
    Hc2Module * my_module = nullptr;
    Tuning tuning = Tuning::EqualTuning;

    void setModule(Hc2Module *module) {
        my_module = module;
        if (my_module) {
            tuning = my_module->rounding.tuning;
        }
        describe(format_string("Tuning: %s", describeTuning(tuning).c_str()).c_str());
    }

    void step() override {
        Hamburger::step();
        if (my_module && my_module->rounding.tuning != tuning) {
            tuning = my_module->rounding.tuning;
            describe(format_string("Tuning: %s", describeTuning(tuning).c_str()).c_str());
        }
    }

    MenuItem * createTuningMenuItem(Tuning tuningItem)
    {
        assert(my_module);
        return createCheckMenuItem(describeTuning(tuningItem), "", 
            [=](){ return my_module->rounding.tuning == tuningItem; },
            [=](){ my_module->rounding.tuning = tuningItem; });
    }

    void appendContextMenu(Menu * menu) override
    {
        if (!my_module) return;
        menu->addChild(createMenuLabel(describeTuning(tuning));
        menu->addChild(new MenuSeparator);
        menu->addChild(createTuningMenuItem(Tuning::EqualTuning));
        menu->addChild(createSubmenuItem("n-Tone Equal", "", [=](Menu * menu) {
            menu->addChild(createTuningMenuItem(Tuning::OneTone));
            menu->addChild(createTuningMenuItem(static_cast<Tuning>(2)));
            menu->addChild(createTuningMenuItem(static_cast<Tuning>(3)));
            menu->addChild(createTuningMenuItem(static_cast<Tuning>(4)));
            menu->addChild(createTuningMenuItem(static_cast<Tuning>(5)));
            menu->addChild(createTuningMenuItem(static_cast<Tuning>(6)));
            menu->addChild(createTuningMenuItem(static_cast<Tuning>(7)));
            menu->addChild(createTuningMenuItem(static_cast<Tuning>(8)));
            menu->addChild(createTuningMenuItem(static_cast<Tuning>(9)));
            menu->addChild(createTuningMenuItem(static_cast<Tuning>(10)));
            menu->addChild(createTuningMenuItem(static_cast<Tuning>(11)));
            menu->addChild(createTuningMenuItem(static_cast<Tuning>(17)));
            menu->addChild(createTuningMenuItem(static_cast<Tuning>(19)));
            menu->addChild(createTuningMenuItem(static_cast<Tuning>(22)));
            menu->addChild(createTuningMenuItem(static_cast<Tuning>(24)));
            menu->addChild(createTuningMenuItem(static_cast<Tuning>(26)));
            menu->addChild(createTuningMenuItem(static_cast<Tuning>(31)));
            menu->addChild(createTuningMenuItem(static_cast<Tuning>(43)));
            menu->addChild(createTuningMenuItem(Tuning::FiftyTone));
        }));
        menu->addChild(createSubmenuItem("Just", "", [=](Menu * menu) {
            menu->addChild(createTuningMenuItem(Tuning::JustC));
            menu->addChild(createTuningMenuItem(Tuning::JustCs));
            menu->addChild(createTuningMenuItem(Tuning::JustD));
            menu->addChild(createTuningMenuItem(Tuning::JustEb));
            menu->addChild(createTuningMenuItem(Tuning::JustF));
            menu->addChild(createTuningMenuItem(Tuning::JustFs));
            menu->addChild(createTuningMenuItem(Tuning::JustG));
            menu->addChild(createTuningMenuItem(Tuning::JustAb));
            menu->addChild(createTuningMenuItem(Tuning::JustA));
            menu->addChild(createTuningMenuItem(Tuning::JustBb));
            menu->addChild(createTuningMenuItem(Tuning::JustB));
        }));
        menu->addChild(createSubmenuItem("User-defined", "", [=](Menu * menu) {
            menu->addChild(createTuningMenuItem(Tuning::UserTuning1));
            menu->addChild(createTuningMenuItem(static_cast<Tuning>(Tuning::UserTuning1 + 1)));
            menu->addChild(createTuningMenuItem(static_cast<Tuning>(Tuning::UserTuning1 + 2)));
            menu->addChild(createTuningMenuItem(static_cast<Tuning>(Tuning::UserTuning1 + 3)));
            menu->addChild(createTuningMenuItem(static_cast<Tuning>(Tuning::UserTuning1 + 4)));
            menu->addChild(createTuningMenuItem(static_cast<Tuning>(Tuning::UserTuning1 + 5)));
            menu->addChild(createTuningMenuItem(static_cast<Tuning>(Tuning::UserTuning1 + 6)));
            menu->addChild(createTuningMenuItem(static_cast<Tuning>(Tuning::UserTuning1 + 7)));
        }));
    }
};

}
#endif