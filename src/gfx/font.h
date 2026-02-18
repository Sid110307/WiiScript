#pragma once

#include <string>
#include <grrlib.h>

class Font
{
public:
    ~Font();

    bool load(const std::string& path, int size);
    void drawText(const std::string& text, float x, float y, uint32_t color, float scale = 1.0f) const;

    [[nodiscard]] float textWidth(const std::string& text, float scale = 1.0f) const;
    [[nodiscard]] float textHeight(float scale = 1.0f) const;
    [[nodiscard]] bool isValid() const;

private:
    GRRLIB_ttfFont* font = nullptr;
    int fontSize = 16;
};
