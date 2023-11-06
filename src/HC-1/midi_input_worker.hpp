#pragma once
#ifndef MIDI_INPUT_WORKER_HPP_INCLUDED
#define MIDI_INPUT_WORKER_HPP_INCLUDED
#include <condition_variable>
#include <mutex>
#include <thread>
#include <rack.hpp>
#include "HC-1.hpp"
#include "../em_midi.hpp"

using namespace ::rack;

namespace pachde {

struct MidiInputWorker
{
    bool stop{false};
    bool pausing{false};
    rack::dsp::RingBuffer<uMidiMessage, 1024> midi_consume;
    std::mutex m;
    std::condition_variable cv;
    Hc1Module* hc1;
    rack::Context* context;
    std::thread my_thread;

    MidiInputWorker(Hc1Module* HC1, rack::Context* rack) : hc1(HC1), context(rack) {}

    void start();
    void pause();
    void resume();
    void post_quit();
    void post_message(uMidiMessage msg);
    void run();
};

}
#endif