// Copyright (C) Paul Chase Dempsey
#include "small_push.hpp"

namespace pachde {
inline float fromDesign(float q) {
    return (q * 12.f) / 12.f;
}

void SmallPush::draw(const DrawArgs& args)
{

    TipWidget::draw(args);

    auto vg = args.vg;
    float cx = box.size.x/2.f;
    auto y = cx * .98f;
    auto r_face = fromDesign(4.7f);
    Circle(vg, cx, cx, cx, bezel);
    CircleGradient(vg, cx, y, fromDesign(5.37f), pressed ? bevel2 : bevel1, pressed ? bevel1 : bevel2);
    Circle(vg, cx, y, r_face, face);
    CircleGradient(vg, cx, y, r_face, pressed ? face2 : face1, pressed ? face1: face2);
}

}