#pragma once

#include "./widget.h"

class Dock : public Widget
{
public:
    Widget *left = nullptr, *right = nullptr, *top = nullptr, *bottom = nullptr, *center = nullptr;
    float leftWidth = 0.0f, rightWidth = 0.0f, topHeight = 0.0f, bottomHeight = 0.0f, padding = 0.0f;

protected:
    void onUpdate(double) override
    {
        Rect content = Rect({0, 0, bounds.w, bounds.h}).inset(padding);

        if (left) left->bounds = left->visible ? content.takeLeft(leftWidth) : Rect::empty();
        if (right) right->bounds = right->visible ? content.takeRight(rightWidth) : Rect::empty();
        if (top) top->bounds = top->visible ? content.takeTop(topHeight) : Rect::empty();
        if (bottom) bottom->bounds = bottom->visible ? content.takeBottom(bottomHeight) : Rect::empty();
        if (center) center->bounds = center->visible ? content : Rect::empty();
    }
};

enum class BoxDir { Horizontal, Vertical };

class Box : public Widget
{
public:
    explicit Box(const BoxDir d) : dir(d)
    {
    }

    BoxDir dir = BoxDir::Horizontal;
    float padding = 0.0f, gap = 0.0f;
    bool crossStretch = false;

protected:
    void onUpdate(double) override
    {
        const Rect content = Rect({0, 0, bounds.w, bounds.h}).inset(padding);

        std::vector<Widget*> visibleChildren;
        visibleChildren.reserve(children.size());
        for (const auto& c : children) if (c->visible) visibleChildren.push_back(c.get());

        const int count = static_cast<int>(visibleChildren.size());
        if (count <= 0) return;

        const float mainAvailable = dir == BoxDir::Horizontal ? content.w : content.h,
                    crossAvailable = dir == BoxDir::Horizontal ? content.h : content.w;
        float fixedSum = 0.0f, flexSum = 0.0f;

        for (const Widget* w : visibleChildren)
        {
            const LayoutParams& lp = w->layout;

            if (const float fixedMain = dir == BoxDir::Horizontal ? lp.fixedWidth : lp.fixedHeight; fixedMain >= 0.0f)
                fixedSum += fixedMain;
            else if (lp.flex > 0.0f) flexSum += lp.flex;
        }

        const float totalGaps = gap * static_cast<float>(std::max(0, count - 1));
        float rest = mainAvailable - fixedSum - totalGaps;
        if (rest < 0.0f) rest = 0.0f;

        float cursor = dir == BoxDir::Horizontal ? content.x : content.y;
        for (Widget* w : visibleChildren)
        {
            LayoutParams lp = w->layout;
            const float fixedMain = dir == BoxDir::Horizontal ? lp.fixedWidth : lp.fixedHeight,
                        fixedCross = dir == BoxDir::Horizontal ? lp.fixedHeight : lp.fixedWidth,
                        prefMain = dir == BoxDir::Horizontal ? w->bounds.w : w->bounds.h,
                        prefCross = dir == BoxDir::Horizontal ? w->bounds.h : w->bounds.w;

            float mainSize = fixedMain >= 0.0f
                                 ? fixedMain
                                 : lp.flex > 0.0f && flexSum > 0.0f
                                 ? rest * (lp.flex / flexSum)
                                 : prefMain,
                  crossSize = fixedCross >= 0.0f ? fixedCross : crossStretch ? crossAvailable : prefCross;

            const float ww = std::clamp(dir == BoxDir::Horizontal ? mainSize : crossSize, lp.minWidth, lp.maxWidth),
                        hh = std::clamp(dir == BoxDir::Horizontal ? crossSize : mainSize, lp.minHeight, lp.maxHeight);
            float cx = content.x, cy = content.y;

            if (dir == BoxDir::Horizontal)
            {
                if (crossAvailable > hh) cy += (crossAvailable - hh) * lp.alignY;

                w->bounds = Rect({cursor, cy, ww, hh});
                cursor += ww + gap;
            }
            else
            {
                if (crossAvailable > ww) cx += (crossAvailable - ww) * lp.alignX;

                w->bounds = Rect({cx, cursor, ww, hh});
                cursor += hh + gap;
            }
        }
    }
};
