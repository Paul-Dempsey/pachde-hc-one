#pragma once
#ifndef PRESETS_HPP_INCLUDED
#define PRESETS_HPP_INCLUDED
#include "plugin.hpp"
#include "misc.hpp"
#include "preset_meta.hpp"

namespace pachde {
// #define VERBOSE_LOG
// #include "debug_log.hpp"

inline const char * preset_type_name(uint8_t bank_hi) {
    if (127 == bank_hi) return "system";
    if (126 == bank_hi) return "edit";
    if (0 == bank_hi) return "user";
    return "?";
}

template<const size_t size>
class FixedStringBuffer
{
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

class LivePreset
{
    FixedStringBuffer<32> _name;
    FixedStringBuffer<256> _text;
    std::string id;

    void default_macros();
    int index_of_id(const std::string& id);

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

    LivePreset();
    void clear();
    void parse_text();
    std::string describe_short();
    std::string describe(bool multi_line = true);
};

struct Preset
{
    uint8_t bank_hi; // cc0
    uint8_t bank_lo; // cc32
    uint8_t number;  // program change
    std::string name;
    std::string text;
    bool favorite;
    int favorite_order;
    std::string meta_text;
    std::vector<uint16_t> categories;

    Preset()
    :   bank_hi(0), bank_lo(0), number(0),
        name(""), text(""), 
        favorite(false), favorite_order(-1),
        meta_text("")
    {}

    Preset(const Preset & preset) = delete; // no copy constructor

    explicit Preset(const LivePreset& preset) 
    :   bank_hi(preset.bank_hi),
        bank_lo(preset.bank_lo),
        number(preset.number),
        name(preset.name()),
        text(preset.text()),
        favorite(false),
        favorite_order(-1),
        meta_text("")
    {
    }

    void ensure_category_list();
    const std::vector<uint16_t>& get_category_list();

    void nada(); // debugging

    bool isSysPreset() { return 127 == bank_hi; }

    std::string meta(); // cached
    std::string make_friendly_text(); // not cached
    void clear();
    std::string describe_short();
    std::string describe(bool multi_line = true);
    bool is_same_preset(const LivePreset& other);
    bool is_same_preset(const Preset& other);
    json_t* toJson();
    void fromJson(const json_t* root);
};


struct IPresetHolder
{
    virtual bool isCurrentPreset(std::shared_ptr<Preset> preset) { return false; }
    virtual void setPreset(std::shared_ptr<Preset> preset) {}
    virtual void addFavorite(std::shared_ptr<Preset> preset) {}
    virtual void unFavorite(std::shared_ptr<Preset> preset) {}
};

enum class PresetOrder {
    Alpha,
    System,
    Category
};
bool preset_system_order(const std::shared_ptr<Preset>& p1, const std::shared_ptr<Preset>& p2);
bool preset_alpha_order(const std::shared_ptr<Preset>& preset1, const std::shared_ptr<Preset>& preset2);
bool preset_category_order(const std::shared_ptr<Preset>& p1, const std::shared_ptr<Preset>& p2);

inline std::function<bool (const std::shared_ptr<Preset>&, const std::shared_ptr<Preset>&)> getPresetSort(PresetOrder order)
{
    switch (order) {
    case PresetOrder::Alpha: return preset_alpha_order;
    case PresetOrder::System: return preset_system_order;
    case PresetOrder::Category: return preset_category_order;
    default: assert(false); break;
    }
}

bool favorite_order(const std::shared_ptr<Preset>& p1, const std::shared_ptr<Preset>& p2);

}
#endif