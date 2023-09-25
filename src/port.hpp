#pragma once
#include <rack.hpp>
#include "colors.hpp"
using namespace ::rack;

namespace pachde {

struct ColorPort : PortWidget
{
    NVGcolor ring = RampGray(G_40);
    NVGcolor collar1, collar2, bezel, tube, bevel1, bevel2, sleeve;

    ColorPort()
    { 
        box.size.x = box.size.y = 14.f;
        sleeve  = RampGray(G_15);
        tube    = RampGray(G_05);
        collar1 = RampGray(G_50);
        collar2 = RampGray(G_30);
        bezel   = RampGray(G_25);
        bevel1  = RampGray(G_65);
        bevel2  = RampGray(G_85);
    }
	void draw(const DrawArgs& args) override;

    void setMainColor(NVGcolor color)
    {
        ring = isColorTransparent(color) ? RampGray(G_40) : color;
    }
    NVGcolor getMainColor() { return ring; }
};

}