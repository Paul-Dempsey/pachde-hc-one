#include "he_group.hpp"
namespace he_group {

#define VERBOSE_LOG
#include "debug_log.hpp"

bool he_item_order(const HEPresetItem& i1, const HEPresetItem& i2)
{
    return i1.index < i2.index;
}

enum class HakenPresetItemParseState {
    start,
    num,
    comma,
    quote,
    end
};

using CBIT = std::vector<uint8_t>::const_iterator;
using PS = HakenPresetItemParseState;

bool parse_line(CBIT it, CBIT end, std::vector<HEPresetItem>& result)
{
    PS state = PS::start;
    int num = 0;
    while (it != end && state != PS::end) {
        switch (state) {
        case PS::start:
            if (std::isspace(*it)) {
                ++it;
            } else if (std::isdigit(*it)) {
                num = (*it - '0');
                ++it;
                state = PS::num;
            } else {
                return false;
            }
            break;

        case PS::num:
            if (std::isdigit(*it)) {
                num = num * 10 + (*it - '0');
                ++it;
            } else {
                state = PS::comma;
            }
            break;

        case PS::comma:
            if (',' == *it) {
                state = PS::quote;
                ++it;
            } else if (std::isspace(*it)) {
                ++it;
            } else {
                return false;
            }
            break;

        case PS::quote:
            if ('"' == *it) {
                ++it;
                state = PS::end;
            } else if (std::isspace(*it)) {
                ++it;
            } else {
                state = PS::end;
            }
            break;

        case PS::end:
            assert(false);
            break;
        }
    }

    CBIT last = end;
    // trim trailing space and quote
    --last;
    while (*last <= 32) { --last; }
    if ('"' == *last) { --last; }
    end = last + 1;

    bool dot_mid = false;
    if ('d' == *last || 'D' == *last) {
        --last;
        if ('i' == *last || 'I' == *last) {
            --last;
            if ('m' == *last || 'M' == *last) {
                --last;
                if ('.' == *last) {
                    dot_mid = true;
                }
            }
        }
    }
    if (dot_mid) end = last;
    result.push_back(HEPresetItem{num, std::string(it, end)});
    return true;
}

std::vector<HEPresetItem> ReadGroupFile(const std::string& path)
{
    std::vector<HEPresetItem> result;
    if (path.empty()) return result;
    if (!system::isFile(path)) return result;
    try {
        auto bytes = system::readFile(path);
        if (bytes.size() <= 5) { return result; }

        auto it = bytes.cbegin();
        while (it != bytes.cend()) {
            auto line_start = it;
            while (it != bytes.cend() && *it != '\n') {
                ++it;
            }
            if (!parse_line(line_start, it, result)) { break; }
            while (it != bytes.cend() && *it <= 32) { ++it; }
        }
        std::sort(result.begin(), result.end(), he_item_order);
    } catch(Exception& e) {
        DebugLog("Exception %s", e.msg.c_str());
    }
    return result;
}
}