#pragma once

#include "./widget.h"
#include "../theme.h"

#include <string>
#include <grrlib.h>

class List : public Widget
{
public:
    explicit List(std::vector<std::string> items = {},
                  std::function<void(const std::string&)> onItemSelected = nullptr)
        : items(std::move(items)), onItemSelected(std::move(onItemSelected))
    {
        focusable = true;
    }

    std::vector<std::string> items;
    std::function<void(const std::string& item)> onItemSelected;
    std::function<void(float x, float y)> onContextMenu;

    int selected = -1;
    bool hovered = false;
    float rowH = 22.0f, viewportTopY = 0.0f, viewportH = 0.0f;

    [[nodiscard]] float contentWidth() const
    {
        float maxW = 0.0f;
        if (const Font* f = getFont(); f)
            for (const auto& item : items) maxW = std::max(maxW, f->textWidth(item) + rowH);

        return maxW;
    }

    [[nodiscard]] float contentHeight() const { return rowH * static_cast<float>(items.size()); }

    void selectFirstSelectable()
    {
        for (int i = 0; i < static_cast<int>(items.size()); ++i)
            if (!items[i].empty())
            {
                selected = i;
                return;
            }
        selected = -1;
    }

    bool onEvent(const Input::InputEvent& e) override
    {
        if (!visible || !enabled) return Widget::onEvent(e);
        const Rect r = worldBounds();

        if (e.type == Input::InputEvent::Type::Pointer)
        {
            hovered = e.pointer.valid && r.contains(e.pointer.x, e.pointer.y);
            return Widget::onEvent(e);
        }

        if (e.type != Input::InputEvent::Type::KeyDown) return Widget::onEvent(e);
        if (!(focused || hovered)) return Widget::onEvent(e);

        const int n = static_cast<int>(items.size());
        if (n <= 0)
        {
            selected = -1;
            return true;
        }
        clampSelection();

        if (e.key == Input::Key::A)
        {
            if ((static_cast<uint8_t>(e.mods) & static_cast<uint8_t>(Input::KeyMods::ContextMenu)) != 0)
            {
                if (e.pointer.valid && onContextMenu) onContextMenu(e.pointer.x, e.pointer.y);
                return true;
            }

            if (e.pointer.valid)
            {
                const int i = std::clamp(static_cast<int>((e.pointer.y - r.y) / rowH), 0, n - 1);
                selected = i;

                if (isSelectable(i) && onItemSelected) onItemSelected(items[selected]);
                else clampSelection();
            }
            else if (focused)
            {
                clampSelection();
                if (isSelectable(selected) && onItemSelected) onItemSelected(items[selected]);
            }
            else selected = -1;

            return true;
        }

        if (e.key == Input::Key::Up)
        {
            selectNext(-1);
            return true;
        }

        if (e.key == Input::Key::Down)
        {
            selectNext(1);
            return true;
        }

        return Widget::onEvent(e);
    }

protected:
    void onDraw() const override
    {
        const Rect r = worldBounds();

        roundedRectangle(r.x, r.y, r.w, r.h, radiusX, radiusY, theme().panel, true);
        roundedRectangle(r.x, r.y, r.w, r.h, radiusX, radiusY, theme().panelBorder, false);

        int first = 0, last = static_cast<int>(items.size());
        if (viewportH > 0.0f)
        {
            first = std::max(0, static_cast<int>(std::floor((viewportTopY - r.y) / rowH)));
            last = std::min(static_cast<int>(items.size()),
                            static_cast<int>(std::ceil((viewportTopY + viewportH - r.y) / rowH)) + 1);
        }

        for (int i = first; i < last; ++i)
        {
            const float y = r.y + static_cast<float>(i) * rowH;
            std::string text = items[i];

            if (text.empty())
            {
                GRRLIB_Line(r.x + 5, y + rowH / 2, r.x + r.w - 5, y + rowH / 2, theme().textDisabled);
                continue;
            }

            GRRLIB_Rectangle(r.x, y, r.w, rowH, i == selected ? theme().selection : theme().btn, true);
            GRRLIB_Line(r.x, y + rowH, r.x + r.w, y + rowH, theme().panelBorder);

            if (const Font* f = getFont(); f)
                if (const float textW = f->textWidth(text); textW > r.w - 20)
                {
                    while (!text.empty() && f->textWidth(text + "...") > r.w - 20) text.pop_back();
                    text += "...";
                }

            if (const Font* f = getFont(); f && !text.empty())
                f->drawText(text, r.x + 10, y + (rowH - f->textHeight()) / 2, theme().text);
        }
    }

    [[nodiscard]] bool isSelectable(const int i) const
    {
        return i >= 0 && i < static_cast<int>(items.size()) && !items[i].empty();
    }

    void clampSelection()
    {
        const int n = static_cast<int>(items.size());
        if (n <= 0)
        {
            selected = -1;
            return;
        }

        selected = std::clamp(selected, -1, n - 1);
        if (selected == -1 || isSelectable(selected)) return;

        int up = selected, down = selected;
        for (int guard = 0; guard < n; ++guard)
        {
            up = (up - 1 + n) % n;
            down = (down + 1) % n;

            if (isSelectable(up))
            {
                selected = up;
                return;
            }
            if (isSelectable(down))
            {
                selected = down;
                return;
            }
        }
        selected = -1;
    }

    void selectNext(const int dir)
    {
        const int n = static_cast<int>(items.size());
        if (n <= 0)
        {
            selected = -1;
            return;
        }

        selected = selected < 0 || selected >= n ? dir > 0 ? 0 : n - 1 : (selected + dir + n) % n;
        for (int i = 0; i < n; ++i)
        {
            if (isSelectable(selected)) return;
            selected = (selected + dir + n) % n;
        }

        selected = -1;
    }
};
