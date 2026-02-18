#pragma once

#include "./widget.h"
#include "../theme.h"

#include <string>
#include <grrlib.h>

class List : public Widget
{
public:
    std::vector<std::string> items;
    int selected = -1;
    bool hovered = false;
    float rowH = 22.0f;

    bool onEvent(const Input::InputEvent& e) override
    {
        if (!visible || !enabled) return false;
        if (e.type == Input::InputEvent::Type::PointerMove)
        {
            hovered = e.pointer.valid && bounds.contains(e.pointer.x, e.pointer.y);
            return false;
        }

        if (e.type == Input::InputEvent::Type::KeyDown && hovered)
        {
            if (e.key == Input::Key::A)
            {
                if (e.pointer.valid)
                {
                    if (const int index = static_cast<int>((e.pointer.y - bounds.y) / rowH); index >= 0 && index <
                        static_cast<int>(items.size()))
                        selected = index;
                }
                return true;
            }

            if (e.key == Input::Key::Up)
            {
                if (selected > 0) selected--;
                return true;
            }

            if (e.key == Input::Key::Down)
            {
                if (selected < static_cast<int>(items.size()) - 1) selected++;
                return true;
            }
        }

        return false;
    }

    [[nodiscard]] bool isFocusable() const override { return enabled; }

protected:
    void onDraw() const override
    {
        GRRLIB_Rectangle(bounds.x, bounds.y, bounds.w, bounds.h, theme().panel, true);
        GRRLIB_Rectangle(bounds.x, bounds.y, bounds.w, bounds.h, theme().panelBorder, false);

        const int maxRows = static_cast<int>(bounds.h / rowH);
        const int n = static_cast<int>(items.size());
        const int shown = std::min(n, maxRows);

        for (int i = 0; i < shown; ++i)
        {
            const float y = bounds.y + static_cast<float>(i) * rowH;
            const uint32_t col = i == selected ? theme().accent : theme().btn;

            if (i == selected) GRRLIB_Rectangle(bounds.x + 2, y + 2, bounds.w - 4, rowH - 4, col, true);
            GRRLIB_Line(bounds.x, y + rowH, bounds.x + bounds.w, y + rowH, theme().panelBorder);
        }
    }
};
