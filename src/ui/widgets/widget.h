#pragma once

#include <vector>
#include <memory>

#include "../theme.h"
#include "../../platform/platform.h"
#include "../../gfx/font.h"

class Widget
{
public:
    virtual ~Widget() = default;

    Rect bounds = {};
    bool visible = true, enabled = true;
    Widget* parent = nullptr;
    std::vector<std::unique_ptr<Widget>> children;
    Font* font = nullptr;
    float radiusX = 8.0f, radiusY = 8.0f;

    template <typename T, typename... Args>
    T* addChild(Args&&... args)
    {
        auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
        ptr->parent = this;
        T* raw = ptr.get();
        children.push_back(std::move(ptr));

        return raw;
    }

    virtual Widget* hitTest(const float px, const float py)
    {
        if (!visible) return nullptr;
        for (int i = static_cast<int>(children.size()) - 1; i >= 0; --i)
            if (Widget* hit = children[i]->hitTest(px, py)) return hit;
        if (worldBounds().contains(px, py)) return this;

        return nullptr;
    }

    virtual bool onEvent(const Input::InputEvent&) { return false; }

    void update(const double dt)
    {
        if (!visible) return;
        onUpdate(dt);
        for (const auto& c : children) c->update(dt);
    }

    void draw() const
    {
        if (!visible) return;
        onDraw();
        for (const auto& c : children) c->draw();
    }

    [[nodiscard]] Rect worldBounds() const
    {
        Rect r = bounds;
        for (const Widget* p = parent; p; p = p->parent)
        {
            r.x += p->bounds.x;
            r.y += p->bounds.y;
        }

        return r;
    }

    [[nodiscard]] Font* getFont() const { return font ? font : parent ? parent->getFont() : nullptr; }
    [[nodiscard]] virtual bool isFocusable() const { return false; }

protected:
    virtual void onUpdate(double)
    {
    }

    virtual void onDraw() const
    {
    }
};
