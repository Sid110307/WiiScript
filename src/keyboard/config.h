#pragma once

enum class KeyAction { Text, Backspace, Tab, Enter, Caps, Shift };

struct KeyboardKey
{
    const char *val = "", *shiftVal = "";
    float unit = 1.0f;
    KeyAction action = KeyAction::Text;
    float x = 0.0f, y = 0.0f, w = 0.0f, h = 0.0f;
};

struct RowSpec
{
    const KeyboardKey* keys = nullptr;
    size_t count = 0;
    float xOffset = 0.0f;
};

inline constexpr KeyboardKey row1[] = {
    {"`", "~"},
    {"1", "!"},
    {"2", "@"},
    {"3", "#"},
    {"4", "$"},
    {"5", "%"},
    {"6", "^"},
    {"7", "&"},
    {"8", "*"},
    {"9", "("},
    {"0", ")"},
    {"-", "_"},
    {"=", "+"},
    {"Bksp", "Bksp", 2.25f, KeyAction::Backspace}
};

inline constexpr KeyboardKey row2[] = {
    {"Tab", "Tab", 1.5f, KeyAction::Tab},
    {"q", "Q"},
    {"w", "W"},
    {"e", "E"},
    {"r", "R"},
    {"t", "T"},
    {"y", "Y"},
    {"u", "U"},
    {"i", "I"},
    {"o", "O"},
    {"p", "P"},
    {"[", "{"},
    {"]", "}"},
    {"\\", "|", 1.75f, KeyAction::Text}
};

inline constexpr KeyboardKey row3[] = {
    {"Caps", "Caps", 1.75f, KeyAction::Caps},
    {"a", "A"},
    {"s", "S"},
    {"d", "D"},
    {"f", "F"},
    {"g", "G"},
    {"h", "H"},
    {"j", "J"},
    {"k", "K"},
    {"l", "L"},
    {";", ":"},
    {"'", "\""},
    {"Enter", "Enter", 2.5f, KeyAction::Enter}
};

inline constexpr KeyboardKey row4[] = {
    {"Shift", "Shift", 2.25f, KeyAction::Shift},
    {"z", "Z"},
    {"x", "X"},
    {"c", "C"},
    {"v", "V"},
    {"b", "B"},
    {"n", "N"},
    {"m", "M"},
    {",", "<"},
    {".", ">"},
    {"/", "?"},
    {"Shift", "Shift", 3.0f, KeyAction::Shift}
};

inline constexpr KeyboardKey row5[] = {
    {" ", " ", 6.25f, KeyAction::Text}
};

inline constexpr RowSpec keyboardLayout[] = {
    {row1, sizeof(row1) / sizeof(KeyboardKey), 0},
    {row2, sizeof(row2) / sizeof(KeyboardKey), 0.0f},
    {row3, sizeof(row3) / sizeof(KeyboardKey), 0.0f},
    {row4, sizeof(row4) / sizeof(KeyboardKey), 0.0f},
    {row5, sizeof(row5) / sizeof(KeyboardKey), 3.75f}
};

inline constexpr size_t keyboardLayoutRows = sizeof(keyboardLayout) / sizeof(RowSpec);
inline constexpr float unitWidth = 24.0f, unitHeight = 21.0f, unitGap = 3.0f, startX = 0.0f, startY = 0.0f;
