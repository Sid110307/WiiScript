#pragma once

#include "../../editor/text.h"
#include "../../editor/commands.h"

class TextInput : public Widget
{
public:
    explicit TextInput(Font& font)
    {
        this->font = &font;
        focusable = true;
    }

    bool extendSelection = false;
    float emptyArea = 20.0f, viewportScrollY = 0.0f, viewportH = 0.0f;
    std::function<void(float x, float y)> onContextMenu;

    [[nodiscard]] float getContentWidth() const
    {
        float maxWidth = 0.0f;
        for (const auto& line : editor.buffer().getLines()) maxWidth = std::max(maxWidth, font->textWidth(line));

        return maxWidth;
    }

    [[nodiscard]] float getContentHeight() const
    {
        return static_cast<float>(editor.buffer().getLines().size()) * font->textHeight() + emptyArea;
    }

    void loadFile(const std::string& path)
    {
        std::vector<uint8_t> data;
        if (!FileSystem::readFile(path, data)) return;

        editor.setText(std::string(data.begin(), data.end()));
        history.clear();

        caretVisible = true;
        caretBlinkTimer = 0.0f;
    }

    void saveFile(const std::string& path) const
    {
        const std::string text = editor.buffer().getText();
        const std::vector<uint8_t> data(text.begin(), text.end());

        FileSystem::writeFile(path, data);
    }

    void cutText()
    {
        if (!focused) return;
        auto r = editor.selectionRange();
        if (r.start == r.end) return;

        clipboard.text = editor.getTextInRange(r);
        history.execute(editor, std::make_unique<DeleteCommand>(r.start, r.end, clipboard.text, editor.cursorState()));

        caretVisible = true;
        caretBlinkTimer = 0.0f;
    }

    void copyText()
    {
        if (!focused) return;
        const auto r = editor.selectionRange();
        if (r.start == r.end) return;

        clipboard.text = editor.getTextInRange(r);
    }

    void pasteText()
    {
        if (!focused || clipboard.text.empty()) return;
        if (auto r = editor.selectionRange(); r.start != r.end)
            history.execute(
                editor, std::make_unique<DeleteCommand>(r.start, r.end, editor.getTextInRange(r),
                                                        editor.cursorState()));
        history.execute(
            editor, std::make_unique<InsertCommand>(editor.cursor().cursor(), clipboard.text, editor.cursorState()));

        caretVisible = true;
        caretBlinkTimer = 0.0f;
    }

    void selectAll()
    {
        if (!focused) return;
        const size_t lastLine = editor.buffer().getLines().empty() ? 0 : editor.buffer().getLines().size() - 1;

        editor.cursor().setCursor({0, 0}, false);
        editor.cursor().startSelection();
        editor.cursor().setCursor({lastLine, editor.buffer().lineLength(lastLine)}, false);
        editor.cursor().updateSelection();

        caretVisible = true;
        caretBlinkTimer = 0.0f;
    }

