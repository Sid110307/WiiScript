#pragma once

constexpr float KW = 30.0f, KH = 24.0f, GAP = 4.0f, KX = 1.0f, KY = 2.0f, CAPS_W = KW * 1.75f + GAP * 0.75f,
                SHIFT_W = KW * 2.25f + GAP * 1.25f;

struct KeyboardKey
{
    bool valid, control;
    float x, y, w, h;
    const char *val, *shiftVal;
};

constexpr KeyboardKey keyboardKeys[] = {
    {false, false, KX + 0 * (KW + GAP), KY + 0 * (KH + GAP), KW, KH, "1", "!"},
    {false, false, KX + 1 * (KW + GAP), KY + 0 * (KH + GAP), KW, KH, "2", "@"},
    {false, false, KX + 2 * (KW + GAP), KY + 0 * (KH + GAP), KW, KH, "3", "#"},
    {false, false, KX + 3 * (KW + GAP), KY + 0 * (KH + GAP), KW, KH, "4", "$"},
    {false, false, KX + 4 * (KW + GAP), KY + 0 * (KH + GAP), KW, KH, "5", "%"},
    {false, false, KX + 5 * (KW + GAP), KY + 0 * (KH + GAP), KW, KH, "6", "^"},
    {false, false, KX + 6 * (KW + GAP), KY + 0 * (KH + GAP), KW, KH, "7", "&"},
    {false, false, KX + 7 * (KW + GAP), KY + 0 * (KH + GAP), KW, KH, "8", "*"},
    {false, false, KX + 8 * (KW + GAP), KY + 0 * (KH + GAP), KW, KH, "9", "("},
    {false, false, KX + 9 * (KW + GAP), KY + 0 * (KH + GAP), KW, KH, "0", ")"},

    {false, false, KX + 0.5f * (KW + GAP) + 0 * (KW + GAP), KY + 1 * (KH + GAP), KW, KH, "q", "Q"},
    {false, false, KX + 0.5f * (KW + GAP) + 1 * (KW + GAP), KY + 1 * (KH + GAP), KW, KH, "w", "W"},
    {false, false, KX + 0.5f * (KW + GAP) + 2 * (KW + GAP), KY + 1 * (KH + GAP), KW, KH, "e", "E"},
    {false, false, KX + 0.5f * (KW + GAP) + 3 * (KW + GAP), KY + 1 * (KH + GAP), KW, KH, "r", "R"},
    {false, false, KX + 0.5f * (KW + GAP) + 4 * (KW + GAP), KY + 1 * (KH + GAP), KW, KH, "t", "T"},
    {false, false, KX + 0.5f * (KW + GAP) + 5 * (KW + GAP), KY + 1 * (KH + GAP), KW, KH, "y", "Y"},
    {false, false, KX + 0.5f * (KW + GAP) + 6 * (KW + GAP), KY + 1 * (KH + GAP), KW, KH, "u", "U"},
    {false, false, KX + 0.5f * (KW + GAP) + 7 * (KW + GAP), KY + 1 * (KH + GAP), KW, KH, "i", "I"},
    {false, false, KX + 0.5f * (KW + GAP) + 8 * (KW + GAP), KY + 1 * (KH + GAP), KW, KH, "o", "O"},
    {false, false, KX + 0.5f * (KW + GAP) + 9 * (KW + GAP), KY + 1 * (KH + GAP), KW, KH, "p", "P"},

    {false, false, KX + CAPS_W + GAP + 0 * (KW + GAP), KY + 2 * (KH + GAP), KW, KH, "a", "A"},
    {false, false, KX + CAPS_W + GAP + 1 * (KW + GAP), KY + 2 * (KH + GAP), KW, KH, "s", "S"},
    {false, false, KX + CAPS_W + GAP + 2 * (KW + GAP), KY + 2 * (KH + GAP), KW, KH, "d", "D"},
    {false, false, KX + CAPS_W + GAP + 3 * (KW + GAP), KY + 2 * (KH + GAP), KW, KH, "f", "F"},
    {false, false, KX + CAPS_W + GAP + 4 * (KW + GAP), KY + 2 * (KH + GAP), KW, KH, "g", "G"},
    {false, false, KX + CAPS_W + GAP + 5 * (KW + GAP), KY + 2 * (KH + GAP), KW, KH, "h", "H"},
    {false, false, KX + CAPS_W + GAP + 6 * (KW + GAP), KY + 2 * (KH + GAP), KW, KH, "j", "J"},
    {false, false, KX + CAPS_W + GAP + 7 * (KW + GAP), KY + 2 * (KH + GAP), KW, KH, "k", "K"},
    {false, false, KX + CAPS_W + GAP + 8 * (KW + GAP), KY + 2 * (KH + GAP), KW, KH, "l", "L"},

    {false, false, KX + SHIFT_W + GAP + 0 * (KW + GAP), KY + 3 * (KH + GAP), KW, KH, "z", "Z"},
    {false, false, KX + SHIFT_W + GAP + 1 * (KW + GAP), KY + 3 * (KH + GAP), KW, KH, "x", "X"},
    {false, false, KX + SHIFT_W + GAP + 2 * (KW + GAP), KY + 3 * (KH + GAP), KW, KH, "c", "C"},
    {false, false, KX + SHIFT_W + GAP + 3 * (KW + GAP), KY + 3 * (KH + GAP), KW, KH, "v", "V"},
    {false, false, KX + SHIFT_W + GAP + 4 * (KW + GAP), KY + 3 * (KH + GAP), KW, KH, "b", "B"},
    {false, false, KX + SHIFT_W + GAP + 5 * (KW + GAP), KY + 3 * (KH + GAP), KW, KH, "n", "N"},
    {false, false, KX + SHIFT_W + GAP + 6 * (KW + GAP), KY + 3 * (KH + GAP), KW, KH, "m", "M"},

    {true, false, KX + 3 * (KW + GAP), KY + 4 * (KH + GAP), KW * 6 + GAP * 5, KH, " ", " "},
    {true, false, KX + 10 * (KW + GAP), KY + 0 * (KH + GAP), KW * 2 + GAP, KH, "BACK", "BACK"},
    {true, false, KX + CAPS_W + GAP + 9 * (KW + GAP), KY + 2 * (KH + GAP), KW * 2.25f + GAP * 1.25f, KH, "ENTER","ENTER"},
    {true, false, KX, KY + 3 * (KH + GAP), SHIFT_W, KH, "SHIFT", "SHIFT"},
    {true, false, KX, KY + 2 * (KH + GAP), CAPS_W, KH, "CAPS", "CAPS"},
};
