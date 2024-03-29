// Copyright (C) Paul Chase Dempsey
#include "port.hpp"

namespace pachde {


void ColorPort::draw(const DrawArgs& args)
{
    auto vg = args.vg;
    float center = box.size.x / 2.f;
    CircleGradient(vg, center, center, center, collar1, collar2);
    Circle(vg, center, center, center - .5f, bezel);
    Circle(vg, center, center, center * .7f, tube);
    CircleGradient(vg, center, center, center * .65f - .85f, bevel1, bevel2);
    Circle(vg, center, center, center *.5f, ring);
    Circle(vg, center, center, center *.25f, sleeve);
    Circle(vg, center, center, center *.15f, RampGray(G_BLACK));
}

}