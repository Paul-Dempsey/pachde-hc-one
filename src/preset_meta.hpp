#pragma once
#ifndef PRESET_META_HPP_INCLUDED
#define PRESET_META_HPP_INCLUDED
#include <rack.hpp>

namespace pachde {
//#define PRESET_FILTERING

enum class PresetGroup : uint8_t {
    Category,
    Type,
    Character,
    Matrix,
    Setting,
    Unknown,
};

const char * toString(PresetGroup group);
inline bool is_space(char c) { return ' ' == c || '\n' == c || '\r' == c || '\t' == c; }
inline bool not_space(char c) { return !is_space(c); }
inline bool is_underscore(char c) { return '_' == c; }

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
    std::string to_string() { return std::string{letters[0], letters[1]}; }
};

constexpr const uint16_t ST = 0x5453;
constexpr const uint16_t WI = 0x4957;
constexpr const uint16_t VO = 0x4f56;
constexpr const uint16_t KY = 0x594b;
constexpr const uint16_t CL = 0x4c43;
constexpr const uint16_t OT = 0x544f;
constexpr const uint16_t PE = 0x4550;
constexpr const uint16_t PT = 0x5450;
constexpr const uint16_t PR = 0x5250;
constexpr const uint16_t DO = 0x4f44;
constexpr const uint16_t MD = 0x444d;
constexpr const uint16_t CV = 0x5643;
constexpr const uint16_t UT = 0x5455;

#if defined PRESET_FILTERING
class PresetFilter
{
    struct FilterItem {
        uint16_t code;
        bool show;
    };

    std::vector<FilterItem> data;
    size_t hidden = 0;

public:

    PresetFilter() {
        data.reserve(13);
        // sorted order so we can use binary search
        data.push_back(FilterItem{MD, true});
        data.push_back(FilterItem{PE, true});
        data.push_back(FilterItem{WI, true});
        data.push_back(FilterItem{CL, true});
        data.push_back(FilterItem{DO, true});
        data.push_back(FilterItem{VO, true});
        data.push_back(FilterItem{PR, true});
        data.push_back(FilterItem{OT, true});
        data.push_back(FilterItem{PT, true});
        data.push_back(FilterItem{ST, true});
        data.push_back(FilterItem{UT, true});
        data.push_back(FilterItem{CV, true});
        data.push_back(FilterItem{KY, true});
#define VALIDATE_PresetFilter_ORDER
#if defined VALIDATE_PresetFilter_ORDER
        auto a = data.cbegin();
        auto b = data.cbegin() + 1;
        while (b != data.cend()) {
            assert(a->code < b->code);
            ++a;
            ++b;
        }
#endif
    }

    bool isFiltered() const
    {
        return hidden;
    }

    bool isShow(uint16_t code) const
    {
        auto it = std::lower_bound(data.cbegin(), data.cend(), code, [](const FilterItem& item, uint16_t code) { return item.code < code; });
        return  (it == data.cend()) ? false : it->show;
    }

    void setShow(uint16_t code, bool visible)
    {
        auto it = std::lower_bound(data.begin(), data.end(), code, [](const FilterItem& item, uint16_t code) { return item.code < code; });
        if (it != data.end()) {
            it->show = visible;
            if (!visible) {
                ++hidden;
            } else {
                --hidden;
            }
            assert(hidden <= data.size());
        }
    }
    void toggleShow(uint16_t code)
    {
        auto it = std::lower_bound(data.begin(), data.end(), code, [](const FilterItem& item, uint16_t code) { return item.code < code; });
        if (it != data.end()) {
            it->show = !it->show;
            if (it->show) {
                --hidden;
            } else {
                ++hidden;
            }
            assert(hidden <= data.size());
        }
    }
};
#endif

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
    HCCategoryCode();

    std::shared_ptr<PresetMeta> find(uint16_t key) const;
    std::vector<std::shared_ptr<PresetMeta>> make_category_list(const std::string& text) const;
    std::string make_category_json(const std::string& text) const;
    std::string make_category_mulitline_text(const std::string& text) const;
    std::string categoryName(uint16_t key) const;
};

void FillCategoryCodeList(const std::string& text, std::vector<uint16_t>& vec);
void foreach_code(const std::string&, std::function<bool(uint16_t)> callback);

extern const HCCategoryCode hcCategoryCode;

}
#endif
