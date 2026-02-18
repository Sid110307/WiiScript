#pragma once

#include "./widget.h"
#include "../theme.h"

#include <string>
#include <grrlib.h>

class List : public Widget
{
public:
    std::vector<std::string> items;
    size_t selected = -1;
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

        if (e.type == Input::InputEvent::Type::KeyDown && hovered)
        {
            if (e.key == Input::Key::A)
            {
                if (e.pointer.valid)
                    if (const auto index = static_cast<size_t>((e.pointer.y - r.y) / rowH); index < items.size())
                        selected = index;
                return true;
            }

            if (e.key == Input::Key::Up)
            {
                if (selected > 0) selected--;
                return true;
            }

            if (e.key == Input::Key::Down)
            {
                if (selected < items.size() - 1) selected++;
                return true;
            }
        }

        return false;
    }

    [[nodiscard]] bool isFocusable() const override { return enabled; }

protected:
    void onDraw() const override
    {
        const Rect r = worldBounds();

        roundRectangle(r.x, r.y, r.w, r.h, radiusX, radiusY, theme().panel, true);
        roundRectangle(r.x, r.y, r.w, r.h, radiusX, radiusY, theme().panelBorder, false);

        for (size_t i = 0; i < std::min(items.size(), static_cast<size_t>(r.h / rowH)); ++i)
        {
            const float y = r.y + static_cast<float>(i) * rowH;
            const uint32_t col = i == selected ? theme().accent : theme().btn;

            if (i == selected) GRRLIB_Rectangle(r.x + 2, y + 2, r.w - 4, rowH - 4, col, true);
            GRRLIB_Line(r.x, y + rowH, r.x + r.w, y + rowH, theme().panelBorder);

            if (const Font* f = getFont(); f && f->isValid() && !items[i].empty())
            {
                std::string text = items[i];
                if (f->textWidth(items[i]) > r.w - 4)
                {
                    while (!text.empty() && f->textWidth(text + "...") > r.w - 4) text.pop_back();
                    text += "...";
                }

                f->drawText(text, r.x + 2, y + (rowH - f->textHeight()) / 2, theme().text);
            }
        }
    }
};
