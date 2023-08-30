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

bool preset_order(const std::shared_ptr<Preset>& p1, const std::shared_ptr<Preset>& p2)
{
    uint32_t c1 = (p1->bank_hi << 16) | (p1->bank_lo << 8) | p1->number;
    uint32_t c2 = (p2->bank_hi << 16) | (p2->bank_lo << 8) | p2->number;
    return c1 < c2;
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
    return preset_order(p1, p2);
}

}