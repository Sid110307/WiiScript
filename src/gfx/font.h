#pragma once

#include <string>
#include <vector>
#include <memory>

#include <grrlib.h>

class Font
{
public:
    bool load(const std::string& path, int size);
    void drawText(std::string_view text, float x, float y, uint32_t color) const;

    [[nodiscard]] float textWidth(std::string_view text) const;
    [[nodiscard]] float textHeight() const;

private:
    using FontPtr = std::unique_ptr<GRRLIB_ttfFont, decltype(&GRRLIB_FreeTTF)>;

    FontPtr font = {nullptr, &GRRLIB_FreeTTF};
    std::vector<uint8_t> data;
    int fontSize = 16;
};
