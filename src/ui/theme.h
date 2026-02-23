#pragma once

#include <cstdint>

struct Theme
{
    uint32_t bg = 0x101018FF;
    uint32_t panel = 0x1A1A22FF;
    uint32_t panelBorder = 0x2A2A36FF;
    uint32_t modalBackdrop = 0x00000088;

    uint32_t btn = 0x252533FF;
    uint32_t btnHover = 0x333346FF;
    uint32_t btnDown = 0x44445CFF;
    uint32_t btnDisabled = 0x1C1C26FF;

    uint32_t text = 0xE0E0E0FF;
    uint32_t textDisabled = 0x707080FF;

    uint32_t accent = 0xFFCC00FF;
    uint32_t selection = 0x2F4F9BFF;
    uint32_t focus = 0xFFCC00FF;

    uint32_t scrollTrack = 0x22222CFF;
    uint32_t scrollThumb = 0x3A3A4EFF;
    uint32_t scrollHover = 0x4A4A66FF;
    uint32_t scrollActive = 0x5A5A7AFF;
};

const Theme& theme();
