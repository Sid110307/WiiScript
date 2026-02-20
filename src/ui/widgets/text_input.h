#pragma once

struct TextPos
{
    size_t line = 0, col = 0;
    friend bool operator==(const TextPos& a, const TextPos& b) { return a.line == b.line && a.col == b.col; }
    friend bool operator!=(const TextPos& a, const TextPos& b) { return !(a == b); }
};

inline TextPos minPos(const TextPos a, const TextPos b)
{
    return a.line < b.line ? a : a.line > b.line ? b : a.col < b.col ? a : b;
}

inline TextPos maxPos(const TextPos a, const TextPos b) { return minPos(b, a); }

class TextBuffer
{
public:
    void setText(const std::string& text)
    {
        lines.clear();

        std::string cur;
        cur.reserve(text.size());

        for (const char ch : text)
        {
            if (ch == '\r') continue;
            if (ch == '\n')
            {
                lines.push_back(cur);
                cur.clear();
            }
            else cur.push_back(ch);
        }

        lines.push_back(cur);
        setCursor({0, 0});
    }

    [[nodiscard]] std::string getText() const
    {
        std::string result;
        for (size_t i = 0; i < lines.size(); ++i)
        {
            result += lines[i];
            if (i < lines.size() - 1) result += '\n';
        }

        return result;
    }

    [[nodiscard]] const std::vector<std::string>& getLines() const { return lines; }
    [[nodiscard]] TextPos cursor() const { return cursorPos; }

    void setCursor(const TextPos pos, const bool clearSel = true)
    {
        cursorPos = pos;
        clampCursor();

        if (clearSel) clearSelection();
        if (selecting) updateSelection();
    }

    [[nodiscard]] bool hasSelection() const { return selecting && selectionStart != selectionEnd; }
    [[nodiscard]] TextPos selectionStartPos() const { return minPos(selectionStart, selectionEnd); }
    [[nodiscard]] TextPos selectionEndPos() const { return maxPos(selectionStart, selectionEnd); }

    void startSelection()
    {
        selecting = true;
        selectionStart = selectionEnd = cursorPos;
    }

    void updateSelection() { selectionEnd = cursorPos; }

    void clearSelection()
    {
        selecting = false;
        selectionStart = selectionEnd = cursorPos;
    }

    void insertText(const std::string& text)
    {
        deleteAnySelection();

        for (const char ch : text)
        {
            if (ch == '\r') continue;
            if (ch == '\n')
            {
                newLine();
                continue;
            }

            auto& line = lines[cursorPos.line];
            line.insert(cursorPos.col, 1, ch);
            cursorPos.col++;
        }
    }

    void backspace()
    {
        if (hasSelection())
        {
            deleteAnySelection();
            return;
        }

        if (cursorPos.col > 0)
        {
            auto& line = lines[cursorPos.line];

            line.erase(cursorPos.col - 1, 1);
            cursorPos.col--;
        }
        else if (cursorPos.line > 0)
        {
            const std::string prevLine = lines[cursorPos.line - 1];

            lines[cursorPos.line - 1] += lines[cursorPos.line];
            lines.erase(lines.begin() + static_cast<int>(cursorPos.line));
            cursorPos.line--;
            cursorPos.col = prevLine.size();
        }
    }

    void newLine()
    {
        deleteAnySelection();

        auto& line = lines[cursorPos.line];
        const std::string nextLine = line.substr(cursorPos.col);

        line.erase(cursorPos.col);
        lines.insert(lines.begin() + static_cast<int>(cursorPos.line) + 1, nextLine);
        cursorPos.line++;
        cursorPos.col = 0;
    }

    void moveLeft(const bool extendSelection = false)
    {
        if (extendSelection && !selecting) startSelection();

        if (cursorPos.col > 0) cursorPos.col--;
        else if (cursorPos.line > 0)
        {
            cursorPos.line--;
            cursorPos.col = lines[cursorPos.line].size();
        }

        if (extendSelection) updateSelection();
        else clearSelection();
    }

