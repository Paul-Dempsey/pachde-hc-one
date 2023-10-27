#include "vert_slider.hpp"
#include "../misc.hpp"

namespace pachde {

void SliderOnSelectKeyImpl(SvgSlider * self, const rack::widget::Widget::SelectKeyEvent &e)
{
    if (e.action == GLFW_PRESS || e.action == GLFW_REPEAT)
    {
        auto pq = self->getParamQuantity();
        if (pq)
        {
            bool plain = (e.action != GLFW_REPEAT) && ((e.mods & RACK_MOD_MASK) == 0);
            bool shift = e.mods & GLFW_MOD_SHIFT;
            // bool ctrl = e.mods & GLFW_MOD_CONTROL;
            switch (e.key)
            {
            case GLFW_KEY_0:
                pq->setValue(pq->getMaxValue());
                e.consume(self);
                break;
            case GLFW_KEY_1: case GLFW_KEY_2: case GLFW_KEY_3: case GLFW_KEY_4:
            case GLFW_KEY_5: case GLFW_KEY_6: case GLFW_KEY_7: case GLFW_KEY_8: case GLFW_KEY_9:
                pq->setValue(14.1f * (e.key - GLFW_KEY_1));
                e.consume(self);
                break;

            case GLFW_KEY_HOME:
                if (plain) {
                    pq->setValue(pq->getMaxValue());
                    e.consume(self);
                }
                break;
            case GLFW_KEY_END:
                if (plain) {
                    pq->setValue(pq->getMinValue());
                    e.consume(self);
                }
                break;
            case GLFW_KEY_UP:
                pq->setValue(pq->getValue() + (shift ? 10.f : 1.f));
                e.consume(self);
                break;
            case GLFW_KEY_DOWN:
                pq->setValue(pq->getValue() - (shift ? 10.f : 1.f));
                e.consume(self);
                break;
            case GLFW_KEY_PAGE_UP:
                pq->setValue(pq->getValue() + 10.f);
                e.consume(self);
                break;
            case GLFW_KEY_PAGE_DOWN:
                pq->setValue(pq->getValue() - 10.f);
                e.consume(self);
                break;
            }
        }
    }
    if (e.isConsumed())
        return;
    self->SvgSlider::onSelectKey(e);
}

void SliderOnHoverScrollImpl(SvgSlider * self, const rack::widget::Widget::HoverScrollEvent & e)
{
    auto pq = self->getParamQuantity();
    if (pq) {
        auto dx = e.scrollDelta;
        auto mods = APP->window->getMods();
        if (dx.y < 0.f) {
            pq->setValue(pq->getValue() - ((mods & GLFW_MOD_SHIFT) ? 10.f : 1.f));
        } else if (dx.y > 0.f) {
            pq->setValue(pq->getValue() + ((mods & GLFW_MOD_SHIFT) ? 10.f : 1.f));
        }
        e.consume(self);
    } else {
        self->SvgSlider::onHoverScroll(e);
    }

}

void SliderOnButtonImpl(SvgSlider * self, const rack::widget::Widget::ButtonEvent &e, float shrinky)
{
    if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT && (e.mods & RACK_MOD_MASK) == 0) {
        e.consume(self);
        auto pq = self->getParamQuantity();
        if (pq) {
            float v = self->box.size.y - shrinky - e.pos.y;
            v = rescale(v, 0.f, self->box.size.y, pq->getMinValue(), pq->getMaxValue());
            pq->setValue(v);
        }
    } else {
        self->SvgSlider::onButton(e);
    }    
}

//const NVGcolor slider_bg = nvgRGB(0x2d, 0x5d, 0x86); //#2d5d86
const NVGcolor value_bg = nvgRGB(0x8f, 0x7a, 0x25); //#8f7a25

void VerticalSlider::SliderValueFill::draw(const DrawArgs& args)
{
    auto break_pos = reference->box.pos.y + 4;
    FillRect(args.vg, 1.5f, break_pos, box.size.x - 3.f, box.size.y - break_pos, value_bg);
}

VerticalSlider::VerticalSlider()
{
    setBackgroundSvg(Svg::load(asset::plugin(pluginInstance, "res/VertSlider_bg.svg")));
    setHandleSvg(Svg::load(asset::plugin(pluginInstance, "res/VertSliderThumb.svg")));
    setHandlePosCentered(Vec(10.f, 199.f), Vec(10.f, 4.f));
    auto fill = new SliderValueFill(handle);
    fill->box.size = box.size;
    fb->addChildBelow(fill, handle);
    handle->box.pos.y = 198.f;
}

void VerticalSlider::onButton(const ButtonEvent &e)
{
    SliderOnButtonImpl(this, e, 2.f);
}

void VerticalSlider::onHover(const HoverEvent &e)
{
    SvgSlider::onHover(e);
    e.consume(this);
}
void VerticalSlider::onEnter(const EnterEvent& e)  {
    APP->event->setSelectedWidget(this);
    e.consume(this);
    SvgSlider::onEnter(e);
}
void VerticalSlider::onLeave(const LeaveEvent& e)  {
    APP->event->setSelectedWidget(nullptr);
    SvgSlider::onLeave(e);
}

void VerticalSlider::onSelectKey(const SelectKeyEvent& e) 
{
    SliderOnSelectKeyImpl(this, e);
}

void VerticalSlider::onHoverScroll(const HoverScrollEvent & e)
{
    SliderOnHoverScrollImpl(this, e);
}

// ==== MinMaxSlider ===========================================

MinMaxSlider::MinMaxSlider()
: is_min(true)
{
    box.size.x = 7.5f;
    box.size.y = 200.f;
}

void MinMaxSlider::makeUi()
{
    setBackgroundSvg(Svg::load(asset::plugin(pluginInstance, "res/MinMaxSlider_bg.svg")));
    setHandleSvg(Svg::load(asset::plugin(pluginInstance, is_min ? "res/ThumbMin.svg" : "res/ThumbMax.svg")));
    setHandlePosCentered(Vec(3.75f, 197.f), Vec(3.75, 3.f));
}

void MinMaxSlider::onButton(const ButtonEvent &e)
{
    SliderOnButtonImpl(this, e, 2.f);
}

void MinMaxSlider::onHover(const HoverEvent &e)
{
    SvgSlider::onHover(e);
    e.consume(this);
}

void MinMaxSlider::onEnter(const EnterEvent &e)
{
    APP->event->setSelectedWidget(this);
    e.consume(this);
    SvgSlider::onEnter(e);
}

void MinMaxSlider::onLeave(const LeaveEvent &e)
{
    APP->event->setSelectedWidget(nullptr);
    SvgSlider::onLeave(e);
}


void MinMaxSlider::onSelectKey(const SelectKeyEvent &e)
{
    SliderOnSelectKeyImpl(this, e);
}

void MinMaxSlider::onHoverScroll(const HoverScrollEvent &e)
{
    SliderOnHoverScrollImpl(this, e);
}

}