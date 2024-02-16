// Copyright (C) Paul Chase Dempsey
#include "presets.hpp"

namespace pachde {

void Preset::clear()
{
    name.clear();
    text.clear();
    meta_text.clear();
    categories.clear();
}

void Preset::ensure_category_list()
{
    if (categories.empty() && !text.empty()) {
        FillCategoryCodeList(text, categories);
    }
}

const std::vector<uint16_t>& Preset::get_category_list()
{
    ensure_category_list();
    return categories;
}

std::string Preset::meta()
{
    if (meta_text.empty() && !text.empty()) {
        meta_text = make_friendly_text();
    }
    return meta_text;
}

std::string Preset::make_friendly_text()
{
    return hcCategoryCode.make_category_mulitline_text(text);
}

std::string Preset::describe_short()
{
    return format_string("%s %d.%d.%d", name.c_str(), bank_hi, bank_lo, number);
}

std::string Preset::describe(bool multi_line /*= true*/)
{
    if (name.empty()) return "-";
    char line_break = multi_line ? '\n' : ' ';
    auto m = meta();
    return format_string(PRESET_FORMAT, name.c_str(),
        line_break, preset_type_name(bank_hi), bank_lo, number + 1,
        line_break, m.empty() ? "-" : m.c_str() );
}

uint16_t Preset::primaryCategory()
{
    ensure_category_list();
    if (categories.empty()) return OT;
    return *categories.cbegin();
}

std::string Preset::categoryName()
{
    return CategoryCode(primaryCategory()).to_string();
}

bool Preset::is_same_preset(const LivePreset& other)
{
    if ((bank_hi == other.bank_hi) 
        && (bank_lo == other.bank_lo)
        && (number == other.number)
        && (0 == name.compare(other.name()))
    ) {
        return true; 
    }

    if ((other.bank_hi == 126)
        && (bank_lo == clamp(other.bank_lo - 1, 0, 127))
        && (number == clamp(other.number - 1, 0, 127))
        && (0 == name.compare(other.name()))
    ) {
        assert(0 == bank_hi || 127 == bank_hi);
        return true;
    }
    if ((0 == bank_hi || 127 == bank_hi)
        && (other.bank_hi == 126)
        && (other.bank_lo == 0)
        && (other.number == 0)
        && (0 == name.compare(other.name()))
    ) {
        return true;
    }

    // if (0 == name.compare(other.name())) {
    //     nada();// missing case?
    // }
    return false;
}

bool Preset::is_same_preset(const Preset& other)
{
    if ((bank_hi == other.bank_hi) 
        && (bank_lo == other.bank_lo)
        && (number == other.number)
    ) {
        // if (name.compare(other.name)) {
        //     assert(false);
        // }
        return true;
    }
    return false;
}

json_t* Preset::toJson()
{
    json_t* root = json_object();
    json_object_set_new(root, "hi", json_integer(bank_hi));
    json_object_set_new(root, "lo", json_integer(bank_lo));
    json_object_set_new(root, "num", json_integer(number));
    json_object_set_new(root, "name", json_stringn(name.c_str(), name.size()));
    json_object_set_new(root, "text", json_stringn(text.c_str(), text.size()));
    return root;
}

void Preset::fromJson(const json_t* root)
{
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
}

// -----------------------------------------------------------------
// LivePreset

void LivePreset::default_macros()
{
    macro[0] = "i";
    macro[1] = "ii";
    macro[2] = "iii";
    macro[3] = "iv";
    macro[4] = "v";
    macro[5] = "vi";
}

int LivePreset::index_of_id(const std::string& id)
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

LivePreset::LivePreset() : bank_hi(0), bank_lo(126), number(0)
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

void LivePreset::clear()
{
    _name.clear();
    _text.clear();
    default_macros();
}

void LivePreset::parse_text()
{
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

std::string LivePreset::describe_short()
{
    return format_string("%s %d.%d.%d", name(), bank_hi, bank_lo, number);
}

std::string LivePreset::describe(bool multi_line /*= true*/)
{
    auto line_break = multi_line ? '\n' : ' ';
    return format_string(PRESET_FORMAT, name(), 
        line_break, preset_type_name(bank_hi), bank_lo, number,
        line_break, text());
}

bool preset_system_order(const std::shared_ptr<Preset>& p1, const std::shared_ptr<Preset>& p2)
{
    uint32_t c1 = (p1->bank_hi << 16) | (p1->bank_lo << 8) | p1->number;
    uint32_t c2 = (p2->bank_hi << 16) | (p2->bank_lo << 8) | p2->number;
    return c1 < c2;
}

bool preset_category_order(const std::shared_ptr<Preset>& p1, const std::shared_ptr<Preset>& p2)
{
    auto cat1 = p1->get_category_list();
    auto cat2 = p2->get_category_list();
    if (cat1.empty()) {
        return cat2.empty() ? preset_alpha_order(p1, p2) : false;
    }
    if (cat2.empty()) {
        return true;
    }
    auto m1 = hcCategoryCode.find(*cat1.cbegin());
    assert(m1 && m1->group == PresetGroup::Category);
    auto m2 = hcCategoryCode.find(*cat2.cbegin());
    assert(m2 && m2->group == PresetGroup::Category);
    if (m1->group == m2->group) {
        if (m1->group == PresetGroup::Unknown) {
            return preset_alpha_order(p1, p2);
        } else {
            if (m1->index < m2->index) return true;
            if (m1->index == m2->index) return preset_alpha_order(p1, p2);
            return false;
        }
    }
    return preset_alpha_order(p1, p2);
}

bool preset_alpha_order(const std::shared_ptr<Preset>& preset1, const std::shared_ptr<Preset>& preset2)
{
    assert(!preset1->name.empty());
    assert(!preset2->name.empty());
    auto p1 = preset1->name.cbegin();
    auto p2 = preset2->name.cbegin();
    for (; p1 != preset1->name.cend() && p2 != preset2->name.cend(); ++p1, ++p2) {
        if (*p1 == *p2) continue;
        auto c1 = std::tolower(*p1);
        auto c2 = std::tolower(*p2);
        if (c1 == c2) continue;
        if (c1 < c2) return true;
        return false;
    }
    if (p1 == preset1->name.cend() && p2 != preset2->name.cend()) {
        return true;
    }
    return false;
}

bool favorite_order(const std::shared_ptr<Preset>& p1, const std::shared_ptr<Preset>& p2)
{
    if (p1->favorite != p2->favorite) return p1->favorite; // favorites come first
    if (p1->favorite_order >= 0 && p2->favorite_order >= 0) return p1->favorite_order < p2->favorite_order;
    if (p1->favorite_order < 0  && p2->favorite_order >= 0) return false;
    if (p1->favorite_order >= 0 && p2->favorite_order < 0) return true;
    assert(p1->favorite_order < 0 && p2->favorite_order < 0);
    return preset_alpha_order(p1, p2);
}

}