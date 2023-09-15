#pragma once
#include "plugin.hpp"
#include "colors.hpp"
using namespace ::rack;

namespace pachde {

inline std::shared_ptr<window::Font> GetPluginFontSemiBold()
{
    return APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/HankenGrotesk-SemiBold.ttf"));
}

inline std::shared_ptr<window::Font> GetPluginFontRegular()
{
    return APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/HankenGrotesk-Regular.ttf"));
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

struct BasicTextLabel: Widget
{
    std::string _text;
    NVGcolor _color;
    TextAlignment _align;
    float _text_height;
    bool _bold;

    BasicTextLabel()
    :   _color(RampGray(G_90)),
        _align(TextAlignment::Left),
        _text_height(12.f),
        _bold(true)
    {
    }
    std::string getText() { return _text; }
    void setPos(Vec pos) { box.pos = pos; }
    void setSize(Vec size) { box.size = size; }
    void text(std::string text) { _text = text; }
    void text_height(float height) { _text_height = height; box.size.y = height; }
    void style(float height, bool bold = true, TextAlignment alignment = TextAlignment::Left)
    {
        _text_height = height;
        _bold = bold;
        _align = alignment;
    }
    void color(const NVGcolor &new_color) { _color = new_color; }
    void render(const DrawArgs& args)
    {
        if (_text.empty()) return;

        auto vg = args.vg;
        auto font = _bold ? GetPluginFontSemiBold() : GetPluginFontRegular();
        if (!FontOk(font)) return;

        nvgSave(vg);
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

    void draw(const DrawArgs& args) override
    {
        Widget::draw(args);
        render(args);
    }
};

struct DynamicTextLabel : BasicTextLabel
{
    bool _bright = false;
    bool _lazy = false;
    bool _dirt = false;

    void bright(bool lit = true) { _bright = lit; }

    std::function<std::string ()> _getText;
    void setFetch(std::function<std::string ()> get)
    {
        _getText = get;
    }
    void setLazy() {
        _dirt = _lazy = true;
    }
    void modified(bool changed = true)
    {
        _dirt = changed;
    }
    void setText(std::string text)
    {
        BasicTextLabel::text(text);
        _dirt = true;
    }
    void refresh()
    {
        if (_getText) {
            if (!_lazy || _dirt) {
                BasicTextLabel::text(_getText());
                _dirt = false;
            }
        }
    }
    void drawLayer(const DrawArgs& args, int layer) override
    {
        Widget::drawLayer(args, layer);
        if (1 != layer || !_bright) return;
        refresh();
        BasicTextLabel::render(args);
    }
    void draw(const DrawArgs& args) override
    {
        Widget::draw(args);
        if (_bright) return;
        refresh();
        BasicTextLabel::render(args);
    }
};

inline DynamicTextLabel* createLazyDynamicTextLabel(
    Vec pos,
    Vec size, 
    std::function<std::string ()> get,
    float text_height = 12.f,
    bool bold = true,
    TextAlignment alignment = TextAlignment::Center,
    const NVGcolor &color = RampGray(G_90),
    bool bright = false)
{
    auto w = new DynamicTextLabel();
    w->setSize(size);
    w->setPos(pos);
    w->setLazy();
    w->setFetch(get);
    w->style(text_height, bold, alignment);
    if (alignment == TextAlignment::Center) {
        w->box.pos.x -= size.x * .5f;
    }
    w->color(color);
    if (bright) {
        w->bright();
    }
    return w;
}

struct StaticTextLabel: Widget
{
    FramebufferWidget* _fb = nullptr;
    BasicTextLabel* _label = nullptr;

    StaticTextLabel()
    {
        _label = new BasicTextLabel();
        _fb = new widget::FramebufferWidget;
        _fb->addChild(_label);
	    addChild(_fb);
        dirty();
    }
    std::string getText() {
        return _label ? _label->_text : "";
    }
    void setPos(Vec pos) {
        box.pos = pos;
    }
    void setSize(Vec size) {
        box.size = size;
        _fb->box.size = size;
        _label->box.size = size;
    }
    void modified(bool modified = true) {
        _fb->setDirty(modified);
    }
    void dirty() { _fb->setDirty(); }
    void text(const std::string & text) {
        _label->text(text);
        dirty();
    }
    void color(const NVGcolor &new_color) {
        _label->color(new_color);
        dirty();
    }
    void text_height(float height) {
        _label->text_height(height);
        dirty();
    }
    void style(float size, bool bold = true, TextAlignment alignment = TextAlignment::Left) {

        _label->style(size, bold, alignment);
        dirty();
    }
    void draw(const DrawArgs& args) override
    {
        Widget::draw(args);
#if defined VISIBLE_STATICTEXTLABEL_BOUNDS
        FillRect(args.vg, _fb->box.pos.x, _fb->box.pos.y, _fb->box.size.x, _fb->box.size.y, Overlay(GetStockColor(StockColor::Yellow), .20f));
#endif
    }
};

struct LazyDynamicLabel : StaticTextLabel
{
    std::function<std::string ()> _getText;
    
    LazyDynamicLabel(){}

    void modified(bool modified = true) {
        if (modified && _getText) {
            text(_getText());
        } else {
            StaticTextLabel::modified(modified);
        }
    }

    void setFetch(std::function<std::string ()> get, bool refreshable = false)
    {
        _getText = get;
    }
};

template<typename TWidget = StaticTextLabel>
TWidget* createStaticTextLabel(
    math::Vec pos,
    float width,
    std::string text,
    TextAlignment alignment = TextAlignment::Center,
    float text_height = 12.f,
    bool bold = true,
    const NVGcolor& color = RampGray(G_90)
    )
{
    TWidget* w = createWidget<TWidget>(pos);
    w->setSize(Vec(width, text_height));
    if (alignment == TextAlignment::Center) {
        w->setPos(Vec(w->box.pos.x - width*.5f, w->box.pos.y));
    }
    w->text(text);
    w->style(text_height, bold, alignment);
    w->color(color);
    return w;
}

template<typename TWidget = DynamicTextLabel>
TWidget* createDynamicLabel (
    math::Vec pos,
    float width,
    std::function<std::string ()> get,
    TextAlignment alignment = TextAlignment::Center,
    float text_height = 12.f,
    bool bold = true
    )
{
    TWidget* w = createWidget<TWidget>(pos);
    w->setSize(Vec(width, text_height));
    if (alignment == TextAlignment::Center) {
        w->setPos(Vec(w->box.pos.x -= width *.5f, w->box.pos.y));
    }
    w->style(text_height, bold, alignment);
    w->setFetch(get);
    return w;
}

} // namespace pachde