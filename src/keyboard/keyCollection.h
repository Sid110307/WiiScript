#pragma once

#include <vector>
#include "./config.h"

class KeyCollection
{
public:
    KeyCollection();

    [[nodiscard]] size_t keyCount() const;
    [[nodiscard]] const KeyboardKey& keyAt(size_t i) const;
    [[nodiscard]] const std::vector<KeyboardKey>& getKeys() const;

    bool shift = false, caps = false;

private:
    std::vector<KeyboardKey> keys;
};
