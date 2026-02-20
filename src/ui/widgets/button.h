#pragma once

#include "./widget.h"
#include "../theme.h"
#include "../../gfx/drawing.h"

#include <functional>

class Button : public Widget
{
public:
    explicit Button(std::string text = "", std::function<void()> onClick = nullptr)
        : text(std::move(text)), onClick(std::move(onClick)) { focusableOverride = true; }

    std::string text;
    std::function<void()> onClick;
    const float paddingX = 12.0f, paddingY = 6.0f;
    bool hovered = false, pressed = false;

    bool onEvent(const Input::InputEvent& e) override
    {
        const Rect r = worldBounds();
        if ((!visible || !enabled) && !(e.type == Input::InputEvent::Type::KeyUp && e.key == Input::Key::A && pressed))
            return false;

        if (e.type == Input::InputEvent::Type::Pointer)
        {
            hovered = e.pointer.valid && r.contains(e.pointer.x, e.pointer.y);
            return false;
        }

        if (e.type == Input::InputEvent::Type::KeyDown && e.key == Input::Key::A)
        {
            hovered = e.pointer.valid && r.contains(e.pointer.x, e.pointer.y);
            if (hovered || focused)
            {
                pressed = true;
                return true;
            }

            return false;
        }

        if (e.type == Input::InputEvent::Type::KeyUp && e.key == Input::Key::A)
        {
            const bool wasPressed = pressed;
            pressed = false;
            hovered = e.pointer.valid && r.contains(e.pointer.x, e.pointer.y);

            if (wasPressed && (hovered || focused) && onClick) onClick();
            return wasPressed;
        }

        return false;
    }

protected:
    void onDraw() const override
    {
        const Rect r = worldBounds();
        const uint32_t textColor = enabled ? theme().text : theme().textDisabled,
                       btnColor = pressed
                                      ? theme().btnDown
                                      : hovered
                                      ? theme().btnHover
                                      : enabled
                                      ? theme().btn
                                      : theme().btnDisabled;

        roundedRectangle(r.x, r.y, r.w, r.h, radiusX, radiusY, btnColor, true);
        roundedRectangle(r.x, r.y, r.w, r.h, radiusX, radiusY, theme().panelBorder, false);

        if (const Font* f = getFont(); f && !text.empty())
            f->drawText(text, r.x + (r.w - f->textWidth(text)) / 2, r.y + (r.h - f->textHeight()) / 2, textColor);
    }

    void onUpdate(double) override
    {
        if (const Font* f = getFont(); f && !text.empty())
        {
            if (layout.fixedWidth < 0.0f) bounds.w = f->textWidth(text) + paddingX * 2.0f;
            if (layout.fixedHeight < 0.0f) bounds.h = f->textHeight() + paddingY * 2.0f;
        }
    }
};
