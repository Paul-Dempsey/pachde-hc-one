#pragma once
#ifndef MISC_HPP_INCLUDED
#define MISC_HPP_INCLUDED
#include <rack.hpp>
using namespace ::rack;
namespace pachde {

std::string format_string(const char *fmt, ...);
size_t common_prefix_length(std::string alpha, std::string beta);
bool is_EMDevice(const std::string name);
std::string FilterDeviceName(std::string text);
//std::string AbbreviatedName(std::string name);
std::string TempName();

inline bool GetBool(const json_t* root, const char* key, bool default_value) {
    auto j = json_object_get(root, key);
    return j ? json_is_true(j) : default_value;
}

inline float GetFloat(const json_t* root, const char* key, float default_value) {
    auto j = json_object_get(root, key);
    return j ? json_real_value(j) : default_value;
}

enum class InitState {
    Uninitialized,
    Pending,
    Complete,
    Broken
};

inline const char * InitStateName(InitState state) {
    switch (state) {
        case InitState::Uninitialized: return "Uninitialized";
        case InitState::Pending: return "Pending";
        case InitState::Complete: return "Complete";
        case InitState::Broken: return "Broken";
        default: return "(unknown)";
    }
}

}
#endif