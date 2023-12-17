#pragma once
#ifndef COMMON_LAYOUT_HPP_INCLUDED
#define COMMON_LAYOUT_HPP_INCLUDED

namespace pachde {

constexpr const float KNOB_RADIUS = 12.f;
constexpr const float HALF_KNOB = KNOB_RADIUS * .5f;

// partner widget
constexpr const float PARTNER_LEFT = 3.5f;
constexpr const float PARTNER_TOP = 14.f;
constexpr const float PARTNER_WIDTH = 180.f;

// Vertical knob+CV layout
constexpr const float VK_REL_OFFSET = 13.f;
constexpr const float VK_REL_VOFFSET = 16.25f;
constexpr const float VK_CV_VOFFSET = 23.f;
}
#endif