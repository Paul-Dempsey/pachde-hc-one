#pragma once
#include "plugin.hpp"
#include "colors.hpp"
using namespace ::rack;

namespace pachde {

inline std::shared_ptr<window::Font> GetPluginFontSemiBold(const char * path = NULL)
{
    return APP->window->loadFont(asset::plugin(pluginInstance, path ? path : "res/fonts/HankenGrotesk-SemiBold.ttf"));
}

inline std::shared_ptr<window::Font> GetPluginFontRegular(const char * path = NULL)
{
    return APP->window->loadFont(asset::plugin(pluginInstance, path ? path : "res/fonts/HankenGrotesk-Regular.ttf"));
}

inline bool FontOk(std::shared_ptr<window::Font> font) {
    return font && font->handle >= 0;
}

void SetTextStyle(NVGcontext *vg, std::shared_ptr<window::Font> font, NVGcolor color = RampGray(G_20), float height = 16);

// Center text horizontally on the given point
// Text style must have been previously set
void CenterText(NVGcontext *vg, float x, float y, const char * text, const char * end);

enum class BaselineCorrection {
    None,
    Baseline
};

// The y coordinate is the baseline (BaselineCorrection::none) 
// or the bottom of text box (BaselineCorrection::Baseline).
// Text style must have been previously set.
void RightAlignText(NVGcontext *vg, float x, float y, const char * text, const char * end, BaselineCorrection correction = BaselineCorrection::None);

enum class TextAlignment { Left, Center, Right };
struct TextLabel : OpaqueWidget
{
    std::string _text;
    NVGcolor _color;
    float _text_height;
    bool _bold;
    bool _clip;
    TextAlignment _align;
    std::function<std::string ()> getText;

    TextLabel() 
    :   _color(RampGray(G_90)),
        _text_height(12.f),
        _bold(true),
        _clip(false)
    {
        box.size.y = _text_height;
    }

    void fetch(std::function<std::string ()> get) { getText = get; }
    void color(const NVGcolor &new_color) { _color = new_color; }
    void clip() { _clip = true; }
    void noClip() { _clip = false; }
    void left() { _align = TextAlignment::Left; }
    void center() { _align = TextAlignment::Center; }
    void right() { _align = TextAlignment::Right; }
    void bold() { _bold = true; }
    void light() { _bold = false; }
    void size(float size) { _text_height = size; box.size.y = size; }

    void setStyle(float size, bool bold = true, TextAlignment alignment = TextAlignment::Left)
    {
        _text_height = size;
        _bold = bold;
        _align = alignment;
    }

    void draw(const DrawArgs& args) override
    {
        OpaqueWidget::draw(args);
        if (getText) {
            _text = getText();
        }
        if (_text.empty()) return;

        auto vg = args.vg;
        auto font = _bold ? GetPluginFontSemiBold() : GetPluginFontRegular();
        if (!font) return;

        nvgSave(vg);
        if (_clip) {
            nvgScissor(vg, box.pos.x, box.pos.y, box.size.x, box.size.y);
        }
        SetTextStyle(vg, font, _color, _text_height);
        switch (_align) {
        case TextAlignment::Left:
            nvgTextAlign(vg, NVG_ALIGN_TOP|NVG_ALIGN_LEFT);
            nvgText(vg, 0.f, 0.f, _text.c_str(), nullptr);
            break;
        case TextAlignment::Center:
            nvgTextAlign(vg, NVG_ALIGN_TOP|NVG_ALIGN_CENTER);
            nvgText(vg, box.size.x * .5, 0.f, _text.c_str(), nullptr);
            break;
        case TextAlignment::Right:
            nvgTextAlign(vg, NVG_ALIGN_TOP|NVG_ALIGN_RIGHT);
            nvgText(vg, box.size.x, 0.f, _text.c_str(), nullptr);
            break;
        }
        nvgRestore(vg);
    }
};

template<typename TWidget = TextLabel>
TWidget* createTextLabel(
    math::Vec pos,
    float width,
    TextAlignment alignment,
    float size,
    std::function<std::string ()> get,
    bool is_bold = true,
    const NVGcolor& color = RampGray(G_90),
    bool clip = false)
{
    TWidget* w = createWidget<TWidget>(pos);
    w->box.size.x = width;
    if (alignment == TextAlignment::Center) {
        w->box.pos.x -= width *.5f;
    }
    w->_color = color;
    w->_bold = is_bold;
    w->_align = alignment;
    w->_clip = clip;
    w->size(size);
    w->fetch(get);
    return w;
}

template<typename TWidget = TextLabel>
TWidget* createStaticTextLabel(
    math::Vec pos,
    float width,
    const char * text,
    TextAlignment alignment = TextAlignment::Center,
    float size = 12.f,
    const NVGcolor& color = RampGray(G_90)
    )
{
    TWidget* w = createWidget<TWidget>(pos);
    w->box.size.x = width;
    if (alignment == TextAlignment::Center) {
        w->box.pos.x -= width *.5f;
    }
    w->_text = text;
    w->_color = color;
    w->_align = alignment;
    w->size(size);
    return w;
}



} // namespace pachde