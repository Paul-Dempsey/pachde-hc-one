#include "PolyMidi.hpp"
#include "../widgets/port.hpp"
#include "mpe_burger.hpp"

namespace pachde {

using PMP = PolyMidiModule::Params;
using PMI = PolyMidiModule::Inputs;
using PML = PolyMidiModule::Lights;
constexpr const float CENTER = 52.5f;
constexpr const float R_LEFT = 36.f;
constexpr const float R_RIGHT = 70.f;
constexpr const float L_KNOB = 26.f;
constexpr const float R_KNOB = 78.f;

std::string MakePolyphonyLabel(const Polyphony& polyphony, const NotePriority& priority)
{
    auto text = format_string("%d", polyphony.polyphony());
    if (polyphony.expanded_polyphony()) {
        text.push_back('+');
    }
    if (priority.increased_computation()) {
        text.push_back('^');
    }
    return text;
}

PolyMidiModuleWidget::PolyMidiModuleWidget(PolyMidiModule * module)
: my_module(module)
{
    setModule(module);
    if (module) {
        my_module->ui_event_sink = this;
    }
    const NVGcolor gold = GetStockColor(StockColor::Gold);

    setPanel(createPanel(asset::plugin(pluginInstance, "res/PolyMidi.svg")));
    addChild(partner_picker = createPartnerPicker());
    partner_picker->setFormat(TextFormatLength::Short);

    addChild(createParamCentered<SmallBlackKnob>(Vec(L_KNOB, 58.f), module, PMP::P_POLY));
    addChild(poly_text = createLazyDynamicTextLabel(
        Vec(L_KNOB, 70.f), Vec(100.f, 12.f),
        [&](){ return my_module ? MakePolyphonyLabel(my_module->polyphony, my_module->priority) : "4+"; },
        9.f, false, TextAlignment::Center, gold, false));

    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<BlueLight>>>(
        Vec(CENTER, 52.f), module, PMP::P_EXPAND, PML::L_EXPAND));
    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<BlueLight>>>(
        Vec(CENTER, 68.f), module, PMP::P_COMPUTE, PML::L_COMPUTE));

    auto tw = createStaticTextLabel(Vec(L_KNOB, 134.f), 80.f, "MPE+", TextAlignment::Center, 9.f, false, gold);
    midi_ui = createWidgetCentered<MpeBurger>(Vec(L_KNOB, 120));
    midi_ui->setLabel(tw);
    if (module) midi_ui->setParam(module->getParamQuantity(PMP::P_MPE));
    addChild(midi_ui);
    addChild(tw);

    addChild(createParamCentered<SmallBlackKnob>(Vec(L_KNOB, 180.f), module, PMP::P_PRI));
    addChild(priority_text = createLazyDynamicTextLabel(
        Vec(L_KNOB, 194.f), Vec(100.f, 12.f),
        [&](){ return my_module ? NotePriorityName(my_module->priority.priority()) : NotePriorityName(NotePriorityType::LRU); },
        9.f, false, TextAlignment::Center, gold, false));

    addChild(createParamCentered<SmallBlackKnob>(Vec(L_KNOB, 240.f), module, PMP::P_VELOCITY));

    addChild(createParamCentered<SmallBlackKnob>(Vec(R_KNOB, 120.f), module, PMP::P_X_BEND));
    addChild(bend_text = createLazyDynamicTextLabel(
        Vec(R_KNOB, 134.f), Vec(100.f, 12.f),
        [&](){ return BendDisplayValue(my_module ? my_module->mpe.get_bend() : 96, true); },
        9.f, false, TextAlignment::Center, gold, false));
    addChild(createParamCentered<SmallBlackKnob>(Vec(R_KNOB, 180.f), module, PMP::P_Y));
    addChild(createParamCentered<SmallBlackKnob>(Vec(R_KNOB, 240.f), module, PMP::P_Z));

    createRouting();
}

void PolyMidiModuleWidget::createRouting()
{
    float y = 308.25f;
    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<BlueLight>>>(
        Vec(R_LEFT, y),
        module, PMP::P_ROUTE_SURFACE_MIDI, PML::L_ROUTE_SURFACE_MIDI));
    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<BlueLight>>>(
        Vec(CENTER, y),
        module, PMP::P_ROUTE_SURFACE_DSP, PML::L_ROUTE_SURFACE_DSP));
    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<BlueLight>>>(
        Vec(R_RIGHT, y),
        module, PMP::P_ROUTE_SURFACE_CVC, PML::L_ROUTE_SURFACE_CVC));

    y = 342.f;
    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<BlueLight>>>(
        Vec(R_LEFT, y),
        module, PMP::P_ROUTE_MIDI_MIDI, PML::L_ROUTE_MIDI_MIDI));
    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<BlueLight>>>(
        Vec(CENTER, y),
        module, PMP::P_ROUTE_MIDI_DSP, PML::L_ROUTE_MIDI_DSP));
    addParam(createLightParamCentered<PDLightLatch<TinySimpleLight<BlueLight>>>(
        Vec(R_RIGHT, y),
        module, PMP::P_ROUTE_MIDI_CVC, PML::L_ROUTE_MIDI_CVC));
}

Hc1Module* PolyMidiModuleWidget::getPartner()
{
    return module ? my_module->getPartner() : nullptr;
}

void PolyMidiModuleWidget::onPolyphonyChanged(const PolyphonyChangedEvent &e)
{
    poly_text->modified();
}

void PolyMidiModuleWidget::onNotePriorityChanged(const NotePriorityChangedEvent& e)
{
    poly_text->modified();
    priority_text->modified();
}

void PolyMidiModuleWidget::onMpeChanged(const MpeChangedEvent &e)
{
    midi_ui->setMode(e.mpe.mode());
    bend_text->modified();
}

void PolyMidiModuleWidget::onDeviceChanged(const DeviceChangedEvent &e)
{
    partner_picker->onDeviceChanged(e);
}

void PolyMidiModuleWidget::onDisconnect(const DisconnectEvent& e)
{
    partner_picker->onDisconnect(e);
}

void PolyMidiModuleWidget::appendContextMenu(Menu *menu)
{
    if (!my_module) return;
    menu->addChild(new MenuSeparator);
    my_module->partner_binding.appendContextMenu(menu);
}

}