#include "./font.h"
#include "../platform/platform.h"

bool Font::load(const std::string& path, const int size)
{
    data.clear();
    if (!FileSystem::readFile(path, data)) return false;
    GRRLIB_ttfFont* f = GRRLIB_LoadTTF(data.data(), static_cast<int>(data.size()));
    if (!f) return false;

    font.reset(f);
    fontSize = size;

    return true;
}

void Font::drawText(const std::string& text, const float x, const float y, const uint32_t color) const
{
    if (!font) return;
    GRRLIB_PrintfTTF(static_cast<int>(x), static_cast<int>(y), font.get(), text.c_str(), fontSize, color);
}

float Font::textWidth(const std::string& text) const
{
    if (!font) return 0.0f;
    return static_cast<float>(GRRLIB_WidthTTF(font.get(), text.c_str(), fontSize));
}

float Font::textHeight() const { return static_cast<float>(fontSize); }
bool Font::isValid() const { return font != nullptr; }
