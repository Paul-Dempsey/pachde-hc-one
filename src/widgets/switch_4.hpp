#pragma once
#ifndef SWITCH_VERT_4_HPP_INCLUDED
#define SWITCH_VERT_4_HPP_INCLUDED
#include <rack.hpp>
#include "../plugin.hpp"
using namespace ::rack;
namespace pachde
{
struct SwitchVert4 : app::SvgSwitch
{
    SwitchVert4()
    {
        shadow->opacity = 0.0;
        addFrame(Svg::load(asset::plugin(pluginInstance, "res/SwitchVert4-0.svg")));
        addFrame(Svg::load(asset::plugin(pluginInstance, "res/SwitchVert4-1.svg")));
        addFrame(Svg::load(asset::plugin(pluginInstance, "res/SwitchVert4-2.svg")));
        addFrame(Svg::load(asset::plugin(pluginInstance, "res/SwitchVert4-3.svg")));
    }
};
struct SwitchHorz4 : app::SvgSwitch
{
    SwitchHorz4()
    {
        shadow->opacity = 0.0;
        addFrame(Svg::load(asset::plugin(pluginInstance, "res/SwitchHorz4-0.svg")));
        addFrame(Svg::load(asset::plugin(pluginInstance, "res/SwitchHorz4-1.svg")));
        addFrame(Svg::load(asset::plugin(pluginInstance, "res/SwitchHorz4-2.svg")));
        addFrame(Svg::load(asset::plugin(pluginInstance, "res/SwitchHorz4-3.svg")));
    }
};

}
#endif