#pragma once
#ifndef MISC_HPP_INCLUDED
#define MISC_HPP_INCLUDED
#include <rack.hpp>
using namespace ::rack;
namespace pachde {

std::string format_string(const char *fmt, ...);
size_t common_prefix_length(const std::string& alpha, const std::string& beta);
bool is_EMDevice(const std::string& name);
std::string FilterDeviceName(std::string text);
//std::string AbbreviatedName(std::string name);
std::string TempName(const std::string& suffix);

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

enum Expansion {
    None  = 0x00,
    Left  = 0x01,
    Right = 0x10,
    Both  = 0x11
};
struct ExpanderPresence {
    Expansion exp;
    //Expansion operator() () const { return exp; }
    ExpanderPresence() : exp(Expansion::None) {}
    ExpanderPresence(Expansion e) : exp(e) {}
    bool operator == (const ExpanderPresence& rhs) { return exp == rhs.exp; }
    bool operator == (const Expansion& rhs) { return exp == rhs; }
    static ExpanderPresence fromRackSide(int rackSide) {
        return ExpanderPresence(rackSide == 0 ? Expansion::Left : rackSide == 1 ? Expansion:: Right : Expansion::None);
    }
    void add(Expansion expansion) { exp = static_cast<Expansion>(exp | expansion); }
    void remove(Expansion expansion) { exp = static_cast<Expansion>(exp & ~expansion); }
    void addRight() { add(Expansion::Right); }
    void addLeft() { add(Expansion::Left); }
    void removeRight() { remove(Expansion::Right); }
    void removeLeft() { remove(Expansion::Left); }
    void clear() { exp = Expansion::None; }
    bool right() const { return exp & Expansion::Right; }
    bool left() const { return exp & Expansion::Left; }
    bool both() const { return exp == Expansion::Both; }
    bool empty() const { return exp == Expansion::None; }
    bool any() { return !empty(); }
};


}
#endif