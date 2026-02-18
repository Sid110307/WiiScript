#pragma once

#include "./widget.h"
#include "../theme.h"

class Panel : public Widget
{
public:
    bool drawBorder = true;

protected:
    void onDraw() const override
    {
        const Rect r = worldBounds();

        roundRectangle(r.x, r.y, r.w, r.h, radiusX, radiusY, theme().panel, true);
        if (drawBorder) roundRectangle(r.x, r.y, r.w, r.h, radiusX, radiusY, theme().panelBorder, false);
    }
};
