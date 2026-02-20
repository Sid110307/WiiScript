#pragma once

#include "./widget.h"
#include "../theme.h"
#include "./layout.h"
#include "../../gfx/drawing.h"

#include <grrlib.h>

class ScrollBar : public Widget
{
public:
    explicit ScrollBar(const BoxDir d) : dir(d) { focusableOverride = true; }

    const float minThumb = 18.0f;
    BoxDir dir = BoxDir::Horizontal;
    float contentSize = 0.0f, viewSize = 0.0f, dragGrab = 0.0f, scrollAmount = 0.0f, *scroll = nullptr;
    bool hovered = false, dragging = false;

    bool onEvent(const Input::InputEvent& e) override
    {
        if (!visible || !enabled || !scroll) return false;
        const Rect r = worldBounds();

        if (e.type == Input::InputEvent::Type::Pointer)
        {
            hovered = e.pointer.valid && r.contains(e.pointer.x, e.pointer.y);

            if (dragging)
            {
                if (!e.pointer.valid)
                {
                    dragging = false;
                    return true;
                }

                const Rect thumb = thumbRect(r);
                if (dir == BoxDir::Vertical)
                {
                    float y = e.pointer.y - r.y - dragGrab;
                    if (const float track = r.h - thumb.h; track > 0.0f)
                    {
                        y = std::clamp(y, 0.0f, track);
                        *scroll = y / track * maxScroll();
                    }
                }
                else
                {
                    float x = e.pointer.x - r.x - dragGrab;
                    if (const float track = r.w - thumb.w; track > 0.0f)
                    {
                        x = std::clamp(x, 0.0f, track);
                        *scroll = x / track * maxScroll();
                    }
                }

                return true;
            }

            return false;
        }

        if (e.type == Input::InputEvent::Type::KeyDown && e.key == Input::Key::A)
        {
            if (!e.pointer.valid || !r.contains(e.pointer.x, e.pointer.y)) return false;
            const Rect thumb = thumbRect(r);

            if (thumb.contains(e.pointer.x, e.pointer.y))
            {
                dragging = true;
                dragGrab = dir == BoxDir::Vertical ? e.pointer.y - thumb.y : e.pointer.x - thumb.x;

                return true;
            }

            *scroll = dir == BoxDir::Vertical
                          ? std::clamp(*scroll + (e.pointer.y < thumb.y ? -viewSize : viewSize), 0.0f, maxScroll())
                          : std::clamp(*scroll + (e.pointer.x < thumb.x ? -viewSize : viewSize), 0.0f, maxScroll());
            return true;
        }

        if (e.type == Input::InputEvent::Type::KeyUp && e.key == Input::Key::A)
        {
            if (dragging)
            {
                dragging = false;
                return true;
            }
            return false;
        }

        if (e.type == Input::InputEvent::Type::KeyDown && focused)
        {
            if (dir == BoxDir::Vertical)
            {
                if (e.key == Input::Key::Up)
                {
                    *scroll = std::max(0.0f, *scroll - scrollAmount);
                    return true;
                }
                if (e.key == Input::Key::Down)
                {
                    *scroll = std::min(maxScroll(), *scroll + scrollAmount);
                    return true;
                }
            }
            else
            {
                if (e.key == Input::Key::Left)
                {
                    *scroll = std::max(0.0f, *scroll - scrollAmount);
                    return true;
                }
                if (e.key == Input::Key::Right)
                {
                    *scroll = std::min(maxScroll(), *scroll + scrollAmount);
                    return true;
                }
            }
        }

        return false;
    }

protected:
    void onDraw() const override
    {
        if (!visible || !enabled) return;
        const Rect r = worldBounds(), t = thumbRect(r);

        roundedRectangle(r.x, r.y, r.w, r.h, radiusX, radiusY, theme().scrollTrack, true);
        roundedRectangle(t.x, t.y, t.w, t.h, radiusX, radiusY,
                         dragging
                             ? theme().scrollActive
                             : hovered || focused
                             ? theme().scrollHover
                             : theme().scrollThumb, true);
    }

private:
    [[nodiscard]] float maxScroll() const { return std::max(0.0f, contentSize - viewSize); }

    [[nodiscard]] Rect thumbRect(const Rect& r) const
    {
        if (contentSize <= 0.0f || viewSize <= 0.0f) return Rect({r.x, r.y, r.w, r.h});
        const float ms = maxScroll(), ratio = std::clamp(ms > 0.0f ? *scroll / ms : 0.0f, 0.0f, 1.0f);

        if (dir == BoxDir::Vertical)
        {
            const float thumbH = std::clamp(r.h * (viewSize / contentSize), minThumb, r.h);
            return Rect({r.x, r.y + (r.h - thumbH) * ratio, r.w, thumbH});
        }

        const float thumbW = std::clamp(r.w * (viewSize / contentSize), minThumb, r.w);
        return Rect({r.x + (r.w - thumbW) * ratio, r.y, thumbW, r.h});
    }
};

class ScrollView : public Widget
{
public:
    Widget* content = nullptr;
    ScrollBar *barX = nullptr, *barY = nullptr;
    float scrollX = 0.0f, scrollY = 0.0f, padding = 0.0f, barWidth = 12.0f;

