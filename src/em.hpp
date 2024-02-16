// Copyright (C) Paul Chase Dempsey
#pragma once
#ifndef EM_HPP_INCLUDED
#define EM_HPP_INCLUDED
#include "em_midi.hpp"
#include "em_types/em_compressor.hpp"
#include "em_types/em_pedal.hpp"
#include "em_types/em_recirculator.hpp"
#include "em_types/em_rounding.hpp"
#include "em_types/em_tilteq.hpp"
#include "em_types/em_tuning.hpp"

namespace eagan_matrix {

struct EaganMatrix
{
    uint16_t firmware_version;
    uint16_t cvc_version;
    EM_Hardware hardware;
    uint8_t middle_c;
    Rounding rounding;
    Recirculator recirculator;
    Compressor compressor;
    TiltEq tilt_eq;
    PedalInfo pedal1;
    PedalInfo pedal2;
    bool reverse_surface;
    uint8_t global_ActionAesMenuRecirc;

    // levels (PreLevel/PostLevel/AudioIn/LineOut/HeadphoneOut)
    // convolution
    // preserve
    // routing

    EaganMatrix()
    :   firmware_version(0),
        cvc_version(0),
        hardware(EM_Hardware::Unknown),
        middle_c(60),
        pedal1(0),
        pedal2(1),
        reverse_surface(false)
    {
    }

    void clear()
    {
        firmware_version = 0;
        cvc_version = 0;
        hardware = EM_Hardware::Unknown;
        middle_c = 60;
        pedal1 = PedalInfo(0);
        pedal2 = PedalInfo(1);
        recirculator.clear();
        reverse_surface = false;
    }
};


}
#endif