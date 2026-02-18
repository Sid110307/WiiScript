#include "./keyCollection.h"

void KeyCollection::resetState()
{
    shift = false;
    caps = false;
}

KeyCollection::KeyCollection()
{
    keys.clear();
    for (const auto& k : keyboardKeys)
        keys.push_back({
            .valid = true, .control = k.control, .x = k.x, .y = k.y, .w = k.w, .h = k.h, .val = k.val,
            .shiftVal = k.shiftVal,
        });
}

KeyboardKey KeyCollection::getSelected(const float x, const float y) const
{
    for (const auto& k : keys) if (x > k.x && x < k.x + k.w && y > k.y && y < k.y + k.h) return k;

    KeyboardKey none = {};
    none.valid = false;

    return none;
}

std::size_t KeyCollection::keyCount() const { return keys.size(); }
const KeyboardKey& KeyCollection::keyAt(const std::size_t i) const { return keys.at(i); }
const std::vector<KeyboardKey>& KeyCollection::getKeys() const { return keys; }
