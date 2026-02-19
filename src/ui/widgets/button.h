#pragma once

#include "./widget.h"
#include "../theme.h"
#include "../../gfx/drawing.h"

#include <functional>

class Button : public Widget
{
public:
    explicit Button(std::string text = "", std::function<void()> onClick = nullptr)
        : text(std::move(text)), onClick(std::move(onClick))
    {
    }

    std::string text;
    std::function<void()> onClick;
    bool hovered = false, pressed = false;

    bool onEvent(const Input::InputEvent& e) override
    {
        if ((!visible || !enabled) && !(e.type == Input::InputEvent::Type::KeyUp && e.key == Input::Key::A && pressed))
            return false;
        if (e.type == Input::InputEvent::Type::PointerMove)
        {
            hovered = e.pointer.valid && worldBounds().contains(e.pointer.x, e.pointer.y);
            return false;
        }

        if (e.type == Input::InputEvent::Type::KeyDown && e.key == Input::Key::A)
            if (hovered)
            {
                pressed = true;
                return true;
            }

        if (e.type == Input::InputEvent::Type::KeyUp && e.key == Input::Key::A)
            if (pressed)
            {
                pressed = false;
                if (hovered && onClick) onClick();

                return true;
            }

        return false;
    }

    [[nodiscard]] bool isFocusable() const override { return visible && enabled; }

protected:
    void onDraw() const override
    {
        const Rect r = worldBounds();

        uint32_t col = theme().btn;
        if (pressed) col = theme().btnDown;
        else if (hovered) col = theme().btnHover;

        roundedRectangle(r.x, r.y, r.w, r.h, radiusX, radiusY, col, true);
        roundedRectangle(r.x, r.y, r.w, r.h, radiusX, radiusY, theme().panelBorder, false);

        if (const Font* f = getFont(); f && f->isValid() && !text.empty())
            f->drawText(text, r.x + (r.w - f->textWidth(text)) / 2, r.y + (r.h - f->textHeight()) / 2, theme().text);
    }
};
