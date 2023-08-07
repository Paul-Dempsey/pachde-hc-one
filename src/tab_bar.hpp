#pragma once
#ifndef TAB_BAR_HPP_INCLUDED
#define TAB_BAR_HPP_INCLUDED
#include "plugin.hpp"
#include "tip_widget.hpp"
#include "text.hpp"
#include "colors.hpp"

namespace pachde {

struct TabBarWidget : OpaqueWidget
{
    struct TabWidget : TipWidget
    {
        bool selected;
        std::string title;

        TabWidget(std::string title) : selected(false), title(title) {
            tip_text = format_string("%s presets", title.c_str());
        }

        void draw(const DrawArgs& args) override
        {
            TipWidget::draw(args);
            auto vg = args.vg;
            if (selected) {
                Circle(vg, 5.f, 6.f, 3.f, preset_name_color);
            }
            auto font = GetPluginFontRegular();
            if (FontOk(font)) {
                SetTextStyle(vg, font, selected ? preset_name_color : RampGray(G_90), 12.f);
                CenterText(vg, box.size.x * 0.5f, 10.f, title.c_str(), nullptr );
            }
        }
    };

    std::vector<TabWidget*> tabs;

    void addTab(std::string title) {
        auto tab = new TabWidget(title);
        addChild(tab);
        tabs.push_back(tab);
        layout();
    }

    void layout()
    {
        float width = box.size.x / tabs.size();
        float x = 0;
        float y = 0;
        for (auto tab : tabs) {
            tab->setPosition(Vec(x, y));
            tab->setSize(Vec(width, box.size.y));
            x += width;
        }
    }

    void selectTab(int select) {
        int n = 0;
        for (auto tab : tabs) {
            tab->selected = (n == select);
            ++n;
        }
    }

    void draw(const DrawArgs& args) override
    {
        OpaqueWidget::draw(args);
        BoxRect(args.vg, 0, 0, box.size.x, box.size.y, RampGray(G_50), 0.75f);
        if (tabs.size() > 1) {
            float width = box.size.x / tabs.size();
            for (float x = width; x < box.size.x; x += width) {
                Line(args.vg, x, 0, x, box.size.y, RampGray(G_50), 0.75f);
            }
        }

    }

};

}
#endif
