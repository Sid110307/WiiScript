#pragma once

#include <vector>
#include "./config.h"

class KeyCollection
{
public:
    KeyCollection();
    void resetState();

    [[nodiscard]] KeyboardKey getSelected(float x, float y) const;
    [[nodiscard]] std::size_t keyCount() const;
    [[nodiscard]] const KeyboardKey& keyAt(std::size_t i) const;
    [[nodiscard]] const std::vector<KeyboardKey>& getKeys() const;

    bool shift = false, caps = false;

private:
    std::vector<KeyboardKey> keys;
};
