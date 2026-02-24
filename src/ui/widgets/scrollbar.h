#pragma once

#include "./widget.h"
#include "../theme.h"
#include "./layout.h"
#include "../../gfx/drawing.h"

#include <grrlib.h>

class ScrollBar : public Widget
{
public:
    explicit ScrollBar(const BoxDir d) : dir(d) { focusable = true; }

    const float minThumb = 18.0f;
    BoxDir dir = BoxDir::Horizontal;
    float contentSize = 0.0f, viewSize = 0.0f, dragGrab = 0.0f, scrollAmount = 0.0f, *scroll = nullptr;
    bool hovered = false, dragging = false;

    bool onEvent(const Input::InputEvent& e) override
    {
        if (!visible || !enabled || !scroll) return Widget::onEvent(e);
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

            return Widget::onEvent(e);
        }

        if (e.type == Input::InputEvent::Type::KeyDown && e.key == Input::Key::A)
        {
            if (!e.pointer.valid || !r.contains(e.pointer.x, e.pointer.y)) return Widget::onEvent(e);
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
            return Widget::onEvent(e);
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

        return Widget::onEvent(e);
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

    ScrollView() { focusable = true; }
    bool onEvent(const Input::InputEvent& e) override
    {
        if (!visible || !enabled) return Widget::onEvent(e);

        if (e.type == Input::InputEvent::Type::KeyDown)
        {
            if (e.key == Input::Key::A && e.pointer.valid)
            {
                Rect clip = worldBounds().inset(padding);
                if (barY && barY->visible) clip.w -= barWidth;
                if (barX && barX->visible) clip.h -= barWidth;

                if (content && clip.contains(e.pointer.x, e.pointer.y)) return content->onEvent(e);
            }

            if (!focused) return Widget::onEvent(e);
            if (barY && barY->visible)
            {
                if (e.key == Input::Key::Up)
                {
                    scrollY -= barY->scrollAmount;
                    clampScroll(bounds.w - (barX && barX->visible ? barWidth : 0.0f),
                                bounds.h - (barY->visible ? barWidth : 0.0f), content ? content->bounds.w : 0.0f,
                                content ? content->bounds.h : 0.0f);

                    return true;
                }
                if (e.key == Input::Key::Down)
                {
                    scrollY += barY->scrollAmount;
                    clampScroll(bounds.w - (barX && barX->visible ? barWidth : 0.0f),
                                bounds.h - (barY->visible ? barWidth : 0.0f), content ? content->bounds.w : 0.0f,
                                content ? content->bounds.h : 0.0f);

                    return true;
                }
            }
            else if (barX && barX->visible)
            {
                if (e.key == Input::Key::Left)
                {
                    scrollX -= barX->scrollAmount;
                    clampScroll(bounds.w - (barX->visible ? barWidth : 0.0f),
                                bounds.h - (barY && barY->visible ? barWidth : 0.0f),
                                content ? content->bounds.w : 0.0f, content ? content->bounds.h : 0.0f);

                    return true;
                }
                if (e.key == Input::Key::Right)
                {
                    scrollX += barX->scrollAmount;
                    clampScroll(bounds.w - (barX->visible ? barWidth : 0.0f),
                                bounds.h - (barY && barY->visible ? barWidth : 0.0f),
                                content ? content->bounds.w : 0.0f, content ? content->bounds.h : 0.0f);

                    return true;
                }
            }
        }

        if (e.type == Input::InputEvent::Type::Scroll)
        {
            scrollX += static_cast<float>(e.scrollX) * (barX ? barX->scrollAmount : 0.0f);
            scrollY += static_cast<float>(e.scrollY) * (barY ? barY->scrollAmount : 0.0f);
            clampScroll(bounds.w - (barX && barX->visible ? barWidth : 0.0f),
                        bounds.h - (barY && barY->visible ? barWidth : 0.0f), content ? content->bounds.w : 0.0f,
                        content ? content->bounds.h : 0.0f);

            return true;
        }

        return Widget::onEvent(e);
    }

protected:
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

    void onUpdate(double) override
    {
        Rect view = Rect({0, 0, bounds.w, bounds.h}).inset(padding), barRectY = Rect::empty(), barRectX = Rect::empty();
        float contentW = content ? std::max(view.w, content->bounds.w) : 0.0f;

        if (!content)
        {
            scrollX = scrollY = 0.0f;
            if (barX) barX->visible = false;
            if (barY) barY->visible = false;

            return;
        }

        float contentH = std::max(view.h, content->bounds.h);
        if (const auto* list = dynamic_cast<List*>(content)) contentH = std::max(contentH, list->contentHeight());
        if (const auto* textInput = dynamic_cast<TextInput*>(content))
        {
            contentW = std::max(contentW, textInput->getContentWidth() + padding * 2.0f);
            contentH = std::max(contentH, textInput->getContentHeight() + padding * 2.0f);
        }

        bool needBarX = barX && contentW > view.w, needBarY = barY && contentH > view.h;

        if (needBarX) barRectX = view.takeBottom(barWidth);
        if (needBarY) barRectY = view.takeRight(barWidth);
        needBarX = barX && contentW > view.w;
        needBarY = barY && contentH > view.h;

        content->bounds = Rect({view.x - scrollX, view.y - scrollY, contentW, contentH});
        if (barX)
        {
            barX->scroll = &scrollX;
            barX->viewSize = view.w;
            barX->visible = needBarX;
            barX->contentSize = contentW;
            barX->bounds = barRectX;
            if (needBarY) barX->bounds.w -= barWidth;
        }
        if (barY)
        {
            barY->scroll = &scrollY;
            barY->viewSize = view.h;
            barY->visible = needBarY;
            barY->contentSize = contentH;
            barY->bounds = barRectY;
            if (needBarX) barY->bounds.h -= barWidth;
        }

        clampScroll(view.w, view.h, contentW, contentH);
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

        if (content && clip.contains(px, py))
        {
            if (Widget* hit = content->hitTest(px, py)) return hit;
            return content;
        }
        if (clip.contains(px, py)) return this;

        return nullptr;
    }

private:
    void clampScroll(const float viewW, const float viewH, const float contentW, const float contentH)
    {
        if (!content)
        {
            scrollX = scrollY = 0.0f;
            return;
        }

        scrollX = std::clamp(scrollX, 0.0f, std::max(0.0f, contentW - viewW));
        scrollY = std::clamp(scrollY, 0.0f, std::max(0.0f, contentH - viewH));
    }
};
