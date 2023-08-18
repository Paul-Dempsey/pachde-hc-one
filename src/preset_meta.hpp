#pragma once
#ifndef PRESET_META_HPP_INCLUDED
#define PRESET_META_HPP_INCLUDED
#include <rack.hpp>

namespace pachde {

enum class PresetGroup : uint8_t {
    Unknown,
    Category,
    Type,
    Character,
    Matrix,
    Setting,
};

const char * toString(PresetGroup group);

std::pair<std::string::const_iterator, std::string::const_iterator>
get_token(std::string::const_iterator start, std::string::const_iterator end, std::function<bool(char)> pred);

union CategoryCode {
    uint16_t code;
    char letters[2];

    CategoryCode(uint16_t u) : code(u) {}
    operator uint16_t() const { return code; }
    CategoryCode(const char * code_token) : letters{*code_token, *(code_token + 1)} {}
    CategoryCode(char a, char b) : letters{a, b} {}
    CategoryCode(std::string::const_iterator it) : letters{*it, *(1+it)} {}
    std::string to_string() { return std::string(letters); }
};

struct PresetMeta {
    uint16_t code;
    PresetGroup group;
    uint8_t index;
    const char * name;
    PresetMeta (const char * code_token, PresetGroup g, uint8_t index, const char * name)
    :   code(CategoryCode(code_token)),
        group(g),
        index(index),
        name(name)
    {}
};

class HCCategoryCode
{
    std::vector<std::shared_ptr<PresetMeta>> data;

public:
    static bool is_space(char c) { return ' ' == c || '\n' == c || '\r' == c || '\t' == c; }
    static bool not_space(char c) { return !is_space(c); }
    static bool is_underscore(char c) { return '_' == c; }

    HCCategoryCode();

    std::shared_ptr<PresetMeta> find(uint16_t key) const;
    void foreach_code(std::string text, std::function<bool(uint16_t)> callback) const;
    std::string make_category_json(std::string text) const;
    std::vector<uint16_t> make_category_code_list(std::string text) const;
    std::vector<std::shared_ptr<PresetMeta>> make_category_list(std::string text) const;
};
extern const HCCategoryCode hcCategoryCode;

}
#endif
