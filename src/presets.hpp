#pragma once
#ifndef PRESETS_HPP_INCLUDED
#define PRESETS_HPP_INCLUDED
#include "plugin.hpp"
#include "text.hpp"

namespace pachde {

inline const char * preset_type_name(uint8_t bank_hi) {
    if (127 == bank_hi) return "system";
    if (126 == bank_hi) return "edit";
    if (0 == bank_hi) return "user";
    return "?";
}

template<const size_t size>
class FixedStringBuffer {
    char data[size];
    char * end;
    const char * const lim = data + size;

public:
    FixedStringBuffer() { clear(); }

    bool empty() const { return end == data; }
    const char * str() const { return data; }

    void clear() {
        *data = 0;
        end = data;
    }

    void build(char a) {
        if (end < lim-1) {
            *(end + 1) = 0;
            *end++ = a;
        }
    }
};

static const char * const PRESET_FORMAT = "%s\n(%s %d.%d)";

class Preset {
    FixedStringBuffer<32> _name;
    FixedStringBuffer<256> _text;
    std::string id;

    void default_macros()
    {
        macro[0] = "i";
        macro[1] = "ii";
        macro[2] = "iii";
        macro[3] = "iv";
        macro[4] = "v";
        macro[5] = "vi";
    }
    int index_of_id(const std::string& id)
    {
        if (0 == id.compare("i")) return 0;
        if (0 == id.compare("ii")) return 1;
        if (0 == id.compare("iii")) return 2;
        if (0 == id.compare("iv")) return 3;
        if (0 == id.compare("g1")) return 4;
        if (0 == id.compare("g2")) return 5;
        if (0 == id.compare("v")) return 4;
        if (0 == id.compare("vi")) return 5;
        return -1;
    }

public:
    uint8_t bank_hi; // cc0
    uint8_t bank_lo; // cc32
    uint8_t number;  // program change
    std::string macro[6];

    void build_name(char a) { _name.build(a); }
    void build_text(char a) { _text.build(a); }

    void clear_name() { _name.clear(); }
    void clear_text() { _text.clear(); }
    bool name_empty() const { return _name.empty(); }
    bool text_empty() const { return _text.empty(); }
    const char * name() const { return _name.str(); }
    const char * text() const { return _text.str(); }

    Preset() : bank_hi(0), bank_lo(126), number(0)
    {
        id.reserve(16);
        macro[0].reserve(56); // max according to the EaganMatrix Programming cookbook
        macro[1].reserve(56);
        macro[2].reserve(56);
        macro[3].reserve(56);
        macro[4].reserve(56);
        macro[5].reserve(56);
        default_macros();
    }
    void clear() {
        _name.clear();
        _text.clear();
        default_macros();
    }

    void parse_text() {
        default_macros();
        if (_text.empty()) return;
        enum ParseState { Macro, Name, Value } state = ParseState::Macro;
        int macro_index = -1;
        for (auto p = _text.str(); *p; ++p) {
            char ch = *p;
            switch (state) {
                case ParseState::Macro: {
                    switch (ch) {
                        case ' ': case '\r':case '\n':
                            break;
                        case '=':
                            macro_index = index_of_id(id);
                            id = "";
                            if (-1 != macro_index) {
                                macro[macro_index] = "";
                            }
                            state = ParseState::Name;
                            break;
                        default:
                            id.push_back(ch);
                            break;
                    }
                } break;
                case ParseState::Name: {
                    switch (ch) {
                        case ' ': case '\r': case '\n':
                            state = ParseState::Macro;
                            break;
                        case '_':
                            state = ParseState::Value;
                            break;
                        default:
                            if (-1 != macro_index) {
                                macro[macro_index].push_back(ch);
                            }
                            break;
                    }
                } break;
                case ParseState::Value: {
                    switch (ch) {
                        case ' ': case '\r':case '\n':
                            state = ParseState::Macro;
                            break;
                        default:
                            break;
                    }
                } break;
            }
        }
    }

    std::string describe() {
        return format_string(PRESET_FORMAT, name(), preset_type_name(bank_hi), bank_lo, number + 1);
    }
};

struct MinPreset {
    std::string name;
    std::string text;
    uint8_t bank_hi; // cc0
    uint8_t bank_lo; // cc32
    uint8_t number;  // program change
    bool favorite;

    MinPreset() : bank_hi(0), bank_lo(0), number(0), favorite(false) {}

    MinPreset(const Preset& preset) 
    :   name(preset.name()),
        text(preset.text()),
        bank_hi(preset.bank_hi),
        bank_lo(preset.bank_lo),
        number(preset.number)
    {
    }

    std::string describe() {
        return format_string(PRESET_FORMAT, name.c_str(), preset_type_name(bank_hi), bank_lo, number + 1);
    }

    bool is_user_preset() const {
        return 0 == bank_hi;
    }

    bool is_sys_preset() const {
        return 127 == bank_hi;
    }
};

}
#endif