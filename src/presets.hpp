#pragma once
#ifndef PRESETS_HPP_INCLUDED
#define PRESETS_HPP_INCLUDED
#include "plugin.hpp"
#include "misc.hpp"
#include "preset_meta.hpp"

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

static const char * const PRESET_FORMAT = "%s%c(%s %d.%d)%c%s";

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
                        case ' ': case '\r': case '\n':
                            state = ParseState::Macro;
                            break;
                        default:
                            break;
                    }
                } break;
            }
        }
    }

    std::string describe(bool multi_line = true) {
        auto line_break = multi_line ? '\n' : ' ';
        return format_string(PRESET_FORMAT, name(), 
            line_break, preset_type_name(bank_hi), bank_lo, number + 1,
            line_break, text());
    }
};

struct MinPreset {
    std::string name;
    std::string text;
    std::string meta_text;
    uint8_t bank_hi; // cc0
    uint8_t bank_lo; // cc32
    uint8_t number;  // program change
    bool favorite;
    int favorite_order;

    MinPreset() : bank_hi(0), bank_lo(0), number(0), favorite(false), favorite_order(-1) {}

    explicit MinPreset(const Preset& preset) 
    :   name(preset.name()),
        text(preset.text()),
        bank_hi(preset.bank_hi),
        bank_lo(preset.bank_lo),
        number(preset.number),
        favorite(false),
        favorite_order(-1)
    {
    }

    // not cached
    std::string make_text_json() {
        return hcCategoryCode.make_category_json(text);
    }

    // cached
    std::string meta() {
        if (meta_text.empty() && !text.empty()) {
            meta_text = make_text_json();
        }
        return meta_text;
    }

    void clear() {
        name.clear();
        text.clear();
        meta_text.clear();
    }

    std::string describe(bool multi_line = true)
    {
        char line_break = multi_line ? '\n' : ' ';
        return format_string(PRESET_FORMAT, name.c_str(),
            line_break, preset_type_name(bank_hi), bank_lo, number + 1,
            line_break, meta().c_str());
    }

    bool isSamePreset(const Preset& other) {
        return (bank_lo == other.bank_lo) && (0 == name.compare(other.name()));
    }
    bool isSamePreset(const MinPreset& other) {
        return (bank_lo == other.bank_lo) && (0 == name.compare(other.name));
    }
    bool isSysPreset() {
        return 127 == bank_hi;
    }

    json_t* toJson() {
        json_t* root = json_object();
        json_object_set_new(root, "hi", json_integer(bank_hi));
        json_object_set_new(root, "lo", json_integer(bank_lo));
        json_object_set_new(root, "num", json_integer(number));
        json_object_set_new(root, "name", json_stringn(name.c_str(), name.size()));
        json_object_set_new(root, "text", json_stringn(text.c_str(), text.size()));
        json_object_set_new(root, "fav", json_boolean(favorite));
        json_object_set_new(root, "ord", json_integer(favorite_order));
        return root;
    }

    void fromJson(const json_t* root) {
        auto j = json_object_get(root, "hi");
        if (j) {
            bank_hi = json_integer_value(j);
        }
        j = json_object_get(root, "lo");
        if (j) {
            bank_lo = json_integer_value(j);
        }
        j = json_object_get(root, "num");
        if (j) {
            number = json_integer_value(j);
        }
        j = json_object_get(root, "name");
        if (j) {
            name = json_string_value(j);
        }
        j = json_object_get(root, "text");
        if (j) {
            text = json_string_value(j);
        }
        favorite = GetBool(root, "fav", false);
        j = json_object_get(root, "ord");
        if (j) {
            favorite_order = json_integer_value(j);
        } else {
            favorite_order = -1;
        }
    }

};


struct IPresetHolder
{
    virtual bool isCurrentPreset(std::shared_ptr<MinPreset> preset) { return false; }
    virtual void setPreset(std::shared_ptr<MinPreset> preset) {}
    virtual void addFavorite(std::shared_ptr<MinPreset> preset) {}
    virtual void unFavorite(std::shared_ptr<MinPreset> preset) {}
};

}
#endif