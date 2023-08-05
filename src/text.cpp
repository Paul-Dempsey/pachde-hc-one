#include "text.hpp"
#include <cctype>

namespace pachde {

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
    if (name.size() <= 8) return name;
    std::string result;
    bool was_space = true;
    for (unsigned char ch: name) {
        if (std::isupper(ch)) {
            result.push_back(ch);
        } else if (std::isdigit(ch)) {
            result.push_back(ch);
        } else if (!std::isspace(ch) && was_space) {
            result.push_back(ch);
        }
        was_space = std::isspace(ch);
    }
    return result;
}

void SetTextStyle(NVGcontext *vg, std::shared_ptr<window::Font> font, NVGcolor color, float height)
{
    assert(FontOk(font));
    nvgFillColor(vg, color);
    nvgFontFaceId(vg, font->handle);
    nvgTextLetterSpacing(vg, 0.f);
    nvgFontSize(vg, height);
}

void CenterText(NVGcontext *vg, float x, float y, const char * text, const char * end)
{
    // nvg offers a variety of text alignment options
    nvgTextAlign(vg, NVG_ALIGN_CENTER);
    nvgText(vg, x, y, text, end);
}

void RightAlignText(NVGcontext *vg, float x, float y, const char * text, const char * end, BaselineCorrection correction)
{
    float bounds[4] = { 0, 0, 0, 0 };
    nvgTextBounds(vg, 0, 0, text, end, bounds);
    auto descent = correction == BaselineCorrection::Baseline ? bounds[3] : 0.;
    nvgText(vg, x - bounds[2], y - descent, text, end);
}

}