#pragma once

#include "./widget.h"
#include "../theme.h"

#include <grrlib.h>

class Panel : public Widget
{
public:
    bool drawBorder = true;

protected:
    void onDraw() const override
    {
        GRRLIB_Rectangle(bounds.x, bounds.y, bounds.w, bounds.h, theme().panel, true);
        if (drawBorder) GRRLIB_Rectangle(bounds.x, bounds.y, bounds.w, bounds.h, theme().panelBorder, false);
    }
};
