#include "./font.h"
#include "../platform/platform.h"

#include <cmath>

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

void Font::drawText(const std::string_view text, const float x, const float y, const uint32_t color) const
{
    if (!font || text.empty()) return;
    GRRLIB_PrintfTTF(static_cast<int>(std::round(x)), static_cast<int>(std::round(y)), font.get(), text.data(),
                     fontSize, color);
}

float Font::textWidth(const std::string_view text) const
{
    if (!font) return 0.0f;
    return static_cast<float>(GRRLIB_WidthTTF(font.get(), text.data(), fontSize));
}

float Font::textHeight() const { return static_cast<float>(fontSize); }
