#pragma once

#include "../ui/widgets/button.h"
#include "../gfx/font.h"
#include "./keyCollection.h"

#include <string>

class KeyButton : public Button
{
public:
    KeyButton(KeyCollection& keys, Font& font, const KeyboardKey& key);
    [[nodiscard]] const KeyboardKey& getKey() const;

protected:
    void onDraw() const override;

private:
    KeyCollection* keys = nullptr;
    Font* font = nullptr;
    KeyboardKey key = {};
};

class Keyboard : public Widget
{
public:
    explicit Keyboard(Font& font);

    [[nodiscard]] const std::string& getText() const;
    void setText(const std::string& str);

protected:
    void onDraw() const override;

private:
    void activateKey(const std::string& keyText);

    KeyCollection keys;
    Font* font = nullptr;
    std::string textValue;
};
