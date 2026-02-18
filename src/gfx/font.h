#pragma once

#include <string>
#include <grrlib.h>

class Font
{
public:
    Font() = default;
    ~Font();

    bool load(const std::string& path, int size);
    void drawText(const std::string& text, float x, float y, uint32_t color = 0xFFFFFFFF, float scale = 1.0f) const;

    [[nodiscard]] float measureTextWidth(const std::string& text, float scale = 1.0f) const;
    [[nodiscard]] float lineHeight(float scale = 1.0f) const;
    [[nodiscard]] bool isValid() const;

private:
    GRRLIB_ttfFont* font = nullptr;
    int fontSize = 16;
};
