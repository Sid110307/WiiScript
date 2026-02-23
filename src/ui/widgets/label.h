#pragma once

#include "./widget.h"
#include "../theme.h"

class Label : public Widget
{
public:
    explicit Label(std::string text = "") : text(std::move(text))
    {
    }

    std::string text;
    float padding = 0.0f;

protected:
    void onDraw() const override
    {
        const Rect r = worldBounds().inset(padding);
        if (const Font* f = getFont(); f && !text.empty()) f->drawText(text, r.x, r.y, theme().text);
    }
};
