#pragma once

#include <cmath>
#include <array>
#include <algorithm>
#include <grrlib.h>

constexpr int STEP_DEG = 10, ANGLES = 360 / STEP_DEG, VERTICES = ANGLES + 4;

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

static const TrigLUT lut = {};

inline void roundedRectangle(const float x, const float y, const float width, const float height, const float radiusX,
                             const float radiusY, const uint32_t color, const bool filled)
{
    if (width <= 0.0f || height <= 0.0f) return;
    const float rx = std::clamp(radiusX, 0.0f, 0.5f * width), ry = std::clamp(radiusY, 0.0f, 0.5f * height),
                innerW = width - 2.0f * rx, innerH = height - 2.0f * ry, cx = x + 0.5f * width, cy = y + 0.5f * height,
                brCx = cx + 0.5f * innerW, brCy = cy + 0.5f * innerH;

    if (rx <= 0.0f || ry <= 0.0f)
    {
        GRRLIB_Rectangle(x, y, width, height, color, filled);
        return;
    }

    std::array<guVector, VERTICES> v = {};
    std::array<uint32_t, VERTICES> col = {};

    auto setV = [&](const int idx, const float vx, const float vy) noexcept
    {
        v[idx].x = vx;
        v[idx].y = vy;
        v[idx].z = 0.0f;
        col[idx] = color;
    };

    auto arc = [&](const int outStart, const int startIdx, const int endIdx, const float ccx, const float ccy) noexcept
    {
        int out = outStart;
        for (int i = startIdx; i <= endIdx; ++i, ++out) setV(out, lut.c[i] * rx + ccx, lut.s[i] * ry + ccy);
    };

    arc(0, 0, 8, brCx, brCy);
    setV(9, brCx - innerW, v[8].y);
    arc(10, 9, 17, brCx - innerW, brCy);
    setV(19, v[18].x, brCy - innerH);
    arc(20, 18, 26, brCx - innerW, brCy - innerH);
    setV(29, v[8].x, v[28].y);
    arc(30, 27, 35, brCx, brCy - innerH);
    setV(39, v[0].x, v[0].y);

    GRRLIB_GXEngine(v.data(), col.data(), VERTICES, filled ? GX_TRIANGLEFAN : GX_LINESTRIP);
}
