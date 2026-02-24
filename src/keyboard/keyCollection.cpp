#include "./keyCollection.h"
#include <numeric>

KeyCollection::KeyCollection()
{
    keys.clear();
    keys.reserve(std::accumulate(std::begin(keyboardLayout), std::end(keyboardLayout), static_cast<size_t>(0),
                                 [](const size_t sum, const RowSpec& r) { return sum + r.count; }));

    for (size_t row = 0; row < keyboardLayoutRows; ++row)
    {
        const RowSpec& r = keyboardLayout[row];

        float x = r.xOffset * (unitWidth + unitGap);
        const float y = static_cast<float>(row) * (unitHeight + unitGap);

        for (size_t i = 0; i < r.count; ++i)
        {
            KeyboardKey k = r.keys[i];
            k.x = x;
            k.y = y;
            k.h = unitHeight;
            k.w = k.unit * unitWidth + (k.unit - 1.0f) * unitGap;

            keys.push_back(k);
            x += k.w + unitGap;
        }
    }
}

size_t KeyCollection::keyCount() const { return keys.size(); }
const KeyboardKey& KeyCollection::keyAt(const size_t i) const { return keys.at(i); }
const std::vector<KeyboardKey>& KeyCollection::getKeys() const { return keys; }
