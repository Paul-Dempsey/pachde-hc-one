#pragma once
#ifndef DSP_WIDGET_HPP_INCLUDED
#define DSP_WIDGET_HPP_INCLUDED
#include "../colors.hpp"
namespace pachde {
using namespace ::rack;

constexpr const float DSP_BAR_HEIGHT = 10.f;
constexpr const float DSP_BAR_WIDTH = 2.5f;
constexpr const float DSP_WIDTH = DSP_BAR_WIDTH * 3.f + 5.f;
constexpr const float DSP_HEIGHT = DSP_BAR_HEIGHT + 3.f;

struct IDsp {
    virtual void set_dsp_ready(bool ready) = 0;
    virtual void set_dsp_value(int index, uint8_t value) = 0;
};

struct DspWidget : Widget, IDsp
{
    struct Inner : TransparentWidget
    {
        uint8_t dsp[3];
        bool ready;
        Inner() : ready(false) {
            sample_values();
        }

        void sample_values() {
            dsp[0] = 65;
            dsp[1] = 30;
            dsp[2] = 75;
        }

        void set_dsp_value(int index, uint8_t value)
        {
            if (index < 0 || index > 2) return;
            dsp[index] = value;
        }

        void drawDSP(NVGcontext* vg)
        {
            float x = 0.f;
            float y = 0.f;

            FillRect(vg, x, y, box.size.x, box.size.y, RampGray(G_30));
            if (!ready) {
                BoxRect(vg, x, y, box.size.x, box.size.y, green_light, .5f);
            }
            x += 1.5f;
            y += 1.5f;
            for (auto n = 0; n < 3; n++) {
                auto pct = dsp[n];
                auto co = pct < 85 ? green_light : red_light;
                auto bh = DSP_BAR_HEIGHT * (pct / 100.f);
                FillRect(vg, x, y + DSP_BAR_HEIGHT - bh, DSP_BAR_WIDTH, bh, co);
                x += DSP_BAR_WIDTH + 1;
            }
        }

        void draw(const DrawArgs& args) override {
            drawDSP(args.vg);
        }
    };

    FramebufferWidget* _fb;
    Inner * _dsp;

    DspWidget()
    {
        auto size = Vec(DSP_WIDTH, DSP_HEIGHT);
        box.size = size;

        _dsp = new Inner();
        _dsp->box.size = size;

        _fb = new widget::FramebufferWidget;
        _fb->box.size = size;
        _fb->addChild(_dsp);

	    addChild(_fb);
        dirty();
    }

    void set_dsp_ready(bool ready) override
    {
        _dsp->ready = ready;
        dirty();
    }
    void set_dsp_value(int index, uint8_t value) override
    {
        _dsp->set_dsp_value(index, value);
        dirty();
    }
    void modified(bool modified = true) {
        _fb->setDirty(modified);
    }
    void dirty() { _fb->setDirty(); }

};

}
#endif