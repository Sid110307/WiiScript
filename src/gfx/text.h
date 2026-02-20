#pragma once

#include <string>
#include <vector>
#include <algorithm>

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
        if (lines.empty()) lines = {""};
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
    [[nodiscard]] std::vector<std::string>& getLines() { return lines; }

    [[nodiscard]] size_t lineCount() const { return lines.empty() ? 1 : lines.size(); }

    [[nodiscard]] size_t lineLength(const size_t line) const
    {
        return lines.empty() ? 0 : lines[std::min(line, lines.size() - 1)].size();
    }

private:
    std::vector<std::string> lines = {""};
};

class TextCursor
{
public:
    explicit TextCursor(const TextBuffer& buffer) { textBuffer = &buffer; }
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

    void moveLeft(const bool extendSelection = false)
    {
        if (extendSelection && !selecting) startSelection();

        if (cursorPos.col > 0) cursorPos.col--;
        else if (cursorPos.line > 0)
        {
            cursorPos.line--;
            cursorPos.col = textBuffer->lineLength(cursorPos.line);
        }

        if (extendSelection) updateSelection();
        else clearSelection();
    }

    void moveRight(const bool extendSelection = false)
    {
        if (extendSelection && !selecting) startSelection();

        if (cursorPos.col < textBuffer->lineLength(cursorPos.line)) cursorPos.col++;
        else if (cursorPos.line < textBuffer->lineCount() - 1)
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
            cursorPos.col = std::min(cursorPos.col, textBuffer->lineLength(cursorPos.line));
        }

        if (extendSelection) updateSelection();
        else clearSelection();
    }

    void moveDown(const bool extendSelection = false)
    {
        if (extendSelection && !selecting) startSelection();
        if (cursorPos.line < textBuffer->lineCount() - 1)
        {
            cursorPos.line++;
            cursorPos.col = std::min(cursorPos.col, textBuffer->lineLength(cursorPos.line));
        }

        if (extendSelection) updateSelection();
        else clearSelection();
    }

private:
    const TextBuffer* textBuffer = nullptr;
    TextPos cursorPos, selectionStart, selectionEnd;
    bool selecting = false;

    void clampCursor()
    {
        cursorPos.line = std::min(cursorPos.line, textBuffer->lineCount() - 1);
        cursorPos.col = std::min(cursorPos.col, textBuffer->lineLength(cursorPos.line));
    }
};

class TextEditor
{
public:
    TextEditor() : textCursor(textBuffer)
    {
    }

    void setText(const std::string& text)
    {
        textBuffer.setText(text);
        textCursor.setCursor({0, 0});
    }

    [[nodiscard]] std::string getText() const { return textBuffer.getText(); }
    [[nodiscard]] const TextBuffer& buffer() const { return textBuffer; }
    [[nodiscard]] TextCursor& cursor() { return textCursor; }
    [[nodiscard]] const TextCursor& cursor() const { return textCursor; }

    void insertText(const std::string& text)
    {
        deleteAnySelection();

        auto& lines = textBuffer.getLines();
        TextPos c = textCursor.cursor();

        for (const char ch : text)
        {
            if (ch == '\r') continue;
            if (ch == '\n')
            {
                newLine();
                c = textCursor.cursor();

                continue;
            }

            auto& line = lines[c.line];

            line.insert(c.col, 1, ch);
            c.col++;
            textCursor.setCursor(c, true);
        }
    }

    void backspace()
    {
        if (textCursor.hasSelection())
        {
            deleteAnySelection();
            return;
        }

        auto& lines = textBuffer.getLines();

        if (TextPos c = textCursor.cursor(); c.col > 0)
        {
            auto& line = lines[c.line];

            line.erase(c.col - 1, 1);
            c.col--;
            textCursor.setCursor(c, true);
        }
        else if (c.line > 0)
        {
            const std::string prevLine = lines[c.line - 1];

            lines[c.line - 1] += lines[c.line];
            lines.erase(lines.begin() + static_cast<int>(c.line));
            c.line--;
            c.col = prevLine.size();
            textCursor.setCursor(c, true);
        }
    }

    void newLine()
    {
        deleteAnySelection();

        auto& lines = textBuffer.getLines();
        TextPos c = textCursor.cursor();
        auto& line = lines[c.line];
        const std::string nextLine = line.substr(c.col);

        line.erase(c.col);
        lines.insert(lines.begin() + static_cast<int>(c.line) + 1, nextLine);
        c.line++;
        c.col = 0;
        textCursor.setCursor(c, true);
    }

private:
    TextBuffer textBuffer;
    TextCursor textCursor;

    void deleteAnySelection()
    {
        if (!textCursor.hasSelection()) return;

        auto& lines = textBuffer.getLines();
        const TextPos start = textCursor.selectionStartPos();

        if (const TextPos end = textCursor.selectionEndPos(); start.line == end.line)
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

        textCursor.setCursor(start, true);
    }
};
