#pragma once
#ifndef MIDI_INPUT_PROXY_HEADER_INCLUDED
#define MIDI_INPUT_PROXY_HEADER_INCLUDED
#include <rack.hpp>
using namespace ::rack;

namespace pachde {
    
struct IProcessMidi {
    virtual void processMidi(const midi::Message& msg) = 0;
};

struct MidiInputProxy : midi::Input
{
    IProcessMidi * handler = nullptr;
    MidiInputProxy(IProcessMidi * processor) : handler(processor) { }
    void onMessage(const midi::Message& message) override
    {
        handler->processMidi(message);
    }
};

}
#endif