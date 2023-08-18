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

        explicit TabWidget(const std::string& title) : selected(false), title(title) {
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
                CenterText(vg, box.size.x * 0.5f, 9.f, title.c_str(), nullptr );
            }
        }
    };

    std::vector<TabWidget*> tabs;

    int getSelectedTab() {
        return std::distance(tabs.begin(), std::find_if(tabs.begin(), tabs.end(), [](const TabWidget* p){ return p->selected; }));
    }

    void addTab(const std::string& title) {
        auto tab = new TabWidget(title);
        addChild(tab);
        tabs.push_back(tab);
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

    void onButton(const ButtonEvent& e) override
    {
        OpaqueWidget::onButton(e);
        if (e.isConsumed()) {
            unsigned n = 0;
            for (auto p: tabs) {
                if (p->box.contains(e.pos)) break;
                 ++n;
            }
            if (n < tabs.size()) {
                selectTab(n);
            }
        }
    }

    void draw(const DrawArgs& args) override
    {
        OpaqueWidget::draw(args);
        BoxRect(args.vg, 0, 0, box.size.x -1.f, box.size.y, RampGray(G_40), 0.75f);
        if (tabs.size() > 1) {
            float width = box.size.x / tabs.size();
            for (float x = width; x < box.size.x; x += width) {
                Line(args.vg, x, 0, x, box.size.y, RampGray(G_40), 0.75f);
            }
        }

    }

};

}
#endif
