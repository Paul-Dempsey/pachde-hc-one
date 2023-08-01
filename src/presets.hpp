#pragma once
#ifndef PRESETS_HPP_INCLUDED
#define PRESETS_HPP_INCLUDED
#include "plugin.hpp"
namespace pachde {

struct MinPreset {
    std::string name;
    std::string text;
    uint8_t bank_hi; // cc0
    uint8_t bank_lo; // cc32
    uint8_t number;  // program change
};


template<const size_t size>
class FixedStringBuffer {
    char data[size];
    char * end;
    const char * const lim = data + size;

public:
    FixedStringBuffer() { clear(); }

    bool empty() { return end == data; }
    const char * str() { return data; }

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

class Preset {
    FixedStringBuffer<32> _name;
    FixedStringBuffer<256> _text;
    std::string id;

    bool parsed;

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
    bool name_empty() { return _name.empty(); }
    bool text_empty() { return _text.empty(); }
    const char * name() { return _name.str(); }
    const char * text() { return _text.str(); }

    Preset() : parsed(false), bank_hi(0), bank_lo(126), number(0)
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
        parsed = false;
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
        parsed = true;
    }
};

}
#endif