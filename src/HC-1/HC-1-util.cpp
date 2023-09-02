#include "HC-1.hpp"

namespace pachde {

int randomZeroTo(int size)
{
    if (size <= 1) return 0;
    do {
        float r = random::uniform();
        if (r != 1.0f) return static_cast<int>(r * size);
    } while(true);
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
    if (p1->favorite_order >= 0 && p2->favorite_order >= 0) return p1->favorite_order < p2->favorite_order;
    if (p1->favorite_order < 0  && p2->favorite_order >= 0) return false;
    if (p1->favorite_order >= 0 && p2->favorite_order < 0) return true;
    assert(p1->favorite_order < 0 && p2->favorite_order < 0);
    // int cmp = p1->name.compare(p2->name);
    // if (cmp < 0) return true;
    // if (cmp > 0) return false;
    return preset_system_order(p1, p2);
}

}