    void onKey(const char* key, const KeyAction action)
    {
        if (!focused) return;

        const auto before = editor.cursorState();
        auto makeDeleteCmd = [&]() -> std::unique_ptr<EditCommand>
        {
            auto r = editor.selectionRange();
            if (r.start == r.end) return nullptr;

            return std::make_unique<DeleteCommand>(r.start, r.end, editor.getTextInRange({r.start, r.end}), before);
        };

        switch (action)
        {
        case KeyAction::Backspace:
            {
                if (editor.cursor().hasSelection())
                {
                    history.execute(editor, makeDeleteCmd());
                    break;
                }

                if (TextPos c = editor.cursor().cursor(); c.col > 0)
                {
                    TextPos a = c;

                    a.col--;
                    history.execute(
                        editor, std::make_unique<DeleteCommand>(a, c, editor.getTextInRange({a, c}), before));
                }
                else if (c.line > 0)
                {
                    TextPos a = {c.line - 1, editor.buffer().lineLength(c.line - 1)};
                    TextPos b = {c.line, 0};

                    history.execute(
                        editor, std::make_unique<DeleteCommand>(a, b, editor.getTextInRange({a, b}), before));
                }

                break;
            }
        case KeyAction::Tab:
            {
                if (editor.cursor().hasSelection())history.execute(editor, makeDeleteCmd());
                history.execute(
                    editor, std::make_unique<InsertCommand>(editor.cursor().cursor(), "    ", editor.cursorState()));

                break;
            }
        case KeyAction::Enter:
            {
                if (editor.cursor().hasSelection()) history.execute(editor, makeDeleteCmd());
                history.execute(
                    editor, std::make_unique<InsertCommand>(editor.cursor().cursor(), "\n", editor.cursorState()));

                break;
            }
        case KeyAction::Text:
            {
                if (!key || !key[0]) break;
                if (editor.cursor().hasSelection()) history.execute(editor, makeDeleteCmd());

                history.execute(
                    editor, std::make_unique<InsertCommand>(editor.cursor().cursor(), std::string(key),
                                                            editor.cursorState()));
                break;
            }
        default:
            break;
        }

        caretVisible = true;
        caretBlinkTimer = 0.0f;
    }

    bool onEvent(const Input::InputEvent& e) override
    {
        if (!visible || !enabled) return Widget::onEvent(e);
        const Rect r = worldBounds();

        if (e.type == Input::InputEvent::Type::Pointer)
        {
            if (draggingSelection && e.pointer.valid)
            {
                if (e.pointer.y < r.y + 20.0f)
                    viewportScrollY = std::clamp(viewportScrollY - font->textHeight(), 0.0f,
                                                 std::max(0.0f, getContentHeight() - viewportH));
                else if (e.pointer.y > r.y + r.h - 20.0f)
                    viewportScrollY = std::clamp(viewportScrollY + font->textHeight(), 0.0f,
                                                 std::max(0.0f, getContentHeight() - viewportH));

                editor.cursor().setCursor(posFromPointer(e.pointer.x, e.pointer.y), false);
                editor.cursor().updateSelection();
                caretVisible = true;
                caretBlinkTimer = 0.0f;

                return true;
            }
            return Widget::onEvent(e);
        }

        if (e.type == Input::InputEvent::Type::KeyDown)
        {
            if (e.key == Input::Key::A)
            {
                if ((static_cast<uint8_t>(e.mods) & static_cast<uint8_t>(Input::KeyMods::ContextMenu)) != 0)
                {
                    if (e.pointer.valid && onContextMenu) onContextMenu(e.pointer.x, e.pointer.y);
                    return true;
                }

                if (e.pointer.valid && r.contains(e.pointer.x, e.pointer.y))
                {
                    editor.cursor().setCursor(posFromPointer(e.pointer.x, e.pointer.y), false);
                    editor.cursor().startSelection();
                    draggingSelection = true;

                    return true;
                }
                return Widget::onEvent(e);
            }

            if (e.key == Input::Key::B)
            {
                if (!focused) return Widget::onEvent(e);
                extendSelection = true;

                return true;
            }
            if (!focused) return Widget::onEvent(e);

            if (e.key == Input::Key::Minus)
            {
                if (!editor.cursor().hasSelection()) return true;

                copyText();
                if (extendSelection) cutText();

                return true;
            }
            if (e.key == Input::Key::Plus)
            {
                if (clipboard.text.empty()) return true;
                pasteText();

                return true;
            }

            if (e.key == Input::Key::Left)
            {
                editor.cursor().moveLeft(extendSelection);
                return true;
            }
            if (e.key == Input::Key::Right)
            {
                editor.cursor().moveRight(extendSelection);
                return true;
            }
            if (e.key == Input::Key::Up)
            {
                editor.cursor().moveUp(extendSelection);
                return true;
            }
            if (e.key == Input::Key::Down)
            {
                editor.cursor().moveDown(extendSelection);
                return true;
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
                if (!focused) return Widget::onEvent(e);
                extendSelection = false;

                return true;
            }
        }
        return Widget::onEvent(e);
    }

private:
    TextEditor editor;
    CommandHistory history;
    Clipboard clipboard;

