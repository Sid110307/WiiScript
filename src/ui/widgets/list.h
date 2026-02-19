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
    }

    std::vector<std::string> items;
    std::function<void(const std::string&)> onItemSelected;

    int selected = -1;
    bool hovered = false;
    float rowH = 22.0f;

    bool onEvent(const Input::InputEvent& e) override
    {
        const Rect r = worldBounds();

        if (!visible || !enabled) return false;
        if (e.type == Input::InputEvent::Type::PointerMove)
        {
            hovered = e.pointer.valid && r.contains(e.pointer.x, e.pointer.y);
            return false;
        }

        if (e.type != Input::InputEvent::Type::KeyDown) return false;
        if (!hovered) return false;

        const int n = static_cast<int>(items.size());
        if (n <= 0)
        {
            selected = -1;
            return true;
        }
        if (selected >= n) selected = n - 1;

        if (e.key == Input::Key::A)
        {
            if (e.pointer.valid) selected = std::clamp(static_cast<int>((e.pointer.y - r.y) / rowH), 0, n - 1);
            if (selected >= 0 && selected < n && onItemSelected) onItemSelected(items[selected]);

            return true;
        }

        if (e.key == Input::Key::Up)
        {
            selected = selected > 0 ? selected - 1 : n - 1;
            return true;
        }

        if (e.key == Input::Key::Down)
        {
            selected = selected >= 0 && selected < n - 1 ? selected + 1 : 0;
            return true;
        }

        return false;
    }

    [[nodiscard]] bool isFocusable() const override { return visible && enabled; }

protected:
    void onDraw() const override
    {
        const Rect r = worldBounds();

        roundedRectangle(r.x, r.y, r.w, r.h, radiusX, radiusY, theme().panel, true);
        roundedRectangle(r.x, r.y, r.w, r.h, radiusX, radiusY, theme().panelBorder, false);

        for (int i = 0; i < std::min(static_cast<int>(items.size()), static_cast<int>(r.h / rowH)); ++i)
        {
            const float y = r.y + static_cast<float>(i) * rowH;

            if (i == selected) GRRLIB_Rectangle(r.x, y, r.w, rowH, i == selected ? theme().accent : theme().btn, true);
            GRRLIB_Line(r.x, y + rowH, r.x + r.w, y + rowH, theme().panelBorder);

            if (const Font* f = getFont(); f && f->isValid() && !items[i].empty())
                f->drawText(items[i], r.x + 10, y + (rowH - f->textHeight()) / 2, theme().text);
        }
    }
};
