#pragma once

#include "./widget.h"
#include "./panel.h"
#include "./list.h"
#include "./scrollbar.h"

class ContextMenu : public Widget
{
public:
    struct Item
    {
        std::string label;
        std::function<void()> onClick;
    };

    float padding = 6.0f, minH = 20.0f, maxH = 220.0f, minW = 100.0f, maxW = 360.0f;
    bool open = false;

    ContextMenu()
    {
        visible = false;
        enabled = true;
        focusable = true;

        panel = addChild<Panel>(true);
        scroll = panel->addChild<ScrollView>();
        list = scroll->addChild<List>();
        scroll->content = list;
        scroll->barY = scroll->addChild<ScrollBar>(BoxDir::Vertical);
        scroll->barY->scrollAmount = list->rowH;

        list->onItemSelected = [this](const std::string&)
        {
            if (const int i = list ? list->selected : -1; i >= 0 && i < static_cast<int>(actions.size()))
            {
                const auto fn = actions[i];
                close();

                if (fn) fn();
            }
        };
    }

    void close()
    {
        open = false;
        visible = false;

        if (list)
        {
            list->selected = -1;
            list->items.clear();
        }
        if (scroll)
        {
            scroll->scrollX = 0.0f;
            scroll->scrollY = 0.0f;
        }

        actions.clear();
    }

    [[nodiscard]] bool isOpen() const { return open && visible; }

    void openAt(const float x, const float y, const std::vector<Item>& items, const float screenW, const float screenH)
    {
        if (!list || !scroll || !panel) return;

        actions.clear();
        list->items.clear();
        list->items.reserve(items.size());
        actions.reserve(items.size());

        for (const auto& item : items)
        {
            list->items.push_back(item.label);
            actions.push_back(item.onClick);
        }

        const float w = std::clamp(list->contentWidth() + padding * 2.0f, minW, maxW),
                    h = std::clamp(list->contentHeight() + padding * 2.0f, minH, maxH);
        bounds = {std::clamp(x, 0.0f, screenW - w), std::clamp(y, 0.0f, screenH - h), w, h};

        scroll->scrollX = scroll->scrollY = 0.0f;
        list->selectFirstSelectable();

        open = true;
        visible = true;
    }

protected:
    void onUpdate(double) override
    {
        if (!visible) return;

        panel->bounds = {0, 0, bounds.w, bounds.h};
        scroll->bounds = panel->bounds.inset(padding);

        if (scroll->barY) scroll->barY->scrollAmount = list->rowH;
    }

private:
    Panel* panel = nullptr;
    ScrollView* scroll = nullptr;
    List* list = nullptr;

    std::vector<std::function<void()>> actions;
};
