//
// EMCC_VelSplit velocity (note processing) and split modes
//
#pragma once
#ifndef EM_VELOCITY_SPLIT_HPP_INCLUDED
#define EM_VELOCITY_SPLIT_HPP_INCLUDED
#include <rack.hpp>
#include "../misc.hpp"
#include "../em_midi.hpp"
using namespace ::rack;
using namespace ::em_midi;
namespace eagan_matrix {

namespace VelocitySplit_private {
constexpr const uint8_t MASK = U8(EM_Velocity::MASK);
};

struct VelocitySplit
{
    uint8_t vs;

    bool operator == (const VelocitySplit& rhs) const { return vs == rhs.vs; }
    bool operator != (const VelocitySplit& rhs) const { return vs != rhs.vs; }

    void setVelocity(EM_Velocity vel) {
        vs = (vs & ~VelocitySplit_private::MASK) | U8(vel);
    }
    uint8_t getVelocityByte() const { return vs & VelocitySplit_private::MASK; }
    EM_Velocity getVelocity() const {
        return static_cast<EM_Velocity>(getVelocityByte());
    }
    // Split not implemented: it's being removed from the next Haken firmware release
};
}
#endif