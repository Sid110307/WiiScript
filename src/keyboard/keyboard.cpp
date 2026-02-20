#include "./keyboard.h"

#include <cfloat>

static Rect computeBounds(const std::vector<KeyboardKey>& keys)
{
    float minX = FLT_MAX, minY = FLT_MAX, maxX = -FLT_MAX, maxY = -FLT_MAX;
    for (const auto& k : keys)
    {
        minX = std::min(minX, k.x);
        minY = std::min(minY, k.y);
        maxX = std::max(maxX, k.x + k.w);
        maxY = std::max(maxY, k.y + k.h);
    }

    if (minX == FLT_MAX) return Rect::empty();
    return Rect({minX, minY, maxX - minX, maxY - minY});
}

KeyButton::KeyButton(KeyCollection& keys, Font& font, const size_t keyIndex)
    : keys(&keys), font(&font), keyIndex(keyIndex)
{
}

size_t KeyButton::index() const { return keyIndex; }

void KeyButton::onDraw() const
{
    Button::onDraw();
    if (!keys || !font || !font->isValid()) return;

    const auto& k = keys->keyAt(keyIndex);
    const char* label = keys->shift || (keys->caps && std::isalpha(static_cast<unsigned char>(k.val[0])))
                            ? k.shiftVal
                            : k.val;
    const Rect r = worldBounds();

    font->drawText(label, r.x + (r.w - font->textWidth(label)) / 2, r.y + (r.h - font->textHeight()) / 2, theme().text);
}

Keyboard::Keyboard(Font& font) : font(&font)
{
    keyButtons.clear();
    keyButtons.reserve(keys.keyCount());

    for (size_t i = 0; i < keys.keyCount(); ++i)
    {
        const auto& key = keys.keyAt(i);
        auto* btn = addChild<KeyButton>(keys, font, i);
        keyButtons.push_back(btn);

        btn->bounds = {key.x, key.y, key.w, key.h};
        btn->onClick = [this, i]
        {
            const auto& k = keys.keyAt(i);
            activateKey(keys.shift || (keys.caps && std::isalpha(static_cast<unsigned char>(k.val[0])))
                            ? k.shiftVal
                            : k.val, k.action);
        };
    }
}

const std::string& Keyboard::getText() const { return textValue; }
void Keyboard::setText(const std::string& str) { textValue = str; }

void Keyboard::onDraw() const
{
    const Rect r = worldBounds();
    roundedRectangle(r.x, r.y, r.w, r.h, radiusX, radiusY, theme().panel, true);
}

void Keyboard::onUpdate(double)
{
    if (bounds != lastBounds)
    {
        lastBounds = bounds;
        layoutKeys();
    }
}

void Keyboard::layoutKeys() const
{
    const Rect dest = {0.0f, 0.0f, bounds.w, bounds.h};
    const auto& keysList = keys.getKeys();
    const Rect src = computeBounds(keysList);

    if (dest.w <= 0.0f || dest.h <= 0.0f || src.w <= 0.0f || src.h <= 0.0f) return;
    const float scaleX = dest.w / src.w, scaleY = dest.h / src.h,
                offsetX = dest.x - src.x * scaleX + (dest.w - src.w * scaleX) / 2.0f,
                offsetY = dest.y - src.y * scaleY + (dest.h - src.h * scaleY) / 2.0f;

    const size_t n = std::min(keyButtons.size(), keysList.size());
    for (size_t i = 0; i < n; ++i)
    {
        const auto& k = keysList[i];
        keyButtons[i]->bounds = {k.x * scaleX + offsetX, k.y * scaleY + offsetY, k.w * scaleX, k.h * scaleY};
    }
}

void Keyboard::activateKey(const char* keyText, const KeyAction action)
{
    switch (action)
    {
    case KeyAction::Backspace:
        if (!textValue.empty()) textValue.pop_back();
        break;
    case KeyAction::Tab:
        textValue.push_back('\t');
        break;
    case KeyAction::Enter:
        textValue.push_back('\n');
        break;
    case KeyAction::Caps:
        keys.caps = !keys.caps;
        break;
    case KeyAction::Shift:
        keys.shift = !keys.shift;
        break;
    case KeyAction::Text:
    default:
        if (keyText && keyText[0] != '\0')
        {
            keys.shift = false;
            textValue += keyText;
        }
        break;
    }
}
