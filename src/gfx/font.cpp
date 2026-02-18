#include "./font.h"
#include <cmath>

static int calculateFontSize(const float scale, const int fontSize)
{
    return std::max(8, static_cast<int>(std::roundf(scale * static_cast<float>(fontSize))));
}

Font::~Font()
{
    if (font)
    {
        GRRLIB_FreeTTF(font);
        font = nullptr;
    }
}

bool Font::load(const std::string& path, const int size)
{
    if (font) GRRLIB_FreeTTF(font);

    font = GRRLIB_LoadTTFFromFile(path.c_str());
    if (!font) return false;
    fontSize = size;

    return true;
}

void Font::drawText(const std::string& text, const float x, const float y, const uint32_t color,
                    const float scale) const
{
    if (!font) return;
    GRRLIB_PrintfTTF(static_cast<int>(x), static_cast<int>(y), font, text.c_str(), calculateFontSize(scale, fontSize),
                     color);
}

float Font::textWidth(const std::string& text, const float scale) const
{
    if (!font) return 0.0f;
    return static_cast<float>(GRRLIB_WidthTTF(font, text.c_str(), calculateFontSize(scale, fontSize)));
}

float Font::textHeight(const float scale) const { return static_cast<float>(calculateFontSize(scale, fontSize)); }
bool Font::isValid() const { return font != nullptr; }
