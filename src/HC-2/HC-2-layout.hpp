#pragma once

namespace pachde {
constexpr const float PAD = 1.f;
constexpr const float MORE_PAD = 4.f;
constexpr const float KNOB_BOX_HEIGHT = 62.5f;
constexpr const float KNOB_RADIUS = 12.f;
constexpr const float HALF_KNOB = KNOB_RADIUS *.5f;
constexpr const float KNOB_SPREAD = 54.25f;
constexpr const float KNOB_COL1 = 35.f;
constexpr const float KNOB_ROW = 44.f;

constexpr const float REL_OFFSET = 20.f;
constexpr const float REL_VOFFSET = 10.f;
constexpr const float CV_COLUMN_OFFSET = 24.f;
constexpr const float CV_ROW_OFFSET = 6.f;
constexpr const float STATIC_LABEL_OFFSET = 29.5f;

constexpr const float COMP_BOX_LEFT = 7.5;
constexpr const float COMP_BOX_TOP = 35.f;
constexpr const float COMP_BOX_WIDTH = KNOB_SPREAD * 4.f;

constexpr const float TEQ_BOX_LEFT = 7.5f;
constexpr const float TEQ_BOX_TOP = COMP_BOX_TOP + KNOB_BOX_HEIGHT + 7.5f;
constexpr const float TEQ_BOX_WIDTH = KNOB_SPREAD * 3.f;

}