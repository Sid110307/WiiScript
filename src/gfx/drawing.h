#pragma once

#include <cmath>
#include <array>
#include <algorithm>

#include <grrlib.h>

constexpr int STEP_DEG = 10, ANGLES = 360 / STEP_DEG, VERTICES = ANGLES + 4;
static_assert(90 % STEP_DEG == 0, "STEP_DEG must divide 90 evenly");

struct Rect
{
    float x = 0, y = 0, w = 0, h = 0;
    static Rect empty() { return {0, 0, 0, 0}; }

    bool operator==(const Rect& other) const
    {
        return x == other.x && y == other.y && w == other.w && h == other.h;
    }

    bool operator!=(const Rect& other) const { return !(*this == other); }

    [[nodiscard]] bool contains(const float px, const float py) const
    {
        return px >= x && py >= y && px < x + w && py < y + h;
    }

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

struct TrigLUT
{
    std::array<float, ANGLES> c = {}, s = {};

    TrigLUT()
    {
        constexpr float degToRad = M_PI / 180.0f;
        for (int i = 0; i < ANGLES; ++i)
        {
            const float a = static_cast<float>(i * STEP_DEG) * degToRad;

            c[i] = std::cos(a);
            s[i] = std::sin(a);
        }
    }
};

inline void roundedRectangle(const float x, const float y, const float width, const float height, const float radiusX,
                             const float radiusY, const uint32_t color, const bool filled)
{
    if (width <= 0.0f || height <= 0.0f) return;
    const float rx = std::clamp(radiusX, 0.0f, 0.5f * width), ry = std::clamp(radiusY, 0.0f, 0.5f * height),
                ww = width - 2.0f * rx, hh = height - 2.0f * ry;

    if (rx <= 0.0f || ry <= 0.0f)
    {
        GRRLIB_Rectangle(x, y, width, height, color, filled);
        return;
    }

    static const TrigLUT lut = {};
    std::array<guVector, VERTICES> v = {};
    std::array<uint32_t, VERTICES> col = {};

    auto setV = [&](const int i, const float vx, const float vy) noexcept
    {
        v[i].x = vx;
        v[i].y = vy;
        v[i].z = 0.0f;
        col[i] = color;
    };

    auto arc = [&](const int outStart, const int startIndex, const int endIndex, const float ccx,
                   const float ccy) noexcept
    {
        int out = outStart;
        for (int i = startIndex; i <= endIndex; ++i, ++out) setV(out, lut.c[i] * rx + ccx, lut.s[i] * ry + ccy);
    };

    arc(0, 0, ANGLES / 4 - 1, x + 0.5f * width + 0.5f * ww, y + 0.5f * height + 0.5f * hh);
    setV(ANGLES / 4, x + 0.5f * width + 0.5f * ww - ww, v[ANGLES / 4 - 1].y);
    arc(ANGLES / 4 + 1, ANGLES / 4, ANGLES / 2 - 1, x + 0.5f * width + 0.5f * ww - ww, y + 0.5f * height + 0.5f * hh);
    setV(ANGLES / 2 + 1, v[ANGLES / 2].x, y + 0.5f * height + 0.5f * hh - hh);
    arc(ANGLES / 2 + 2, ANGLES / 2, 3 * ANGLES / 4 - 1, x + 0.5f * width + 0.5f * ww - ww,
        y + 0.5f * height + 0.5f * hh - hh);
    setV(3 * ANGLES / 4 + 2, x + 0.5f * width + 0.5f * ww, v[3 * ANGLES / 4 + 1].y);
    arc(3 * ANGLES / 4 + 3, 3 * ANGLES / 4, ANGLES - 1, x + 0.5f * width + 0.5f * ww,
        y + 0.5f * height + 0.5f * hh - hh);
    setV(VERTICES - 1, v[0].x, v[0].y);

    GRRLIB_GXEngine(v.data(), col.data(), VERTICES, filled ? GX_TRIANGLEFAN : GX_LINESTRIP);
}