    double caretBlinkTimer = 0.0f;
    bool caretVisible = true, draggingSelection = false;

    [[nodiscard]] TextPos posFromPointer(const float px, const float py) const
    {
        const Rect r = worldBounds().inset(10);
        const auto& lines = editor.buffer().getLines();
        if (lines.empty()) return {0, 0};

        const float lineH = font ? font->textHeight() : 16.0f;
        if (lineH <= 0.0f) return {0, 0};

        const size_t line = std::clamp(static_cast<size_t>(std::floor((py - r.y + viewportScrollY) / lineH)),
                                       static_cast<size_t>(0), lines.size() - 1);
        const std::string& s = lines[line];
        if (!font || s.empty() || px - r.x <= 0.0f) return {line, 0};

        if (hitTestLine != line || hitTestLineCache != s || hitTestPrefixWidths.size() != s.size() + 1)
        {
            hitTestLine = line;
            hitTestLineCache = s;
            hitTestPrefixWidths.assign(s.size() + 1, 0.0f);

            std::string temp;
            temp.reserve(s.size());

            hitTestPrefixWidths[0] = 0.0f;
            for (size_t i = 0; i < s.size(); ++i)
            {
                temp.push_back(s[i]);
                hitTestPrefixWidths[i + 1] = font->textWidth(temp);
            }
        }
        if (px - r.x >= hitTestPrefixWidths.back()) return {line, s.size()};

        size_t low = 1, high = s.size();
        while (low < high)
        {
            if (const size_t mid = (low + high) / 2; hitTestPrefixWidths[mid] < px - r.x) low = mid + 1;
            else high = mid;
        }

        const size_t col = std::max(low - 1, static_cast<size_t>(0));
        return {
            line,
            std::abs(px - r.x - hitTestPrefixWidths[col]) <= std::abs(hitTestPrefixWidths[low] - (px - r.x)) ? col : low
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
            const float y = r.y + static_cast<float>(i) * font->textHeight() - viewportScrollY;
            if (editor.cursor().hasSelection() && i >= selStart.line && i <= selEnd.line)
            {
                const size_t c0 = std::clamp(i == selStart.line ? selStart.col : 0, static_cast<size_t>(0),
                                             lines[i].size()),
                             c1 = std::clamp(i == selEnd.line ? selEnd.col : lines[i].size(), static_cast<size_t>(0),
                                             lines[i].size());
                if (c1 > c0)
                {
                    const float x0 = r.x + prefixWidthForLine(i, c0), x1 = r.x + prefixWidthForLine(i, c1);
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

            const float cx = r.x + prefixWidthForLine(c.line, c.col),
                        cy = r.y + static_cast<float>(c.line) * font->textHeight() - viewportScrollY;
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
    }

private:
    mutable size_t hitTestLine = static_cast<size_t>(-1), drawnLine = static_cast<size_t>(-1);
    mutable std::string hitTestLineCache, drawnLineCache;
    mutable std::vector<float> hitTestPrefixWidths, drawnLinePrefixWidths;

    float prefixWidthForLine(const size_t lineIndex, const size_t col) const
    {
        if (lineIndex >= editor.buffer().getLines().size()) return 0.0f;
        const std::string& s = editor.buffer().getLines()[lineIndex];
        if (!font || s.empty() || col == 0 || col > s.size()) return 0.0f;

        if (drawnLine != lineIndex || drawnLineCache != s || drawnLinePrefixWidths.size() != s.size() + 1)
        {
            drawnLine = lineIndex;
            drawnLineCache = s;
            drawnLinePrefixWidths.assign(s.size() + 1, 0.0f);

            std::string temp;
            temp.reserve(s.size());

            drawnLinePrefixWidths[0] = 0.0f;
            for (size_t i = 0; i < s.size(); ++i)
            {
                temp.push_back(s[i]);
                drawnLinePrefixWidths[i + 1] = font->textWidth(temp);
            }
        }
        return drawnLinePrefixWidths[col];
    }
};
