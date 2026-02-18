#pragma once

#include <cmath>
#include <array>
#include <algorithm>
#include <grrlib.h>


inline void roundRectangle(const float x, const float y, const float width, const float height, const float radiusX,
                           const float radiusY, const uint32_t color, const bool filled)
{
    if (width == 0u || height == 0u) return;
    constexpr int kStepDeg = 10, kAngles = 360 / kStepDeg, kTotalVertices = 40;

    struct TrigLUT
    {
        std::array<float, kAngles> c = {}, s = {};

        TrigLUT()
        {
            constexpr float degToRad = M_PI / 180.0f;
            for (int i = 0; i < kAngles; ++i)
            {
                const float a = static_cast<float>(i * kStepDeg) * degToRad;

                c[i] = std::cos(a);
                s[i] = std::sin(a);
            }
        }
    };

    static const TrigLUT lut = {};
    const float rx = std::clamp(radiusX, 0.0f, 0.5f * width), ry = std::clamp(radiusY, 0.0f, 0.5f * height),
                innerW = width - 2.0f * rx, innerH = height - 2.0f * ry, cx = x + 0.5f * width, cy = y + 0.5f * height,
                brCx = cx + 0.5f * innerW, brCy = cy + 0.5f * innerH;

    std::array<guVector, kTotalVertices> v = {};
    std::array<uint32_t, kTotalVertices> col = {};

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

    GRRLIB_GXEngine(v.data(), col.data(), kTotalVertices, filled ? GX_TRIANGLEFAN : GX_LINESTRIP);
}
