#include "PolyMidi.hpp"
#include "../widgets/enum_param.hpp"
namespace pachde {


PolyMidiModule::PolyMidiModule()
{
    config(Params::NUM_PARAMS, Inputs::NUM_INPUTS, Outputs::NUM_OUTPUTS, Lights::NUM_LIGHTS);
    partner_binding.setClient(this);
    std::vector<std::string> connection = {"disconnected", "connected"};
    std::vector<std::string> offon = {"off", "on"};

    configParam(Params::P_POLY, 1.f, 16.f, 1.f, "Base Polyphony")->snapEnabled = true;
    configSwitch(Params::P_EXPAND, 0.f, 1.f, 1.f, "Allow expanded polyphony", offon);
    configSwitch(Params::P_COMPUTE, 0.f, 1.f, 0.f, "Allow increased computation rate", offon);

    configSwitch(Params::P_MPE, U8(MidiMode::Midi), U8(MidiMode::MpePlus), U8(MidiMode::MpePlus), "MIDI/MPE", {"MIDI", "MPE", "MPE+"});
    configBendParam(this, Params::P_X_BEND);

    //configSwitch(Params::P_Y, 0.f, 8.f, 7.f, "Y", { "Off", "cc1 Modulation", "cc2 Breath", "cc3", "cc4 Foot", "cc7 Volume", "cc11 Expression", "cc74 Brightness", "cc74 (no shelf)"});
    configEnumParam<EMY>(Params::P_Y, "Y",this, EMY::Default_Y,
        { EMY::None, EMY::CC_1_Modulation, EMY::CC_2_Breath, EMY::CC_3, EMY::CC_4_Foot, EMY::CC_7_Volume, EMY::CC_11_Expression, EMY::CC_74_Bright, EMY::CC_74_NoShelf },
        { "Off",     "cc1 Modulation",     "cc2 Breath",     "cc3",     "cc4 Foot",     "cc7 Volume",     "cc11 Expression",     "cc74 Brightness", "cc74 (no shelf)" }
        );

    //configSwitch(Params::P_Z, 0.f, 7.f, 7.f, "Z",  { "Off", "cc1 Modulation", "cc2 Breath", "cc3", "cc4 Foot", "cc7 Volume", "cc11 Expression", "Channel pressure"});
    configEnumParam<EMZ>(Params::P_Z, "Z",this, EMZ::Midi_ChannelPressure,
        { EMZ::None, EMZ::CC_1_Modulation, EMZ::CC_2_Breath, EMZ::CC_3, EMZ::CC_4_Foot, EMZ::CC_7_Volume, EMZ::CC_11_Expression, EMZ::Midi_ChannelPressure, },
        { "Off",     "cc1 Modulation",     "cc2 Breath",     "cc3",     "cc4 Foot",     "cc7 Volume",     "cc11 Expression",     "Channel pressure" },
        true
        );

    configSwitch(Params::P_PRI, 0.f, 6.f, 0.f, "Note priority", { "Oldest", "Same note", "Lowest", "Highest", "High 2", "High 3", "High 4"});
    configSwitch(Params::P_VELOCITY, 0.f, 5.f, 0.f, "Velocity", {"Static 127", "Dynamic", "Formula V", "No Notes", "Theremin", "Kyma"});

    configSwitch(Params::P_ROUTE_SURFACE_MIDI,0.f, 1.f, 1.f, "Surface to MIDI", connection);
    configSwitch(Params::P_ROUTE_SURFACE_DSP,0.f, 1.f, 1.f, "Surface to DSP", connection);
    configSwitch(Params::P_ROUTE_SURFACE_CVC,0.f, 1.f, 1.f, "Surface to CVC", connection);
    configSwitch(Params::P_ROUTE_MIDI_MIDI,0.f, 1.f, 1.f, "Midi to MIDI", connection);
    configSwitch(Params::P_ROUTE_MIDI_DSP,0.f, 1.f, 1.f, "Midi to DSP", connection);
    configSwitch(Params::P_ROUTE_MIDI_CVC,0.f, 1.f, 1.f, "Midi to CVC", connection);
    setRoutingLights();
}

PolyMidiModule::~PolyMidiModule()
{
    partner_binding.unsubscribe();
}

json_t * PolyMidiModule::dataToJson()
{
    auto root = json_object();
    json_object_set_new(root, "device", json_string(partner_binding.claim.c_str()));
    return root;
}

void PolyMidiModule::dataFromJson(json_t *root)
{
    auto j = json_object_get(root, "device");
    if (j) {
        partner_binding.setClaim(json_string_value(j));
    }
    getPartner();
}

Hc1Module* PolyMidiModule::getPartner()
{
    return partner_binding.getPartner();
}

void PolyMidiModule::onPolyphonyChanged(const PolyphonyChangedEvent& e)
{
    if (polyphony.to_raw() == e.polyphony.to_raw()) return;
    polyphony = e.polyphony;
    getParamQuantity(Params::P_POLY)->setValue(polyphony.polyphony());
    getParamQuantity(Params::P_EXPAND)->setValue(polyphony.expanded_polyphony());

    if (ui_event_sink) {
        ui_event_sink->onPolyphonyChanged(e);
    }
}

void PolyMidiModule::onNotePriorityChanged(const NotePriorityChangedEvent &e)
{
    if (priority.to_raw() == e.piority.to_raw()) return;
    priority = e.piority;
    getParamQuantity(Params::P_PRI)->setValue(static_cast<float>(U8(priority.priority())));
    getParamQuantity(Params::P_COMPUTE)->setValue(priority.increased_computation());

    if (ui_event_sink) {
        ui_event_sink->onNotePriorityChanged(e);
    }
}

void PolyMidiModule::onMpeChanged(const MpeChangedEvent& e)
{
    getParamQuantity(P_MPE)->setValue(U8(e.mpe.mode()));
    bool mpe = e.mpe.is_any_mpe();
    auto pq = static_cast<BendParamQuantity*>(getParamQuantity(Params::P_X_BEND));
    pq->is_mpe = mpe;
    pq->setValue(e.mpe.get_bend());
    static_cast<EnumQuantity<EMY>*>(getParamQuantity(Params::P_Y))->setEnumValue(e.mpe.get_y());

    EMZ z_param = mpe ? EMZ::Midi_ChannelPressure : e.mpe.get_z();
    static_cast<EnumQuantity<EMZ>*>(getParamQuantity(Params::P_Z))->setEnumValue(z_param);
    if (ui_event_sink) {
        ui_event_sink->onMpeChanged(e);
    }
}

void PolyMidiModule::onRoutingChanged(const RoutingChangedEvent &e)
{
    if (routing == e.routing) return;
    routing = e.routing;
    getParamQuantity(Params::P_ROUTE_SURFACE_MIDI)->setValue(routing & EM_ROUTE_BITS::Surface_Midi);
    getParamQuantity(Params::P_ROUTE_SURFACE_DSP)->setValue(routing & EM_ROUTE_BITS::Surface_Dsp);
    getParamQuantity(Params::P_ROUTE_SURFACE_CVC)->setValue(routing & EM_ROUTE_BITS::Surface_Cvc);
    getParamQuantity(Params::P_ROUTE_MIDI_MIDI)->setValue(routing & EM_ROUTE_BITS::Midi_Midi);
    getParamQuantity(Params::P_ROUTE_MIDI_DSP)->setValue(routing & EM_ROUTE_BITS::Midi_Dsp);
    getParamQuantity(Params::P_ROUTE_MIDI_CVC)->setValue(routing & EM_ROUTE_BITS::Midi_Cvc);

    setRoutingLights();
    // not needed, but will be if the routing graphic is enhanced beyound just lights
    // if (ui_event_sink) {
    //     ui_event_sink->onRoutingChanged(e);
    // }
}

void PolyMidiModule::onVelocityChanged(const VelocityChangedEvent& e)
{
    if (velocity == e.velocity) return;
    velocity = e.velocity;
    getParamQuantity(Params::P_VELOCITY)->setValue(e.velocity.getVelocityByte());
    // if (ui_event_sink) {
    //     ui_event_sink->onVelocityChanged(e);
    // }
}

void PolyMidiModule::onDeviceChanged(const DeviceChangedEvent &e)
{
    partner_binding.onDeviceChanged(e);
    if (ui_event_sink) {
        ui_event_sink->onDeviceChanged(e);
    }
}

void PolyMidiModule::onDisconnect(const DisconnectEvent& e)
{
    partner_binding.onDisconnect(e);
    if (ui_event_sink) {
        ui_event_sink->onDisconnect(e);
    }
}

uint8_t PolyMidiModule::getKnobRouting()
{
    uint8_t result = 0;
    if (getParamQuantity(Params::P_ROUTE_SURFACE_MIDI)->getValue() > 0.5f) { result |= EM_ROUTE_BITS::Surface_Midi; }
    if (getParamQuantity(Params::P_ROUTE_SURFACE_DSP)->getValue() > 0.5f) { result |= EM_ROUTE_BITS::Surface_Dsp; }
    if (getParamQuantity(Params::P_ROUTE_SURFACE_CVC)->getValue() > 0.5f) { result |= EM_ROUTE_BITS::Surface_Cvc; }
    if (getParamQuantity(Params::P_ROUTE_MIDI_MIDI)->getValue() > 0.5f) { result |= EM_ROUTE_BITS::Midi_Midi; }
    if (getParamQuantity(Params::P_ROUTE_MIDI_DSP)->getValue() > 0.5f) { result |= EM_ROUTE_BITS::Midi_Dsp; }
    if (getParamQuantity(Params::P_ROUTE_MIDI_CVC)->getValue() > 0.5f) { result |= EM_ROUTE_BITS::Midi_Cvc; }
    return result;
}

void PolyMidiModule::setRoutingLights()
{
    getLight(Lights::L_ROUTE_SURFACE_MIDI).setBrightness(routing & EM_ROUTE_BITS::Surface_Midi);
    getLight(Lights::L_ROUTE_SURFACE_DSP).setBrightness(routing & EM_ROUTE_BITS::Surface_Dsp);
    getLight(Lights::L_ROUTE_SURFACE_CVC).setBrightness(routing & EM_ROUTE_BITS::Surface_Cvc);
    getLight(Lights::L_ROUTE_MIDI_MIDI).setBrightness(routing & EM_ROUTE_BITS::Midi_Midi);
    getLight(Lights::L_ROUTE_MIDI_DSP).setBrightness(routing & EM_ROUTE_BITS::Midi_Dsp);
    getLight(Lights::L_ROUTE_MIDI_CVC).setBrightness(routing & EM_ROUTE_BITS::Midi_Cvc);
}

Hc1Module* PolyMidiModule::processPolyphony(Hc1Module* partner)
{
    bool poly_changed = false;
    bool ui_expanded = getParamQuantity(Params::P_EXPAND)->getValue() > 0.5f;
    if (ui_expanded != polyphony.expanded_polyphony()) {
        poly_changed = true;
        polyphony.set_expanded_polyphony(ui_expanded);
    }
    uint8_t ui_poly = std::round(getParamQuantity(Params::P_POLY)->getValue());
    if (ui_poly != polyphony.polyphony()) {
        poly_changed = true;
        polyphony.set_polyphony(ui_poly);
    }
    if (poly_changed) {
        if (ui_event_sink) {
            ui_event_sink->onPolyphonyChanged(PolyphonyChangedEvent{polyphony});
        }
        auto partner = getPartner();
        if (partner) {
            partner->em.polyphony.raw = polyphony.raw;
            if (partner->readyToSend()) {
                partner->sendControlChange(EM_SettingsChannel, EMCC_Polyphony, polyphony.raw);
            }
        }
    }
    getLight(Lights::L_EXPAND).setBrightness(polyphony.expanded_polyphony());
    return partner;
}

Hc1Module* PolyMidiModule::processPriority(Hc1Module* partner)
{
    bool pri_changed = false;
    NotePriorityType pri = static_cast<NotePriorityType>(U8(std::round(getParamQuantity(Params::P_PRI)->getValue())));
    if (pri != priority.priority()) {
        pri_changed = true;
        priority.set_priority(pri);
    }
    bool compute = getParamQuantity(Params::P_COMPUTE)->getValue() > 0.5f;
    if (compute != priority.increased_computation()) {
        pri_changed = true;
        priority.set_increased_computation(compute);
    }
    if (pri_changed) {
        if (ui_event_sink) {
            ui_event_sink->onNotePriorityChanged(NotePriorityChangedEvent{priority});
        }
        if (!partner) partner = getPartner();
        if (partner) {
            partner->em.priority.raw = priority.raw;
            if (partner->readyToSend()) {
                partner->sendControlChange(EM_SettingsChannel, EMCC_Priority, priority.to_raw());
            }
        }
    }
    getLight(Lights::L_COMPUTE).setBrightness(priority.increased_computation());
    return partner;
}

Hc1Module* PolyMidiModule::processRouting(Hc1Module *partner)
{
    auto knob_route = getKnobRouting();
    if (routing != knob_route) {
        routing = knob_route;
        if (!partner) partner = getPartner();
        if (partner) {
            partner->em.routing = routing;
            if (partner->readyToSend()) {
                partner->sendControlChange(EM_SettingsChannel, EMCC_Routing, routing);
            }
        }
    }
    setRoutingLights();
    return partner;
}

Hc1Module* PolyMidiModule::processMpe(Hc1Module* partner)
{
    auto mode = static_cast<MidiMode>(GetByteParamValue(getParamQuantity(P_MPE)));
    if (mode != mpe.mode()) {
        auto old = mpe;
        mpe.set_mode(mode);

        auto bpq = static_cast<BendParamQuantity*>(getParamQuantity(P_X_BEND));
        bpq->is_mpe = mpe.is_any_mpe();
        bpq->setValue(mpe.get_bend());

        static_cast<EnumQuantity<EMY>*>(getParamQuantity(P_Y))->setEnumValue(mpe.get_y());
        static_cast<EnumQuantity<EMZ>*>(getParamQuantity(P_Z))->setEnumValue(mpe.get_z());

        if (!partner) partner = getPartner();
        if (partner) {
            partner->em.mpe = mpe;
            if (partner->readyToSend()) {
                if (old.get_bend() != mpe.get_bend()) {
                    partner->sendControlChange(EM_SettingsChannel, EMCC_BendRange, mpe.get_bend());
                }
                if (old.get_y() != mpe.get_y()) {
                    partner->sendControlChange(EM_SettingsChannel, EMCC_YCC, U8(mpe.get_y()));
                }
                if (old.get_z() != mpe.get_z()) {
                    partner->sendControlChange(EM_SettingsChannel, EMCC_ZCC, U8(mpe.get_z()));
                }
            }
        }
    }
    return partner;
}

Hc1Module* PolyMidiModule::processXBend(Hc1Module* partner)
{
    uint8_t knob_bend = static_cast<BendParamQuantity*>(getParamQuantity(P_X_BEND))->getBendValue();
    if (knob_bend != mpe.get_bend()) {
        mpe.set_bend_checked(knob_bend);
        if (!partner) partner = getPartner();
        if (partner) {
            partner->em.mpe = mpe;
            if (partner->readyToSend()) {
                partner->sendControlChange(EM_SettingsChannel, EMCC_BendRange, mpe.get_bend());
            }
        }
    }
    return partner;
}

Hc1Module* PolyMidiModule::processY(Hc1Module* partner)
{
    auto knob_y = static_cast<EnumQuantity<EMY>*>(getParamQuantity(P_Y))->getItemValue();
    if (knob_y != mpe.get_y()) {
        mpe.set_y_checked(knob_y);
        if (!partner) partner = getPartner();
        if (partner) {
            partner->em.mpe = mpe;
            if (partner->readyToSend()) {
                partner->sendControlChange(EM_SettingsChannel, EMCC_YCC, U8(mpe.get_y()));
            }
        }
    }
    return partner;
}

Hc1Module* PolyMidiModule::processZ(Hc1Module* partner)
{
    auto knob_z = static_cast<EnumQuantity<EMZ>*>(getParamQuantity(P_Z))->getItemValue();
    if (knob_z == EMZ::Midi_ChannelPressure) {
        // knob clips to MidiChannelPressure, so we must 
        // translate to the unclipped value per em mode
        switch (mpe.mode()) {
            case MidiMode::Midi: break;
            case MidiMode::Mpe: knob_z = EMZ::Mpe_ChannelPressure; break;
            case MidiMode::MpePlus: knob_z = EMZ::MpePlus_ChannelPressure; break;
        }
    }
    if (knob_z != mpe.get_z()) {
        mpe.set_z_checked(knob_z);
        assert(mpe.get_z() == knob_z); // may need to handle side effects
        if (!partner) partner = getPartner();
        if (partner) {
            partner->em.mpe = mpe;
            if (partner->readyToSend()) {
                partner->sendControlChange(EM_SettingsChannel, EMCC_ZCC, U8(mpe.get_z()));
            }
        }
    }
    return partner;
}

Hc1Module* PolyMidiModule::processVelocity(Hc1Module* partner)
{
    auto knob_velocity = static_cast<EM_Velocity>(GetByteParamValue(getParamQuantity(P_VELOCITY)));
    if (knob_velocity != velocity.getVelocity()) {
        velocity.setVelocity(knob_velocity);
        if (!partner) partner = getPartner();
        if (partner) {
            partner->em.velocity = velocity;
            if (partner->readyToSend()) {
                partner->sendControlChange(EM_SettingsChannel, EMCC_VelSplit, velocity.vs);
            }
        }
    }
    return partner;
}

void PolyMidiModule::process(const ProcessArgs& args)
{
    Hc1Module* partner = nullptr;
    if (0 == ((args.frame + id) % CV_INTERVAL)) {
        partner = processPolyphony(partner);
        partner = processPriority(partner);
        partner = processMpe(partner);
        partner = processXBend(partner);
        partner = processY(partner);
        partner = processZ(partner);
        partner = processVelocity(partner);
        partner = processRouting(partner);
    }
}

}

Model *modelPolyMidi = createModel<pachde::PolyMidiModule, pachde::PolyMidiModuleWidget>("pachde-hc-polymidi");