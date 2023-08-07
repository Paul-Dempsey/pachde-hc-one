#include "small_push.hpp"

namespace pachde {

void SmallPush::draw(const DrawArgs& args)
{
    TipWidget::draw(args);

    auto vg = args.vg;
    float center = box.size.x/2.f;
    CircleGradient(vg, center, center, center, collar1, collar2);
    Circle(vg, center, center, center * 0.95f, bezel);
    Circle(vg, center, center, center * .675f, ring);
    if (pressed) {
        CircleGradient(vg, center, center, center * 0.55f, bevel2, bevel1);
        CircleGradient(vg, center, center, center * 0.5f, face2, face1);
    } else {
        CircleGradient(vg, center, center, center * 0.55f, bevel1, bevel2);
        CircleGradient(vg, center, center, center * 0.5f, face1, face2);
    }
}

}