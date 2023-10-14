#include "misc.hpp"

namespace pachde {

std::string format_string(const char *fmt, ...)
{
    const int len = 512;
    std::string s(len, '\0');
    va_list args;
    va_start(args, fmt);
    auto r = std::vsnprintf(&(*s.begin()), len, fmt, args);
    va_end(args);
    if (r < 0) return "??";
    s.resize(std::min(r, len));
    return s;
}

// case-insensitive
bool alpha_order(const std::string& a, const std::string& b)
{
    if (a.empty()) return false;
    if (b.empty()) return true;
    auto ita = a.cbegin();
    auto itb = b.cbegin();
    for (; ita != a.cend() && itb != b.cend(); ++ita, ++itb) {
        if (*ita == *itb) continue;
        auto c1 = std::tolower(*ita);
        auto c2 = std::tolower(*itb);
        if (c1 == c2) continue;
        if (c1 < c2) return true;
        return false;
    }
    if (ita == a.cend() && itb != b.cend()) {
        return true;
    }
    return false;
}

const char * printable(const std::string& s)
{
    return s.empty() ? "" : s.c_str();
}

std::string spaceless(const std::string& str)
{
    std::string text;
    auto back = std::back_inserter(text);
    for (auto ch: str) {
        if (!std::isspace(ch)) {
            *back++ = ch;
        }
    }
    return text;
}

bool is_safe_file_char(char ch, bool allow_space /*= true*/)
{
    if (ch < 0) return true;
    if (ch < 32) return false;
    else if (' ' == ch) return allow_space;
    else if (std::strchr(":&|/\\?*~<>", ch)) return false;
    return true;
}

std::string to_file_safe(const std::string& str, bool allow_space /*= true*/)
{
    std::string text;
    auto back = std::back_inserter(text);
    bool skipper = false;
    for (auto ch: str) {
        if (is_safe_file_char(ch)) {
            *back++ = ch;
            skipper = '_' == ch;
        } else {
            if (!skipper) {
                *back++ = '_';
                skipper = true;
            }
        }
    }
    return text;
}

std::string to_lower_case(const std::string& name)
{
    std::string text = name;
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c){ return std::tolower(c); });
    return text;
}

std::size_t common_prefix_length(const std::string& alpha, const std::string& beta)
{
    auto a = alpha.cbegin(), ae = alpha.cend();
    auto b = beta.cbegin(), be = beta.cend();
    int common = 0;
    for (; (a < ae && b < be) && (*a == *b); ++a, ++b, ++common) { }
    return common;
}

std::size_t common_prefix_length_insensitive(const std::string& alpha, const std::string& beta)
{
    auto a = alpha.cbegin(), ae = alpha.cend();
    auto b = beta.cbegin(), be = beta.cend();
    int common = 0;
    for (; (a < ae && b < be) && ((*a == *b) || (std::tolower(*a) == std::tolower(*b)));
        ++a, ++b, ++common) { }
    return common;
}

int randomZeroTo(int size)
{
    if (size <= 1) return 0;
    do {
        float r = random::uniform();
        if (r != 1.0f) return static_cast<int>(r * size);
    } while(true);
}

bool GetBool(const json_t* root, const char* key, bool default_value) {
    auto j = json_object_get(root, key);
    return j ? json_is_true(j) : default_value;
}

float GetFloat(const json_t* root, const char* key, float default_value) {
    auto j = json_object_get(root, key);
    return j ? json_real_value(j) : default_value;
}

// std::string AbbreviatedName(std::string name)
// {
//     if (name.size() <= 9) return name.substr(0, 9);
//     std::string result;
//     bool was_space = true;
//     for (unsigned char ch: name) {
//         if (std::isupper(ch)) {
//             result.push_back(ch);
//         } else if (std::isdigit(ch)) {
//             result.push_back(ch);
//         } else {
//             auto space = std::isspace(ch);
//             if (!space && was_space) {
//                 result.push_back(ch);
//             }
//             was_space = space;
//         }
//     }
//     return result;
// }

std::string TempName(const std::string& suffix) {
    return format_string("(%d-%d).%s",
        random::get<uint16_t>(),
        random::get<uint32_t>(),
        suffix.empty() ? ".tmp" : suffix.c_str()
        );
}



const char * InitStateName(InitState state) {
    switch (state) {
    case InitState::Uninitialized: return "Uninitialized";
    case InitState::Pending: return "Pending";
    case InitState::Complete: return "Complete";
    case InitState::Broken: return "Broken";
    default: return "(unknown)";
    }
}

}