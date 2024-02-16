// Copyright (C) Paul Chase Dempsey
#include "cc_map_widget.hpp"

namespace pachde {

void CCMap::onHover(const HoverEvent& e)
{
    TipWidget::onHover(e);
    cc = static_cast<uint8_t>(std::round(e.pos.x + e.mouseDelta.x)) / 2;
    e.consume(this);
    hovering = true;
}

void CCMap::onLeave(const LeaveEvent& e)
{
    TipWidget::onLeave(e);
    cc = last_cc = last_value = 0xff;
    hovering = false;
}

void CCMap::step()
{
    if (hovering) {
        last_cc = cc;
        auto map = getMap();
        if (!map || (map && (map[cc] == 0))) {
            describe("");
        } else if (map) {
            if (map[cc] == last_value) return;
            last_value = map[cc];
            describe(format_string("cc %d:%d %s", cc, last_value, continuumCC(cc)));
        }
        TipWidget::destroyTip();
        TipWidget::createTip();
    }
    TipWidget::step();
}

void CCMap::drawMap(NVGcontext* vg, uint8_t * map, float x, float y)
{
    BoxRect(vg, x, y, 254, 18, RampGray(G_35), .5f);
    if (!map) return;
    ++x;
    y += 17.f;
    for (auto n = 0; n < 127; ++n, ++map, x += 2) {
        if (auto v = *map) {
            auto co = RampGray(G_85);
            switch (v) {
            case 64: co = GetStockColor(StockColor::Aquamarine_Medium); break;
            case 127: co = GetStockColor(StockColor::Tomato); break;
            }
            Line(vg, x, y, x, y - v/8.f, co, 1.6f);
        }
    }
}

uint8_t * CCMap::getMap()
{
    if (!host) return nullptr;
    auto hc1 = host->getPartner();
    if (!hc1) return nullptr;
    switch (channel) {
    case CCMapChannel::One: return hc1->ch0_cc_value;
    case CCMapChannel::Sixteen: return hc1->ch15_cc_value;
    default: return nullptr;
    }
}

void CCMap::drawLayer(const DrawArgs &args, int layer)
{
    TipWidget::drawLayer(args, layer);
    if (1 != layer) return;
    drawMap(args.vg, getMap(), 0.f, 0.f);
}

// void CCMap::draw(const DrawArgs& args)
// {
//     TipWidget::draw(args);
// }

}