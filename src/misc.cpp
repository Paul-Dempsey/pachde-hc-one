#include "misc.hpp"

namespace pachde {

bool is_EMDevice(const std::string name)
{
    std::string text = name;
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c){ return std::tolower(c); });
    if (0 == text.compare(0, 8, "continuu", 0, 8)) { return true; }
    if (0 == text.compare(0, 6, "osmose", 0, 6)) { return true; }
    if (0 == text.compare(0, 5, "eagan", 0, 5)) { return true; }
    return false;
}

std::string format_string(const char *fmt, ...)
{
    const int len = 256;
    std::string s(len, '\0');
    va_list args;
    va_start(args, fmt);
    auto r = std::vsnprintf(&(*s.begin()), len + 1, fmt, args);
    return r < 0 ? "??" : s;
}

size_t common_prefix_length(std::string alpha, std::string beta) {
    auto a = alpha.begin(), ae = alpha.end();
    auto b = beta.begin(), be = beta.end();
    int common = 0;
    for (; (a < ae && b < be) && (*a == *b); ++a, ++b, ++common) { }
    return common;
}

std::string AbbreviatedName(std::string name)
{
    if (name.size() <= 12) return name.substr(0, 8);
    std::string result;
    bool was_space = true;
    for (unsigned char ch: name) {
        if (std::isupper(ch)) {
            result.push_back(ch);
        } else if (std::isdigit(ch)) {
            result.push_back(ch);
        } else {
            auto space = std::isspace(ch);
            if (!space && was_space) {
                result.push_back(ch);
            }
            was_space = space;
        }
    }
    return result;
}

std::string FilterDeviceName(std::string text)
{
    #ifdef ARCH_WIN
    if (!text.empty()) {
        text.erase(text.find_last_not_of("0123456789"));
    }
    #endif
    #ifdef ARCH_LIN
    if (!text.empty()) {
        auto pos = text.find(':');
        if (std::string::npos != pos) {
            return text.substr(pos);
        }
    }
    #endif
    return text;
}

}