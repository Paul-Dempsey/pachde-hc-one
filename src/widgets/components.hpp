#pragma once
#ifndef COMPONENTS_HPP_INCLUDED
#define COMPONENTS_HPP_INCLUDED
#include "../colors.hpp"
#include "../plugin.hpp"
#include "tip_widget.hpp"

//#define IMPLEMENT_DRAW_PERSON
namespace pachde {
constexpr const float ONE_HP = 15.0f;
constexpr const float TWO_HP = 30.0f;
constexpr const float HALF_HP = 7.5f;

void DrawLogo(NVGcontext * vg, float x, float y, NVGcolor fill, float scale = 1.0f);
void FillHeart(NVGcontext* vg, float x, float y, float square, NVGcolor fill);
void StrokeHeart(NVGcontext* vg, float x, float y, float square, NVGcolor stroke, float stroke_width = 1.0f);
#if defined IMPLEMENT_DRAW_PERSON
void StrokePerson(NVGcontext* vg, float x, float y, float square, NVGcolor stroke, float stroke_width = 1.0f);
void FillPerson(NVGcontext* vg, float x, float y, float square, NVGcolor fill);
#endif
void DrawKnobTrack(NVGcontext* vg, const Knob * w, float track_radius, float track_width, const NVGcolor& color);

// struct USB_A_Port : app::SvgPort {

// 	USB_A_Port() {
// 		SvgPort::setSvg(Svg::load(asset::plugin(pluginInstance, "res/USB-A-blue.svg")));
// 	}

//     void guideColor(const NVGcolor& co) {
//         if (nullptr == sw || nullptr == sw->svg || nullptr == sw->svg->handle || nullptr == sw->svg->handle->shapes) {
//             return;
//         }
//         PackedColor color = toPacked(co);
//         auto filled = 0;
//         for (auto shape = this->sw->svg->handle->shapes; shape && (filled < 2); shape = shape->next) {
//             auto d = shape->id[0];
//             if ((0 == shape->id[1]) && (d == 'b' || d == 't')) {
//                 shape->fill.color = color;
//                 ++filled;
//             }
//         }
//         this->fb->dirty = true;
//     }
// };

struct PickMidi : app::MidiButton {
    rack::ui::Tooltip* tip = nullptr;
    std::string text;

    virtual ~PickMidi() {
        if (tip) delete tip;
        tip = nullptr;       
    }
    void describe(std::string description) {
        text = description;
    }
    void createTip() {
        if (!rack::settings::tooltips) return;
        if (tip) return;
        tip = new Tooltip;
        tip->text = text;
        APP->scene->addChild(tip);
    }
    void destroyTip() {
        if (!tip) return;
        auto t = tip;
        tip = nullptr;
	    APP->scene->removeChild(t);
        delete t;
    }
    void onEnter(const EnterEvent& e) override {
        createTip();
    }
    void onLeave(const LeaveEvent& e) override {
        destroyTip();
    }

    PickMidi() {
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/MIDI.svg")));
		shadow->opacity = 0.0;
    }
};


struct PDSmallButton : app::SvgSwitch {
    PDSmallButton() {
        addFrame(Svg::load(asset::plugin(pluginInstance, "res/TinyPush_up.svg")));
        addFrame(Svg::load(asset::plugin(pluginInstance, "res/TinyPush_down.svg")));
    }
};

template <typename TLight>
struct PDLightButton : app::SvgSwitch {
    app::ModuleLightWidget* light;

    PDLightButton() {
        addFrame(Svg::load(asset::plugin(pluginInstance, "res/TinyPush_up.svg")));
        addFrame(Svg::load(asset::plugin(pluginInstance, "res/TinyPush_down.svg")));
        light = new TLight;
        // Move center of light to center of box
        light->box.pos = box.size.div(2).minus(light->box.size.div(2));
        addChild(light);
    }

    app::ModuleLightWidget* getLight() {
        return light;
    }
};
template <typename TLight>
using SmallLightButton = PDLightButton<TLight>;

template <typename TLight>
struct PDLightLatch : PDLightButton<TLight> {
    PDLightLatch() {
        this->momentary = false;
        this->latch = true;
    }
};

// -- square -------------------------------
struct PDSmallSquareButton : app::SvgSwitch
{
    std::function<void()> handler;

    PDSmallSquareButton() {
        addFrame(Svg::load(asset::plugin(pluginInstance, "res/TinyPush_square_up.svg")));
        addFrame(Svg::load(asset::plugin(pluginInstance, "res/TinyPush_square_down.svg")));
    }

    void onClick(std::function<void()> callback) {
        handler = callback;
    }

};

template <typename TLight>
struct PDSquareLightButton : app::SvgSwitch
{
    app::ModuleLightWidget* light;

    PDSquareLightButton() {
        addFrame(Svg::load(asset::plugin(pluginInstance, "res/TinyPush_square_up.svg")));
        addFrame(Svg::load(asset::plugin(pluginInstance, "res/TinyPush_square_down.svg")));
        light = new TLight;
        // Move center of light to center of box
        light->box.pos = box.size.div(2).minus(light->box.size.div(2));
        addChild(light);
    }

    app::ModuleLightWidget* getLight() {
        return light;
    }
};
template <typename TLight>
using SmallSquareLightButton = PDSquareLightButton<TLight>;

template <typename TLight>
struct PDSquareLightLatch : PDSquareLightButton<TLight>
{
    PDSquareLightLatch() {
        this->momentary = false;
        this->latch = true;
    }
};

template<typename TSvg>
struct TKnob: rack::RoundKnob
{
    bool clickStepValue = true;
    float stepIncrement = 1.f;
    float stepModIncrement  = 10.f;
    bool key_modified = false;
    bool key_modified2 = false;

