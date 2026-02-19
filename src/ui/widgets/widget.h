#pragma once

#include <vector>
#include <memory>
#include <limits>

#include "../theme.h"
#include "../../platform/platform.h"
#include "../../gfx/font.h"

struct LayoutParams
{
    float fixedWidth = -1.0f, fixedHeight = -1.0f, flex = 0.0f, minWidth = 0.0f, minHeight = 0.0f,
          maxWidth = std::numeric_limits<float>::infinity(), maxHeight = std::numeric_limits<float>::infinity(),
          alignX = 0.0f, alignY = 0.0f;
};

class Widget
{
public:
    virtual ~Widget() = default;

    Rect bounds = {};
    bool visible = true, enabled = true;

    Widget* parent = nullptr;
    std::vector<std::unique_ptr<Widget>> children;
    Font* font = nullptr;
    LayoutParams layout;

    float radiusX = 8.0f, radiusY = 8.0f;
    bool focusableOverride = false, focused = false;

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
        for (const auto& c : children) c->update(dt);
        onUpdate(dt);
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
    [[nodiscard]] virtual bool isFocusable() const { return focusableOverride && visible && enabled; }

    void collectFocusable(std::vector<Widget*>& out)
    {
        if (!visible || !enabled) return;
        if (isFocusable()) out.push_back(this);
        for (const auto& c : children) c->collectFocusable(out);
    }

protected:
    virtual void onDraw() const
    {
    }

    virtual void onUpdate(double)
    {
    }
};
