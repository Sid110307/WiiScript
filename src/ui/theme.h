#pragma once

#include <cstdint>

struct Rect
{
    float x = 0, y = 0, w = 0, h = 0;

    [[nodiscard]] bool contains(const float px, const float py) const
    {
        return px >= x && py >= y && px < x + w && py < y + h;
    }
};

struct Theme
{
    uint32_t bg = 0x101018FF;
    uint32_t panel = 0x1A1A22FF;
    uint32_t panelBorder = 0x2A2A36FF;

    uint32_t btn = 0x252533FF;
    uint32_t btnHover = 0x333346FF;
    uint32_t btnDown = 0x44445CFF;

    uint32_t accent = 0xFFCC00FF;
};

const Theme& theme();