    TKnob()
    {
        setImage();
    }

    std::function<void(void)> clickHandler;
    void onClick(std::function<void(void)> callback)
    {
        clickHandler = callback;
    }
    void continuous() {
        clickStepValue = false;
    }
    void clicky(float step, float mod_step) {
        clickStepValue = true;
        stepIncrement = step;
        stepModIncrement = mod_step;
    }

    void onHoverKey(const HoverKeyEvent& e) override {
        rack::RoundKnob::onHoverKey(e);
        key_modified = (e.mods & RACK_MOD_MASK) & RACK_MOD_CTRL;
        key_modified2 = (e.mods & RACK_MOD_MASK) & GLFW_MOD_SHIFT;
    }

    void onAction(const ActionEvent& e) override {
        rack::RoundKnob::onAction(e);
        if (clickHandler) {
            clickHandler();
        } else if (clickStepValue) {
            auto pq = getParamQuantity();
            if (pq) {
                float value = pq->getValue();
                if (!key_modified) {
                    auto lim = pq->getMaxValue();
                    if (value == lim) {
                        value = pq->getMinValue();
                    } else {
                        value = value + (key_modified2 ? stepModIncrement : stepIncrement);
                        if (value > lim) {
                            value = lim;
                        }
                    }
                } else {
                    auto lim = pq->getMinValue();
                    if (value == lim) {
                        value = pq->getMaxValue();
                    } else {
                        value = value - (key_modified2 ? stepModIncrement : stepIncrement);
                        if (value < pq->getMinValue()) {
                            value = lim;
                        }
                    }
                }
                pq->setValue(value);
            }
        }
        if (fb) {
            fb->setDirty(true);
        }
    }

    void setImage() {
        if (bg && bg->svg) return;
        if (TSvg::isSystemSvg()) {
            setSvg(Svg::load(asset::system(TSvg::knob())));
            bg->setSvg(Svg::load(asset::system(TSvg::background())));
        } else {
            setSvg(Svg::load(asset::plugin(pluginInstance, TSvg::knob())));
            bg->setSvg(Svg::load(asset::plugin(pluginInstance, TSvg::background())));
        }
        if (fb) {
            fb->setDirty(true);
        }
    }
};

struct SmallBlackKnobSvg {
    static bool isSystemSvg() { return true; }
    static std::string knob() {
        return "res/ComponentLibrary/RoundSmallBlackKnob.svg";
    }
    static std::string background() {
        return "res/ComponentLibrary/RoundSmallBlackKnob_bg.svg";
    }
};
using SmallBlackKnob = TKnob<SmallBlackKnobSvg>;

template<typename TSvg>
struct TButton : SvgButton
{
    bool key_ctrl;
    bool key_shift;
    std::function<void(bool, bool)> handler;
    TipHolder * tip_holder;

    TButton() 
    :   key_ctrl(false),
        key_shift(false),
        handler(nullptr),
        tip_holder(nullptr)
    {
        setImage();
    }

    virtual ~TButton()
    {
        if (tip_holder) {
            delete tip_holder;
            tip_holder = nullptr;
        }
    }

    void describe(std::string text)
    {
        if (!tip_holder) {
            tip_holder = new TipHolder();
        }
        tip_holder->setText(text);
    }

    void setHandler(std::function<void(bool,bool)> callback)
    {
        handler = callback;
    }

    void setImage()
    {
        if (frames.size() > 0) return;
        if (TSvg::isSystemSvg()) {
            addFrame(Svg::load(asset::system(TSvg::up())));
            addFrame(Svg::load(asset::system(TSvg::down())));
        } else {
            addFrame(Svg::load(asset::plugin(pluginInstance, TSvg::up())));
            addFrame(Svg::load(asset::plugin(pluginInstance, TSvg::down())));
        }
        if (fb) {
            fb->setDirty(true);
        }
    }
    void destroyTip() {
        if (tip_holder) { tip_holder->destroyTip(); }
    }
    void createTip() {
        if (tip_holder) { tip_holder->createTip(); }
    }

    void onEnter(const EnterEvent& e) override {
        SvgButton::onEnter(e);
        createTip();
    }
    void onLeave(const LeaveEvent& e) override {
        SvgButton::onLeave(e);
        destroyTip();
    }
    void onDragLeave(const DragLeaveEvent& e) override {
        SvgButton::onDragLeave(e);
        destroyTip();
    }
    void onDragEnd(const DragEndEvent& e) override
    {
        SvgButton::onDragEnd(e);
        destroyTip();
    }

    void onHoverKey(const HoverKeyEvent& e) override
    {
        SvgButton::onHoverKey(e);
        key_ctrl = (e.mods & RACK_MOD_MASK) & RACK_MOD_CTRL;
        key_shift = (e.mods & RACK_MOD_MASK) & GLFW_MOD_SHIFT;
    }

    void onAction(const ActionEvent& e) override
    {
        destroyTip();
        if (handler) {
            handler(key_ctrl, key_shift);
        } else {
            SvgButton::onAction(e);
        }
    }

};

struct SmallSquareButtonSvg {
    static bool isSystemSvg() { return false; }
    static std::string up() {
        return "res/TinyPush_square_up.svg";
    }
    static std::string down() {
        return "res/TinyPush_square_down.svg";
    }
};
using SmallSquareButton = TButton<SmallSquareButtonSvg>;

}
#endif