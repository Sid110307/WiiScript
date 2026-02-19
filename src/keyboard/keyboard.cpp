#include "./keyboard.h"

KeyButton::KeyButton(KeyCollection& keys, Font& font, const KeyboardKey& key) : keys(&keys), font(&font), key(key)
{
}

const KeyboardKey& KeyButton::getKey() const { return key; }

void KeyButton::onDraw() const
{
    Button::onDraw();
    if (!font || !font->isValid()) return;

    const char* label = key.val;
    const Rect r = worldBounds();
    if (keys->shift || keys->caps) label = key.shiftVal;

    font->drawText(label, r.x + (r.w - font->textWidth(label)) / 2, r.y + (r.h - font->textHeight()) / 2, theme().text);
}

Keyboard::Keyboard(Font& font) : font(&font)
{
    children.clear();
    for (std::size_t i = 0; i < keys.keyCount(); ++i)
    {
        const KeyboardKey& k = keys.keyAt(i);
        auto* btn = addChild<KeyButton>(keys, font, k);
        btn->bounds = {k.x, k.y, k.w, k.h};

        btn->onClick = [this, i]
        {
            const KeyboardKey& kk = keys.keyAt(i);
            activateKey(keys.shift || keys.caps ? kk.shiftVal : kk.val);
        };
    }
}

const std::string& Keyboard::getText() const { return textValue; }
void Keyboard::setText(const std::string& str) { textValue = str; }

void Keyboard::onDraw() const
{
    const Rect r = worldBounds();

    roundedRectangle(r.x, r.y, r.w, r.h, radiusX, radiusY, theme().panel, true);
    roundedRectangle(r.x, r.y, r.w, r.h, radiusX, radiusY, theme().panelBorder, false);
}

void Keyboard::activateKey(const std::string& keyText)
{
    if (keyText.empty()) return;

    if (keyText == "Backspace")
    {
        if (!textValue.empty()) textValue.pop_back();
    }
    else if (keyText == "Tab") textValue.push_back('\t');
    else if (keyText == "Enter") textValue.push_back('\n');
    else if (keyText == "Caps") keys.caps = !keys.caps;
    else if (keyText == "Shift") keys.shift = !keys.shift;
    else
    {
        keys.shift = false;
        textValue.append(keyText);
    }
}
