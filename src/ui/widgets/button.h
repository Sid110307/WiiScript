#pragma once

#include "./widget.h"
#include "../theme.h"

#include <functional>
#include <grrlib.h>

class Button : public Widget
{
public:
    std::function<void()> onClick;
    bool hovered = false, pressed = false;

    bool onEvent(const InputEvent& e) override
    {
        if ((!visible || !enabled) && !(e.type == InputEvent::Type::KeyUp && e.key == Key::A && pressed)) return false;
        if (e.type == InputEvent::Type::PointerMove)
        {
            hovered = e.pointer.valid && bounds.contains(e.pointer.x, e.pointer.y);
            return false;
        }

        if (e.type == InputEvent::Type::KeyDown && e.key == Key::A)
            if (hovered)
            {
                pressed = true;
                return true;
            }

        if (e.type == InputEvent::Type::KeyUp && e.key == Key::A)
            if (pressed)
            {
                pressed = false;
                if (hovered && onClick) onClick();

                return true;
            }

        return false;
    }

    [[nodiscard]] bool isFocusable() const override { return enabled; }

protected:
    void onDraw() const override
    {
        uint32_t col = theme().btn;
        if (pressed) col = theme().btnDown;
        else if (hovered) col = theme().btnHover;

        GRRLIB_Rectangle(bounds.x, bounds.y, bounds.w, bounds.h, col, true);
        GRRLIB_Rectangle(bounds.x, bounds.y, bounds.w, bounds.h, theme().panelBorder, false);
    }
};
