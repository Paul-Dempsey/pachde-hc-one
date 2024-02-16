// Copyright (C) Paul Chase Dempsey
#pragma once
#ifndef CC_MAP_HPP_INCLUDED
#define CC_MAP_HPP_INCLUDED
#include <rack.hpp>
#include "HC-2.hpp"
using namespace ::rack;
namespace pachde {

struct Hc2ModuleWidget;

enum class CCMapChannel {
    One,
    Sixteen
};


constexpr const uint8_t NMB = 0xff; // Not Midi Byte

struct CCMap : TipWidget
{
    CCMapChannel channel;
    Hc2ModuleWidget * host;
    bool hovering;
    uint8_t cc;
    uint8_t last_cc;
    uint8_t last_value;
    CCMap()
    :   host(nullptr),
        hovering(false),
        cc(NMB),
        last_cc(NMB),
        last_value(NMB)
    {
        box.size.x = 254;
        box.size.y = 18;
    }
    void onHover(const HoverEvent& e) override;
    void onLeave(const LeaveEvent& e) override;
    void step() override;
    void drawMap(NVGcontext* vg, uint8_t * map, float x, float y);
    uint8_t * getMap();
    void drawLayer(const DrawArgs& args, int layer) override;
    //void draw(const DrawArgs& args) override;
};

template <typename MAP = CCMap>
MAP* createCCMap(float x, float y, bool center, CCMapChannel chan, Hc2ModuleWidget * the_host)
{
    auto m = new MAP;
    m->box.pos = center 
        ? Vec(x - m->box.size.x *.5, y - m->box.size.y * .5)
        : Vec(x,y);
    m->host = the_host;
    m->channel = chan;
    return m;
}

}
#endif