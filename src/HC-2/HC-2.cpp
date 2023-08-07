#include "HC-2.hpp"
#include "../text.hpp"
#include "../colors.hpp"
#include "../misc.hpp"

namespace pachde {

Hc2Module::Hc2Module()
{

    findEM();
}

void Hc2Module::findEMOut()
{
    if (is_eagan_matrix) {
        int best_id = -1;
        int common = 0;
        for (auto id : midiOutput.getDeviceIds()) {
            auto name = midiOutput.getDeviceName(id);
            int c2 = common_prefix_length(device_name, name);
            if (c2 > common) {
                best_id = id;
                common = c2;
            }
        }
        if (best_id >= 0) {
            midiOutput.setDeviceId(best_id);
            midiOutput.setChannel(15);
        }
    }
}

void Hc2Module::findEM()
{
    for (auto id : Input::getDeviceIds()) {
        auto dev_name = Input::getDeviceName(id);
        if (is_EMDevice(dev_name)) {
            Input::setDeviceId(id);
            Input::setChannel(-1);
            device_name = dev_name;
            is_eagan_matrix = true;
            //heart_time = 0.25f;
            break;
        }
    }
    findEMOut();
}

void Hc2Module::sendNoteOn(uint8_t channel, uint8_t note, uint8_t velocity)
{
    midi::Message msg;
    SetNoteOn(msg, channel, note, velocity);
    auto save_channel = midiOutput.getChannel();
    //DebugLog("Note (%d) ch%d %d %d", save_channel, channel, note, velocity);
    midiOutput.setChannel(channel);
    midiOutput.sendMessage(msg);
    midiOutput.setChannel(save_channel);
}

void Hc2Module::sendNoteOff(uint8_t channel, uint8_t note, uint8_t velocity)
{
    midi::Message msg;
    SetNoteOff(msg, channel, note, velocity);
    auto save_channel = midiOutput.getChannel();
    midiOutput.setChannel(channel);
    midiOutput.sendMessage(msg);
    midiOutput.setChannel(save_channel);
}

void Hc2Module::sendControlChange(uint8_t channel, uint8_t cc, uint8_t value)
{
    assert(-1 == midiOutput.getChannel() || channel == midiOutput.getChannel());
    midi::Message msg;
    SetCC(msg, channel, cc, value);
    midiOutput.sendMessage(msg);
}

void Hc2Module::sendProgramChange(uint8_t channel, uint8_t program)
{
    assert(-1 == midiOutput.getChannel() || channel == midiOutput.getChannel());
    midi::Message msg;
    SetProgramChange(msg, channel, program);
    midiOutput.sendMessage(msg);
}

void Hc2Module::onMessage(const midi::Message& message)
{
    ++midi_count;

}

void Hc2Module::process(const ProcessArgs& args)
{

}

}

Model *modelHc2 = createModel<pachde::Hc2Module, pachde::Hc2ModuleWidget>("pachde-hc-2");