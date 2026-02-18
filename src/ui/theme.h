#pragma once

#include <cstdint>
#include "../gfx/font.h"

struct Rect
{
    float x = 0, y = 0, w = 0, h = 0;

    [[nodiscard]] bool contains(const float px, const float py) const
    {
        return px >= x && py >= y && px < x + w && py < y + h;
    }

    static Rect empty() { return {0, 0, 0, 0}; }
    [[nodiscard]] Rect inset(const float all) const { return {x + all, y + all, w - 2 * all, h - 2 * all}; }

    [[nodiscard]] Rect inset(const float l, const float t, const float r, const float b) const
    {
        return {x + l, y + t, w - (l + r), h - (t + b)};
    }

    Rect takeLeft(const float width)
    {
        const float ww = width < 0 ? 0 : width > w ? w : width;
        const Rect out = {x, y, ww, h};
        x += ww;
        w -= ww;

        return out;
    }

    Rect takeRight(const float width)
    {
        const float ww = width < 0 ? 0 : width > w ? w : width;
        const Rect out = {x + w - ww, y, ww, h};
        w -= ww;

        return out;
    }

    Rect takeTop(const float height)
    {
        const float hh = height < 0 ? 0 : height > h ? h : height;
        const Rect out = {x, y, w, hh};
        y += hh;
        h -= hh;

        return out;
    }

    Rect takeBottom(const float height)
    {
        const float hh = height < 0 ? 0 : height > h ? h : height;
        const Rect out = {x, y + h - hh, w, hh};
        h -= hh;

        return out;
    }

    Rect takeRowItem(const float width, const float height, const float gap = 0.0f)
    {
        const Rect out = {x, y, width, height};
        x += width + gap;
        w -= width + gap;

        return out;
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
    uint32_t text = 0xE0E0E0FF;
};

const Font& font();
const Theme& theme();
