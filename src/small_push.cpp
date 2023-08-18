#include "small_push.hpp"

namespace pachde {

void SmallPush::draw(const DrawArgs& args)
{
    TipWidget::draw(args);

    auto vg = args.vg;
    float cx = box.size.x/2.f;
    CircleGradient(vg, cx, cx, cx, collar1, collar2);
    Circle(vg, cx, cx, cx * 0.95f, bezel);
    Circle(vg, cx, cx, cx * .675f, ring);
    if (pressed) {
        CircleGradient(vg, cx, cx, cx * 0.55f, bevel2, bevel1);
        CircleGradient(vg, cx, cx, cx * 0.5f, face2, face1);
    } else {
        CircleGradient(vg, cx, cx, cx * 0.55f, bevel1, bevel2);
        CircleGradient(vg, cx, cx, cx * 0.5f, face1, face2);
    }
}

}