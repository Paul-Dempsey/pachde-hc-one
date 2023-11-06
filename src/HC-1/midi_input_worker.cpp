#include "midi_input_worker.hpp"

namespace pachde {

// ======================================================
void MidiInputWorker::start()
{
    my_thread = std::thread(&MidiInputWorker::run, this);
}

void MidiInputWorker::post_quit()
{
    std::unique_lock<std::mutex> lock(m);
    stop = true;
    cv.notify_one();
}

void MidiInputWorker::pause()
{
    std::unique_lock<std::mutex> lock(m);
    midi_consume.clear();
    pausing = true;
    cv.notify_one();
}

void MidiInputWorker::resume()
{
    std::unique_lock<std::mutex> lock(m);
    pausing = false;
    cv.notify_one();
}

void MidiInputWorker::post_message(uMidiMessage msg)
{
    if (stop || pausing) return;
    std::unique_lock<std::mutex> lock(m);
    midi_consume.push(msg);
    cv.notify_one();
}

void MidiInputWorker::run() {
    contextSet(context);
	system::setThreadName("Midi Input worker");
    while (1)
    {
        {
            std::unique_lock<std::mutex> lock(m);
            cv.wait(lock, [this]{ return stop || !midi_consume.empty(); });
            if (stop) {
                return;
            }
            if (pausing) {
                continue;
            }
        }
        while (!midi_consume.empty()) {
            auto msg = midi_consume.shift();
            if (msg.channel() == 0 || msg.channel() == 15) {
                hc1->onMidiMessage(msg);
            } else {
                switch (msg.status()){
                    case MidiStatus_NoteOn:
                    case MidiStatus_NoteOff:
                    case MidiStatus_ChannelPressure:
                    case MidiStatus_PitchBend:
                        hc1->onMidiMessage(msg);
                        break;
                }
            }
        }
    }
}

}