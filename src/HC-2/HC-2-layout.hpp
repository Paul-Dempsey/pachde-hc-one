#pragma once

namespace pachde {
constexpr const float PAD = 2.f;
constexpr const float MORE_PAD = 4.f;
constexpr const float ROUND_BOX_TOP = 35.f;
constexpr const float ROUND_BOX_LEFT = 7.5f;
constexpr const float ROUND_BOX_WIDTH = 105.f;
constexpr const float ROUND_BOX_HEIGHT = 60.f;
constexpr const float ROUND_BOX_HALF = 110.f * .5f;
constexpr const float KNOB_RADIUS = 12.f;
constexpr const float HALF_KNOB = KNOB_RADIUS *.5f;
constexpr const float REL_OFFSET = 20.f;
constexpr const float REL_VOFFSET = 10.f;
constexpr const float CV_COLUMN_OFFSET = 24.f;
constexpr const float CV_ROW_OFFSET = 6.f;
constexpr const float STATIC_LABEL_OFFSET = 29.5f;
constexpr const float ROUND_KNOB_ROW = 32.5f;
constexpr const float ROUND_COL1 = ROUND_BOX_HALF - KNOB_RADIUS - 3.f * PAD;
constexpr const float ROUND_COL2 = ROUND_BOX_HALF + KNOB_RADIUS + 2.f * PAD;
constexpr const float ROUND_COL3 = ROUND_BOX_WIDTH - KNOB_RADIUS + PAD;
constexpr const float KNOB_SPREAD = 54.25f;
constexpr const float COMP_BOX_LEFT = 7.5;
constexpr const float COMP_BOX_TOP = ROUND_BOX_TOP + ROUND_BOX_HEIGHT + 7.5f;
constexpr const float COMP_BOX_HEIGHT = 62.5f;
constexpr const float COMP_BOX_WIDTH = KNOB_SPREAD * 4.f;
constexpr const float COMP_COL1 = 35.f;
constexpr const float COMP_KNOB_ROW = 46.f;

}