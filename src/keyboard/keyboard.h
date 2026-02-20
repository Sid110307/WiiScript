#pragma once

#include "../ui/widgets/button.h"
#include "../gfx/font.h"
#include "./keyCollection.h"

#include <string>

class KeyButton : public Button
{
public:
    KeyButton(KeyCollection& keys, Font& font, size_t keyIndex);
    [[nodiscard]] size_t index() const;

protected:
    void onDraw() const override;

private:
    KeyCollection* keys = nullptr;
    Font* font = nullptr;
    size_t keyIndex = 0;
};

class Keyboard : public Widget
{
public:
    explicit Keyboard(Font& font);

    [[nodiscard]] const std::string& getText() const;
    void setText(const std::string& str);

    std::function<void(const char* key, KeyAction action)> onKey;

protected:
    void onDraw() const override;
    void onUpdate(double) override;

private:
    void layoutKeys() const;
    void activateKey(const char* keyText, KeyAction action);

    KeyCollection keys;
    std::vector<KeyButton*> keyButtons;
    Font* font = nullptr;
    std::string textValue;
    Rect lastBounds = Rect::empty();
};