    void moveRight(const bool extendSelection = false)
    {
        if (extendSelection && !selecting) startSelection();

        if (cursorPos.col < lines[cursorPos.line].size()) cursorPos.col++;
        else if (cursorPos.line < lines.size() - 1)
        {
            cursorPos.line++;
            cursorPos.col = 0;
        }

        if (extendSelection) updateSelection();
        else clearSelection();
    }

    void moveUp(const bool extendSelection = false)
    {
        if (extendSelection && !selecting) startSelection();
        if (cursorPos.line > 0)
        {
            cursorPos.line--;
            cursorPos.col = std::min(cursorPos.col, lines[cursorPos.line].size());
        }

        if (extendSelection) updateSelection();
        else clearSelection();
    }

    void moveDown(const bool extendSelection = false)
    {
        if (extendSelection && !selecting) startSelection();
        if (cursorPos.line < lines.size() - 1)
        {
            cursorPos.line++;
            cursorPos.col = std::min(cursorPos.col, lines[cursorPos.line].size());
        }

        if (extendSelection) updateSelection();
        else clearSelection();
    }

private:
    std::vector<std::string> lines = {""};
    TextPos cursorPos, selectionStart, selectionEnd;
    bool selecting = false;

    void clampCursor()
    {
        if (lines.empty()) lines = {""};

        cursorPos.line = std::min(cursorPos.line, lines.size() - 1);
        cursorPos.col = std::min(cursorPos.col, lines[cursorPos.line].size());
    }

    void deleteAnySelection()
    {
        if (!hasSelection()) return;
        const TextPos start = selectionStartPos();

        if (const TextPos end = selectionEndPos(); start.line == end.line)
        {
            auto& line = lines[start.line];
            line.erase(start.col, end.col - start.col);
        }
        else
        {
            auto &startLine = lines[start.line], &endLine = lines[end.line];

            startLine.erase(start.col);
            endLine.erase(0, end.col);
            startLine += endLine;

            lines.erase(lines.begin() + static_cast<int>(start.line) + 1,
                        lines.begin() + static_cast<int>(end.line) + 1);
        }

        setCursor(start);
    }
};

class TextCursor {
public:
    explicit TextCursor(const TextBuffer& b) : buf(&b) {}

    TextPos cursor() const { return cur; }

    void setCursor(TextPos p, bool clearSel = true) {
        cur = clamp(p);
        if (clearSel) clearSelection();
        if (selecting) updateSelection();
    }

    void moveLeft(bool extendSel=false) {
        beginExtendIfNeeded(extendSel);
        if (cur.col > 0) cur.col--;
        else if (cur.line > 0) { cur.line--; cur.col = lineLen(cur.line); }
        endExtend(extendSel);
    }

    void moveRight(bool extendSel=false) {
        beginExtendIfNeeded(extendSel);
        if (cur.col < lineLen(cur.line)) cur.col++;
        else if (cur.line + 1 < lineCount()) { cur.line++; cur.col = 0; }
        endExtend(extendSel);
    }

    void moveUp(bool extendSel=false) {
        beginExtendIfNeeded(extendSel);
        if (cur.line > 0) { cur.line--; cur.col = std::min(cur.col, lineLen(cur.line)); }
        endExtend(extendSel);
    }

    void moveDown(bool extendSel=false) {
        beginExtendIfNeeded(extendSel);
        if (cur.line + 1 < lineCount()) { cur.line++; cur.col = std::min(cur.col, lineLen(cur.line)); }
        endExtend(extendSel);
    }

    // Selection API
    bool hasSelection() const { return selecting && selA != selB; }
    TextPos selectionStartPos() const { return minPos(selA, selB); }
    TextPos selectionEndPos() const { return maxPos(selA, selB); }

    void startSelection() { selecting = true; selA = selB = cur; }
    void updateSelection() { selB = cur; }
    void clearSelection() { selecting = false; selA = selB = cur; }

private:
    const TextBuffer* buf;
    TextPos cur{}, selA{}, selB{};
    bool selecting = false;

    size_t lineCount() const;
    size_t lineLen(size_t line) const;

