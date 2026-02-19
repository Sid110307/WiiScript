#pragma once

constexpr float KW = 24.0f, KH = 21.0f, GAP = 3.0f, KX = 0.0f, KY = 0.0f, CAPS_W = KW * 1.75f + GAP * 0.75f,
                SHIFT_W = KW * 2.25f + GAP * 1.25f;

struct KeyboardKey
{
    float x, y, w, h;
    const char *val, *shiftVal;
};

constexpr KeyboardKey keyboardKeys[] = {
    {KX + 0 * (KW + GAP), KY, KW, KH, "1", "!"},
    {KX + 1 * (KW + GAP), KY, KW, KH, "2", "@"},
    {KX + 2 * (KW + GAP), KY, KW, KH, "3", "#"},
    {KX + 3 * (KW + GAP), KY, KW, KH, "4", "$"},
    {KX + 4 * (KW + GAP), KY, KW, KH, "5", "%"},
    {KX + 5 * (KW + GAP), KY, KW, KH, "6", "^"},
    {KX + 6 * (KW + GAP), KY, KW, KH, "7", "&"},
    {KX + 7 * (KW + GAP), KY, KW, KH, "8", "*"},
    {KX + 8 * (KW + GAP), KY, KW, KH, "9", "("},
    {KX + 9 * (KW + GAP), KY, KW, KH, "0", ")"},
    {KX + 10 * (KW + GAP), KY, KW, KH, "-", "_"},
    {KX + 11 * (KW + GAP), KY, KW, KH, "=", "+"},
    {KX + 12 * (KW + GAP), KY, KW * 2 + GAP, KH, "Backspace", "Backspace"},

    {KX, KY + 1 * (KH + GAP), KW * 1.5f + GAP * 0.5f, KH, "Tab", "Tab"},
    {KX + 0.5f * (KW + GAP) + 1 * (KW + GAP), KY + 1 * (KH + GAP), KW, KH, "q", "Q"},
    {KX + 0.5f * (KW + GAP) + 2 * (KW + GAP), KY + 1 * (KH + GAP), KW, KH, "w", "W"},
    {KX + 0.5f * (KW + GAP) + 3 * (KW + GAP), KY + 1 * (KH + GAP), KW, KH, "e", "E"},
    {KX + 0.5f * (KW + GAP) + 4 * (KW + GAP), KY + 1 * (KH + GAP), KW, KH, "r", "R"},
    {KX + 0.5f * (KW + GAP) + 5 * (KW + GAP), KY + 1 * (KH + GAP), KW, KH, "t", "T"},
    {KX + 0.5f * (KW + GAP) + 6 * (KW + GAP), KY + 1 * (KH + GAP), KW, KH, "y", "Y"},
    {KX + 0.5f * (KW + GAP) + 7 * (KW + GAP), KY + 1 * (KH + GAP), KW, KH, "u", "U"},
    {KX + 0.5f * (KW + GAP) + 8 * (KW + GAP), KY + 1 * (KH + GAP), KW, KH, "i", "I"},
    {KX + 0.5f * (KW + GAP) + 9 * (KW + GAP), KY + 1 * (KH + GAP), KW, KH, "o", "O"},
    {KX + 0.5f * (KW + GAP) + 10 * (KW + GAP), KY + 1 * (KH + GAP), KW, KH, "p", "P"},
    {KX + 0.5f * (KW + GAP) + 11 * (KW + GAP), KY + 1 * (KH + GAP), KW, KH, "[", "{"},
    {KX + 0.5f * (KW + GAP) + 12 * (KW + GAP), KY + 1 * (KH + GAP), KW, KH, "]", "}"},
    {KX + 0.5f * (KW + GAP) + 13 * (KW + GAP), KY + 1 * (KH + GAP), KW * 1.75f + GAP * 0.75f, KH, "\\", "|"},

    {KX, KY + 2 * (KH + GAP), CAPS_W, KH, "Caps", "Caps"},
    {KX + CAPS_W + GAP + 0 * (KW + GAP), KY + 2 * (KH + GAP), KW, KH, "a", "A"},
    {KX + CAPS_W + GAP + 1 * (KW + GAP), KY + 2 * (KH + GAP), KW, KH, "s", "S"},
    {KX + CAPS_W + GAP + 2 * (KW + GAP), KY + 2 * (KH + GAP), KW, KH, "d", "D"},
    {KX + CAPS_W + GAP + 3 * (KW + GAP), KY + 2 * (KH + GAP), KW, KH, "f", "F"},
    {KX + CAPS_W + GAP + 4 * (KW + GAP), KY + 2 * (KH + GAP), KW, KH, "g", "G"},
    {KX + CAPS_W + GAP + 5 * (KW + GAP), KY + 2 * (KH + GAP), KW, KH, "h", "H"},
    {KX + CAPS_W + GAP + 6 * (KW + GAP), KY + 2 * (KH + GAP), KW, KH, "j", "J"},
    {KX + CAPS_W + GAP + 7 * (KW + GAP), KY + 2 * (KH + GAP), KW, KH, "k", "K"},
    {KX + CAPS_W + GAP + 8 * (KW + GAP), KY + 2 * (KH + GAP), KW, KH, "l", "L"},
    {KX + CAPS_W + GAP + 9 * (KW + GAP), KY + 2 * (KH + GAP), KW, KH, ";", ":"},
    {KX + CAPS_W + GAP + 10 * (KW + GAP), KY + 2 * (KH + GAP), KW, KH, "'", "\""},
    {KX + CAPS_W + GAP + 11 * (KW + GAP), KY + 2 * (KH + GAP), KW * 2.25f + GAP * 1.25f, KH, "Enter", "Enter"},

    {KX, KY + 3 * (KH + GAP), SHIFT_W, KH, "Shift", "Shift"},
    {KX + SHIFT_W + GAP + 0 * (KW + GAP), KY + 3 * (KH + GAP), KW, KH, "z", "Z"},
    {KX + SHIFT_W + GAP + 1 * (KW + GAP), KY + 3 * (KH + GAP), KW, KH, "x", "X"},
    {KX + SHIFT_W + GAP + 2 * (KW + GAP), KY + 3 * (KH + GAP), KW, KH, "c", "C"},
    {KX + SHIFT_W + GAP + 3 * (KW + GAP), KY + 3 * (KH + GAP), KW, KH, "v", "V"},
    {KX + SHIFT_W + GAP + 4 * (KW + GAP), KY + 3 * (KH + GAP), KW, KH, "b", "B"},
    {KX + SHIFT_W + GAP + 5 * (KW + GAP), KY + 3 * (KH + GAP), KW, KH, "n", "N"},
    {KX + SHIFT_W + GAP + 6 * (KW + GAP), KY + 3 * (KH + GAP), KW, KH, "m", "M"},
    {KX + SHIFT_W + GAP + 7 * (KW + GAP), KY + 3 * (KH + GAP), KW, KH, ",", "<"},
    {KX + SHIFT_W + GAP + 8 * (KW + GAP), KY + 3 * (KH + GAP), KW, KH, ".", ">"},
    {KX + SHIFT_W + GAP + 9 * (KW + GAP), KY + 3 * (KH + GAP), KW, KH, "/", "?"},

    {KX + 3 * (KW + GAP), KY + 4 * (KH + GAP), KW * 7 + GAP * 5, KH, " ", " "},
};