    ScrollView() { focusableOverride = true; }

protected:
    void onUpdate(double) override
    {
        Rect view = Rect({0, 0, bounds.w, bounds.h}).inset(padding), barRectY = Rect::empty(), barRectX = Rect::empty();
        if (!content)
        {
            scrollX = scrollY = 0.0f;
            if (barX) barX->visible = false;
            if (barY) barY->visible = false;

            return;
        }

        content->bounds.w = std::max(content->bounds.w, view.w);
        if (const auto* list = dynamic_cast<List*>(content)) content->bounds.h = list->contentHeight();
        if (const auto* textInput = dynamic_cast<TextInput*>(content))
            content->bounds.h = static_cast<float>(textInput->buffer().getLines().size()) * textInput->font->
                textHeight() + textInput->emptyArea;

        const bool needBarX = barX && content->bounds.w > view.w, needBarY = barY && content->bounds.h > view.h;
        if (needBarX) barRectX = view.takeBottom(barWidth);
        if (needBarY) barRectY = view.takeRight(barWidth);

        content->bounds = Rect({view.x - scrollX, view.y - scrollY, view.w, view.h});
        if (barX)
        {
            barX->scroll = &scrollX;
            barX->viewSize = view.w;
            barX->visible = needBarX;
            barX->contentSize = content->bounds.w;
            barX->bounds = barRectX;
            if (needBarY) barX->bounds.w -= barWidth;
        }
        if (barY)
        {
            barY->scroll = &scrollY;
            barY->viewSize = view.h;
            barY->visible = needBarY;
            barY->contentSize = content->bounds.h;
            barY->bounds = barRectY;
            if (needBarX) barY->bounds.h -= barWidth;
        }

        clampScroll();
    }

    void onDraw() const override
    {
        if (!visible) return;
        if (content && content->visible)
        {
            Rect clip = worldBounds().inset(padding);

            if (barY && barY->visible) clip.w -= barWidth;
            if (barX && barX->visible) clip.h -= barWidth;

            if (auto* list = dynamic_cast<List*>(content))
            {
                list->viewportTopY = clip.y;
                list->viewportH = clip.h;
            }

            if (auto* textInput = dynamic_cast<TextInput*>(content))
            {
                textInput->viewportScrollY = scrollY;
                textInput->viewportH = clip.h;
            }

            GX_SetScissor(static_cast<int>(clip.x), static_cast<int>(clip.y), static_cast<int>(clip.w),
                          static_cast<int>(clip.h));
            content->draw();
            GX_SetScissor(0, 0, 640, 480);
        }

        if (barX && barX->visible) barX->draw();
        if (barY && barY->visible) barY->draw();
    }

    Widget* hitTest(const float px, const float py) override
    {
        if (!visible) return nullptr;

        Rect clip = worldBounds().inset(padding);
        if (barX && barX->visible)
        {
            clip.h -= barWidth;
            if (Widget* hit = barX->hitTest(px, py)) return hit;
        }
        if (barY && barY->visible)
        {
            clip.w -= barWidth;
            if (Widget* hit = barY->hitTest(px, py)) return hit;
        }

        if (content && clip.contains(px, py)) if (Widget* hit = content->hitTest(px, py)) return hit;
        if (clip.contains(px, py)) return this;

        return nullptr;
    }

    bool onEvent(const Input::InputEvent& e) override
    {
        if (!visible || !enabled) return false;

        if (e.type == Input::InputEvent::Type::KeyDown && focused)
        {
            if (barY && barY->visible)
            {
                if (e.key == Input::Key::Up)
                {
                    scrollY -= barY->scrollAmount;
                    clampScroll();

                    return true;
                }
                if (e.key == Input::Key::Down)
                {
                    scrollY += barY->scrollAmount;
                    clampScroll();

                    return true;
                }
            }
            else if (barX && barX->visible)
            {
                if (e.key == Input::Key::Left)
                {
                    scrollX -= barX->scrollAmount;
                    clampScroll();

                    return true;
                }
                if (e.key == Input::Key::Right)
                {
                    scrollX += barX->scrollAmount;
                    clampScroll();

                    return true;
                }
            }
        }

        if (e.type == Input::InputEvent::Type::Scroll)
        {
            scrollX += static_cast<float>(e.scrollX) * (barX ? barX->scrollAmount : 0.0f);
            scrollY += static_cast<float>(e.scrollY) * (barY ? barY->scrollAmount : 0.0f);
            clampScroll();

            return true;
        }

        return false;
    }

private:
    void clampScroll()
    {
        if (!content)
        {
            scrollX = scrollY = 0.0f;
            return;
        }

        const float maxX = std::max(
                        0.0f, (barX ? barX->contentSize : content->bounds.w) - std::max(
                            0.0f, bounds.w - 2.0f * padding - (barY && barY->visible ? barWidth : 0.0f))),
                    maxY = std::max(0.0f, (barY ? barY->contentSize : content->bounds.h) - std::max(
                                        0.0f, bounds.h - 2.0f * padding - (barX && barX->visible ? barWidth : 0.0f)));

        scrollX = std::clamp(scrollX, 0.0f, maxX);
        scrollY = std::clamp(scrollY, 0.0f, maxY);
    }
};
