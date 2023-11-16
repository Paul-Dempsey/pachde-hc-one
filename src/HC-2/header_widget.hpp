#pragma once
#ifndef HEADER_WIDGET_HPP_INCLUDED
#define HEADER_WIDGET_HPP_INCLUDED
#include <rack.hpp>
#include "../colors.hpp"
using namespace ::rack;
namespace pachde {

struct HeaderWidget : TransparentWidget
{
    void draw(const DrawArgs& args) override
    {
        auto vg = args.vg;
        FillRect(vg, 0, 0, box.size.x, 14.f, nvgRGBAf(0x2d/255.f, 0x5d/255.f, 0x86/255.f, .35f));
        BoxRect(vg, 0, 0, box.size.x, box.size.y, RampGray(G_40), 0.5f);
    }
};

template <typename BW = HeaderWidget>
BW * createHeaderWidget(float x, float y, float width, float height)
{
    auto bw = rack::createWidget<BW>(Vec(x,y));
    bw->box.size.x = width;
    bw->box.size.y = height;
    return bw;
}

}
#endif