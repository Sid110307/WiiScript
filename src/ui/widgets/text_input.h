#pragma once

#include "../../gfx/text.h"

class TextInput : public Widget
{
public:
    explicit TextInput(Font& font)
    {
        this->font = &font;
        focusableOverride = true;
    }

    bool extendSelection = false;
    float emptyArea = 20.0f, viewportScrollY = 0.0f, viewportH = 0.0f;

    [[nodiscard]] const TextBuffer& buffer() const { return editor.buffer(); }

    [[nodiscard]] float getSize() const
    {
        return static_cast<float>(editor.buffer().getLines().size()) * font->textHeight() + emptyArea;
    }

    void onKey(const char* key, const KeyAction action)
    {
        if (!focused) return;

        switch (action)
        {
        case KeyAction::Backspace:
            editor.backspace();
            break;
        case KeyAction::Tab:
            editor.insertText("    ");
            break;
        case KeyAction::Enter:
            editor.newLine();
            break;
        case KeyAction::Text:
            if (key && key[0]) editor.insertText(key);
            break;
        default:
            break;
        }

        caretVisible = true;
        caretBlinkTimer = 0.0f;
        bounds.h = std::max(bounds.h, getSize());
    }

    bool onEvent(const Input::InputEvent& e) override
    {
        if (!visible || !enabled) return false;
        const Rect r = worldBounds();

        if (e.type == Input::InputEvent::Type::Pointer)
        {
            if (draggingSelection && e.pointer.valid)
            {
                editor.cursor().setCursor(posFromPointer(e.pointer.x, e.pointer.y), false);
                editor.cursor().updateSelection();
                caretVisible = true;
                caretBlinkTimer = 0.0f;

                return true;
            }
            return false;
        }

        if (e.type == Input::InputEvent::Type::KeyDown)
        {
            if (e.key == Input::Key::A)
            {
                if (e.pointer.valid && r.contains(e.pointer.x, e.pointer.y))
                {
                    editor.cursor().setCursor(posFromPointer(e.pointer.x, e.pointer.y), false);
                    editor.cursor().startSelection();
                    draggingSelection = true;

                    return true;
                }
                return false;
            }

            if (e.key == Input::Key::B)
            {
                if (!focused) return false;
                extendSelection = true;

                return true;
            }

            if (!focused) return false;
            switch (e.key)
            {
            case Input::Key::Left:
                editor.cursor().moveLeft(extendSelection);
                return true;
            case Input::Key::Right:
                editor.cursor().moveRight(extendSelection);
                return true;
            case Input::Key::Up:
                editor.cursor().moveUp(extendSelection);
                return true;
            case Input::Key::Down:
                editor.cursor().moveDown(extendSelection);
                return true;
            default:
                break;
            }
        }

        if (e.type == Input::InputEvent::Type::KeyUp)
        {
            if (e.key == Input::Key::A && draggingSelection)
            {
                draggingSelection = false;
                return true;
            }
            if (e.key == Input::Key::B)
            {
                if (!focused) return false;
                extendSelection = false;

                return true;
            }
        }
        return false;
    }

private:
    TextEditor editor;
    double caretBlinkTimer = 0.0f;
    bool caretVisible = true, draggingSelection = false;

    [[nodiscard]] TextPos posFromPointer(const float px, const float py) const
    {
        const Rect r = worldBounds().inset(10);
        const auto& lines = editor.buffer().getLines();
        const size_t line = std::clamp(static_cast<size_t>(std::floor((py - r.y) / font->textHeight())),
                                       static_cast<size_t>(0), lines.size() - 1);
        const std::string& s = lines[line];
        const float charW = s.empty() ? font->textWidth(" ") : font->textWidth(s) / static_cast<float>(s.size());

        return {
            line, px - r.x > 0.0f ? std::min(static_cast<size_t>(std::floor((px - r.x) / charW + 0.5f)), s.size()) : 0
        };
    }

protected:
    void onDraw() const override
    {
        const Rect r = worldBounds().inset(10);
        const auto& lines = editor.buffer().getLines();
        TextPos selStart = {}, selEnd = {};

        if (editor.cursor().hasSelection())
        {
            selStart = editor.cursor().selectionStartPos();
            selEnd = editor.cursor().selectionEndPos();
        }

        const size_t startLine = std::clamp(static_cast<size_t>(std::floor(viewportScrollY / font->textHeight())),
                                            static_cast<size_t>(0), lines.size()),
                     endLine = std::clamp(
                         static_cast<size_t>(std::ceil((viewportScrollY + viewportH) / font->textHeight())) + 1,
                         static_cast<size_t>(0), lines.size());
        for (size_t i = startLine; i < endLine; ++i)
        {
            const float y = r.y + static_cast<float>(i) * font->textHeight();
            if (editor.cursor().hasSelection() && i >= selStart.line && i <= selEnd.line)
            {
                const size_t c0 = std::clamp(i == selStart.line ? selStart.col : 0, static_cast<size_t>(0),
                                             lines[i].size()),
                             c1 = std::clamp(i == selEnd.line ? selEnd.col : lines[i].size(), static_cast<size_t>(0),
                                             lines[i].size());
                if (c1 > c0)
                {
                    const float x0 = r.x + font->textWidth(lines[i].substr(0, c0)),
                                x1 = r.x + font->textWidth(lines[i].substr(0, c1));
                    GRRLIB_Rectangle(x0, y, x1 - x0, font->textHeight(), theme().selection, true);
                }
            }

            if (!lines[i].empty()) font->drawText(lines[i], r.x, y, theme().text);
        }

        if (focused && caretVisible && !lines.empty())
        {
            TextPos c = editor.cursor().cursor();
            c.line = std::clamp(c.line, static_cast<size_t>(0), lines.size() - 1);
            c.col = std::clamp(c.col, static_cast<size_t>(0), lines[c.line].size());

            const float cx = r.x + font->textWidth(lines[c.line].substr(0, c.col)),
                        cy = r.y + static_cast<float>(c.line) * font->textHeight();
            GRRLIB_Line(cx, cy, cx, cy + font->textHeight(), theme().accent);
        }
    }

    void onUpdate(const double dt) override
    {
        caretBlinkTimer += dt;
        if (caretBlinkTimer >= 0.5f)
        {
            caretBlinkTimer = 0.0f;
            caretVisible = !caretVisible;
        }

        bounds.h = std::max(bounds.h, getSize());
    }
};
