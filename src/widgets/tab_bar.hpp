#pragma once
#ifndef TAB_BAR_HPP_INCLUDED
#define TAB_BAR_HPP_INCLUDED
#include "../colors.hpp"
#include "../plugin.hpp"
#include "../text.hpp"
#include "symbol_widget.hpp"
#include "tip_widget.hpp"
namespace pachde {

enum class PresetTab : uint8_t {
    User,
    Favorite,
    System,

    First = User,
    Last = System
};

using Symbol = SymbolWidget::Symbol;

struct TabBarWidget : OpaqueWidget
{
    template <typename TBase = OpaqueWidget>
    struct TTabWidget : TBase //TipWidget
    {
        bool selected;
        bool hovered;
        bool extra;
        std::string title;
        PresetTab kind;
        SymbolWidget * sym;
        std::function<void(Menu *)> addMenu;

        explicit TTabWidget(const std::string& title, PresetTab tab_kind)
        :   selected(false),
            hovered(false),
            extra(false),
            title(title),
            kind(tab_kind),
            sym(nullptr),
            addMenu(nullptr)
        {
            if (kind == PresetTab::User)
            {
                sym = createWidget<SymbolWidget>(Vec(6.5f, 4.f));
                sym->setSymbol(Symbol::Person);
                TBase::addChild(sym);
            }
        }

        void setAppendMenu(std::function<void(Menu *)> menufn) {
            addMenu = menufn;
        }

        void onHover(const HoverEvent& e) override {
            TBase::onHover(e);
            auto inset = this->box.shrink(Vec(1.f, .75f));
            hovered = inset.contains(e.pos.plus(this->box.pos));
            if (this->hovered) {
                e.consume(this);
            }
        }

        void onLeave(const LeaveEvent& e) override {
            TBase::onLeave(e);
            hovered = false;
        }

        void createContextMenu() {
            if (addMenu) {
                ui::Menu* menu = createMenu();
                addMenu(menu);
            }
        }

        void onButton(const ButtonEvent& e) override
        {
            if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT && (e.mods & RACK_MOD_MASK) == 0) {
                e.consume(this);
                createContextMenu();
            }
            TBase::onButton(e);
        }

        void draw(const DrawArgs& args) override
        {
            auto vg = args.vg;
            FillRect(vg, .5f, -.75f, this->box.size.x -.75f, this->box.size.y + .5f,
                selected ? RampGray(G_20)
                    : hovered ? RampGray(G_30)
                    : RampGray(G_15));

            if (PresetTab::User == kind && sym) {
                sym->setVisible(selected);
            }

            TBase::draw(args);

            auto font = GetPluginFontRegular();
            auto lineco = RampGray(G_30);
            if (selected) {
                switch (kind) {
                case PresetTab::User:
                    //FillPerson(vg, 6.5f, 4.f, 8.f, nvgHSL(210.f/360.f, .5f, .5f));
                    break;
                case PresetTab::Favorite:
                    FillHeart(vg, 6.5f, 4.f, 6.f, PORT_PINK);
                    if (this->extra) {
                        SetTextStyle(vg, font, RampGray(G_90), 9.f);
                        nvgText(vg, this->box.size.x - 10.f, 9.f, "\u0192", nullptr);
                    }
                    break;
                case PresetTab::System:
                    Circle(vg, 6.5f, 6.f, 2.f, preset_name_color);
                    break;
                }
                Line(vg, .5f, -.5f, this->box.size.x-.5f, -.5f, lineco, .75f); // top
                Line(vg, .5f, -.5f, .5f, this->box.size.y, lineco, .75f); // left
                Line(vg, this->box.size.x-.5f, -.5f, this->box.size.x-.5f, this->box.size.y, lineco, .75f); // right
            } else {
                Line(vg, 0.f, this->box.size.y-.5f, this->box.size.x, this->box.size.y-.5f, lineco, .75f);
            }
            SetTextStyle(vg, font, selected ? preset_name_color : RampGray(G_90), 12.f);
            CenterText(vg, this->box.size.x * 0.5f, 9.f, title.c_str(), nullptr );
        }
    };
    using TabWidget = TTabWidget<OpaqueWidget>;

    std::vector<TabWidget*> tabs;

    PresetTab getSelectedTab() {
        return static_cast<PresetTab>(std::distance(tabs.begin(), std::find_if(tabs.begin(), tabs.end(), [](const TabWidget* p){ return p->selected; })));
    }

    void addTab(const std::string& title, PresetTab kind, std::function<void(Menu *)> menufn) {
        auto tab = new TabWidget(title, kind);
        tab->setAppendMenu(menufn);
        addChild(tab);
        tabs.push_back(tab);
    }

    void layout()
    {
        float width = floor(box.size.x / tabs.size());
        float x = 0;
        float y = 0;
        for (auto tab : tabs) {
            tab->setPosition(Vec(x, y));
            tab->setSize(Vec(width, box.size.y));
            x += width;
        }
    }

    void setExtra(PresetTab which, bool is_extra) {
        tabs[static_cast<size_t>(which)]->extra = is_extra;
    }

    void selectTab(PresetTab select) {
        uint8_t n = 0;
        for (auto tab : tabs) {
            tab->selected = (n == static_cast<uint8_t>(select));
            ++n;
        }
    }

    void onButton(const ButtonEvent& e) override
    {
        if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT && (e.mods & RACK_MOD_MASK) == 0)
        {
            unsigned n = 0;
            for (auto p: tabs) {
                if (p->box.contains(e.pos)) {
                    break;
                }
                 ++n;
            }
            if (n < tabs.size()) {
                selectTab(static_cast<PresetTab>(n));
            }
            e.consume(this);
            return;
        }
        OpaqueWidget::onButton(e);
    }
};

}
#endif
