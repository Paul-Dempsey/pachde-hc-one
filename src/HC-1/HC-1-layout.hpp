// Copyright (C) Paul Chase Dempsey
#pragma once

namespace pachde {
constexpr const float PANEL_WIDTH = 360.f;
constexpr const float PRESET_TOP = 37.f;
constexpr const float PRESET_LEFT = 7.5f;
constexpr const float PRESET_WIDTH = 320.f;
constexpr const float PRESET_RIGHT = PRESET_LEFT + PRESET_WIDTH;
constexpr const float PRESET_SLOT_HEIGHT = 27.f;
constexpr const float PRESET_HEIGHT = 8.f * PRESET_SLOT_HEIGHT;
constexpr const float PRESET_BOTTOM = PRESET_TOP + PRESET_HEIGHT;

constexpr const float RIGHT_COLUMN_BUTTONS = PRESET_RIGHT + (PANEL_WIDTH - PRESET_RIGHT)*.5f;
constexpr const float KNOB_LEFT   = 45.f;
constexpr const float KNOB_SPREAD = 54.25f;
constexpr const float KNOB_ROW_1  = 287.f;
constexpr const float KNOB_ROW_2  = 345.f;
constexpr const float RKNOB_LEFT  = KNOB_LEFT; //- KNOB_SPREAD *.5f;

constexpr const float LIGHT_SPREAD = 4.f;
constexpr const float LABEL_OFFSET = 20.f;
constexpr const float STATIC_LABEL_OFFSET = 29.5f;

constexpr const float CV_COLUMN_OFFSET = 24.f;
constexpr const float RB_OFFSET = 20.f;
constexpr const float RB_VOFFSET = 15.f;
constexpr const float CV_ROW_1 = KNOB_ROW_1 + 6.f;
constexpr const float CV_ROW_2 = KNOB_ROW_2 + 6.f;

constexpr const float RECIRC_BOX_TOP = KNOB_ROW_2 - LABEL_OFFSET - 14.f;
constexpr const float RECIRC_BOX_LEFT = RKNOB_LEFT - 35.5f;
constexpr const float RECIRC_BOX_WIDTH = KNOB_SPREAD * 5.f;
constexpr const float RECIRC_BOX_RIGHT = RECIRC_BOX_LEFT + RECIRC_BOX_WIDTH;
constexpr const float RECIRC_BOX_HEIGHT = 52.5f;
constexpr const float RECIRC_BOX_BOTTOM = RECIRC_BOX_TOP + RECIRC_BOX_HEIGHT;
constexpr const float RECIRC_TITLE_WIDTH = 70.f;
constexpr const float RECIRC_BOX_CENTER = RECIRC_BOX_LEFT + RECIRC_BOX_WIDTH *.5f;
constexpr const float RECIRC_EXTEND_CENTER = RKNOB_LEFT + KNOB_SPREAD * 3.5f;
constexpr const float RECIRC_ENABLE_CENTER = RKNOB_LEFT + KNOB_SPREAD * .5f;

constexpr const float STATUS_LEFT = 48.f;
constexpr const float STATUS_SPREAD = 4.95f;
}