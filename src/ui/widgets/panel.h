#pragma once

#include "./widget.h"
#include "../theme.h"

class Panel : public Widget
{
public:
    explicit Panel(const bool drawBorder = true) : drawBorder(drawBorder)
    {
    }

    bool drawBorder = true;

protected:
    void onDraw() const override
    {
        const Rect r = worldBounds();

        roundedRectangle(r.x, r.y, r.w, r.h, radiusX, radiusY, theme().panel, true);
        if (drawBorder) roundedRectangle(r.x, r.y, r.w, r.h, radiusX, radiusY, theme().panelBorder, false);
    }
};
