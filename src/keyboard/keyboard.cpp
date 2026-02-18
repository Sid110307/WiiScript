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
    if (!key.control && (keys->shift || keys->caps)) label = key.shiftVal;

    font->drawText(label, bounds.x + (bounds.w - font->textWidth(label)) / 2,
                   bounds.y + (bounds.h - font->textHeight()) / 2, 0xFFFFFFFF);
}


Keyboard::Keyboard(Font& font) : font(&font)
{
    children.clear();
    for (std::size_t i = 0; i < keys.keyCount(); ++i)
    {
        const KeyboardKey& k = keys.keyAt(i);
        auto* btn = addChild<KeyButton>(keys, font, k);

        btn->bounds.x = bounds.x + k.x;
        btn->bounds.y = bounds.y + k.y;
        btn->bounds.w = static_cast<float>(k.w);
        btn->bounds.h = static_cast<float>(k.h);
        btn->onClick = [this, &k] { activateKey(k.control ? k.val : keys.shift || keys.caps ? k.shiftVal : k.val); };
    }
}

const std::string& Keyboard::getText() const { return textValue; }
void Keyboard::setText(const std::string& str) { textValue = str; }

void Keyboard::onDraw() const
{
    GRRLIB_Rectangle(bounds.x, bounds.y, bounds.w, bounds.h, theme().panel, true);
    GRRLIB_Rectangle(bounds.x, bounds.y, bounds.w, bounds.h, theme().panelBorder, false);

    if (font && font->isValid()) font->drawText(textValue, bounds.x + keys.textX, bounds.y + keys.textY, 0xFFFFFFFF);
}


void Keyboard::activateKey(const std::string& t)
{
    if (t.empty()) return;

    if (t == "BACK ⌫" && !textValue.empty()) textValue.pop_back();
    else if (t == "ENTER ⏎") textValue.push_back('\n');
    else if (t == "CAPS ⇪") keys.caps = !keys.caps;
    else if (t == "SHIFT ⇧") keys.shift = !keys.shift;
    else
    {
        keys.shift = false;
        textValue.append(t);
    }
}
