#include "./text.h"

TextEditor::TextEditor() : textCursor(textBuffer)
{
}

void TextEditor::setText(const std::string& text)
{
    textBuffer.setText(text);
    textCursor.setCursor({0, 0});
}

std::string TextEditor::getText() const { return textBuffer.getText(); }
const TextBuffer& TextEditor::buffer() const { return textBuffer; }
TextCursor& TextEditor::cursor() { return textCursor; }
const TextCursor& TextEditor::cursor() const { return textCursor; }

void TextEditor::insertText(const std::string& text)
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
    }

    textCursor.setCursor(c, true);
}

void TextEditor::backspace()
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

void TextEditor::newLine()
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

TextCursor::State TextEditor::cursorState() const { return textCursor.getState(); }
void TextEditor::setCursorState(const TextCursor::State& state) { textCursor.setState(state); }

TextEditor::Range TextEditor::selectionRange() const
{
    if (!textCursor.hasSelection()) return {textCursor.cursor(), textCursor.cursor()};
    return {textCursor.selectionStartPos(), textCursor.selectionEndPos()};
}

std::string TextEditor::getTextInRange(const Range& range) const
{
    const auto& lines = textBuffer.getLines();
    if (lines.empty()) return {};

    const TextPos a = minPos(range.start, range.end), b = maxPos(range.start, range.end);
    if (a == b) return {};
    std::string out;

    if (a.line == b.line)
    {
        out = lines[a.line].substr(a.col, b.col - a.col);
        return out;
    }

    out += lines[a.line].substr(a.col);
    out += '\n';

    for (size_t line = a.line + 1; line < b.line; ++line)
    {
        out += lines[line];
        out += '\n';
    }

    out += lines[b.line].substr(0, b.col);
    return out;
}

void TextEditor::deleteRange(const Range& range)
{
    if (textBuffer.getLines().empty()) return;

    const TextPos a = minPos(range.start, range.end), b = maxPos(range.start, range.end);
    if (a == b) return;
    auto& lines = textBuffer.getLines();

    if (a.line == b.line)
    {
        auto& line = lines[a.line];
        line.erase(a.col, b.col - a.col);
    }
    else
    {
        auto& startLine = lines[a.line];
        const auto& endLine = lines[b.line];

        const std::string tail = endLine.substr(b.col);

        startLine.erase(a.col);
        startLine += tail;

        lines.erase(lines.begin() + static_cast<int>(a.line) + 1, lines.begin() + static_cast<int>(b.line) + 1);
    }

    if (lines.empty()) lines = {""};
    textCursor.setCursor(a, true);
}

void TextEditor::insertTextAt(const TextPos pos, const std::string& text)
{
    auto& lines = textBuffer.getLines();
    if (lines.empty()) lines = {""};

    TextPos cur = pos;
    for (const char c : text)
    {
        if (c == '\r') continue;
        if (c == '\n')
        {
            auto& line = lines[cur.line];
            const std::string tail = line.substr(cur.col);

            line.erase(cur.col);
            lines.insert(lines.begin() + static_cast<int>(cur.line) + 1, tail);

            cur.line++;
            cur.col = 0;

            continue;
        }

        auto& line = lines[cur.line];

        line.insert(cur.col, 1, c);
        cur.col++;
    }

    textCursor.setCursor(cur, true);
}

void TextEditor::deleteAnySelection()
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