    TextPos clamp(TextPos p) const {
        if (lineCount() == 0) return {};
        p.line = std::min(p.line, lineCount() - 1);
        p.col  = std::min(p.col, lineLen(p.line));
        return p;
    }

    void beginExtendIfNeeded(bool extendSel) {
        if (extendSel && !selecting) startSelection();
    }
    void endExtend(bool extendSel) {
        if (extendSel) updateSelection();
        else clearSelection();
    }
};

class TextInput : public Widget
{
public:
    explicit TextInput(Font& font)
    {
        this->font = &font;
        focusableOverride = true;
    }

    float emptyArea = 20.0f, viewportScrollY = 0.0f, viewportH = 0.0f;
    [[nodiscard]] const TextBuffer& buffer() const { return textBuffer; }

    void onKey(const char* key, const KeyAction action)
    {
        if (!focused) return;

        switch (action)
        {
        case KeyAction::Backspace:
            textBuffer.backspace();
            break;
        case KeyAction::Tab:
            textBuffer.insertText("    ");
            break;
        case KeyAction::Enter:
            textBuffer.newLine();
            break;
        case KeyAction::Text:
            if (key && key[0]) textBuffer.insertText(key);
            break;
        default:
            break;
        }

        caretVisible = true;
        caretBlinkTimer = 0.0f;
        bounds.h = std::max(
            bounds.h, static_cast<float>(textBuffer.getLines().size()) * font->textHeight() + emptyArea);
    }

    bool onEvent(const Input::InputEvent& e) override
    {
        if (!visible || !enabled) return false;
        const Rect r = worldBounds();

        if (e.type == Input::InputEvent::Type::Pointer)
        {
            if (draggingSelection && e.pointer.valid)
            {
                textBuffer.setCursor(posFromPointer(e.pointer.x, e.pointer.y), false);
                textBuffer.updateSelection();
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
                    textBuffer.setCursor(posFromPointer(e.pointer.x, e.pointer.y), false);
                    textBuffer.startSelection();
                    draggingSelection = true;

                    return true;
                }
                return false;
            }
            if (!focused) return false;

            switch (e.key)
            {
            case Input::Key::Left:
                textBuffer.moveLeft(false);
                return true;
            case Input::Key::Right:
                textBuffer.moveRight(false);
                return true;
            case Input::Key::Up:
                textBuffer.moveUp(false);
                return true;
            case Input::Key::Down:
                textBuffer.moveDown(false);
                return true;
            default:
                break;
            }
        }

        if (e.type == Input::InputEvent::Type::KeyUp && e.key == Input::Key::A && draggingSelection)
        {
            draggingSelection = false;
            return true;
        }
        return false;
    }

private:
    TextBuffer textBuffer;
    double caretBlinkTimer = 0.0f;
    bool caretVisible = true, draggingSelection = false;

    [[nodiscard]] TextPos posFromPointer(const float px, const float py) const
    {
        const Rect r = worldBounds().inset(10);
        const auto& lines = textBuffer.getLines();
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
        const auto& lines = textBuffer.getLines();
        TextPos selStart = {}, selEnd = {};

        if (textBuffer.hasSelection())
        {
            selStart = textBuffer.selectionStartPos();
            selEnd = textBuffer.selectionEndPos();
        }

        const size_t startLine = std::clamp(static_cast<size_t>(std::floor(viewportScrollY / font->textHeight())),
                                            static_cast<size_t>(0), lines.size()),
                     endLine = std::clamp(
                         static_cast<size_t>(std::ceil((viewportScrollY + viewportH) / font->textHeight())) + 1,
                         static_cast<size_t>(0), lines.size());
        for (size_t i = startLine; i < endLine; ++i)
        {
            const float y = r.y + static_cast<float>(i) * font->textHeight();
            if (textBuffer.hasSelection() && i >= selStart.line && i <= selEnd.line)
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
            TextPos c = textBuffer.cursor();
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

        bounds.h = std::max(
            bounds.h, static_cast<float>(textBuffer.getLines().size()) * font->textHeight() + emptyArea);
    }
};
