#pragma once

#include <string>
#include <vector>

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

struct Clipboard
{
    std::string text;

    void clear() { text.clear(); }
    [[nodiscard]] bool empty() const { return text.empty(); }
};

class TextBuffer
{
public:
    void setText(const std::string& text);
    [[nodiscard]] std::string getText() const;

    [[nodiscard]] const std::vector<std::string>& getLines() const;
    [[nodiscard]] std::vector<std::string>& getLines();
    [[nodiscard]] size_t lineCount() const;
    [[nodiscard]] size_t lineLength(size_t line) const;

private:
    std::vector<std::string> lines = {""};
};

class TextCursor
{
public:
    explicit TextCursor(const TextBuffer& buffer);

    [[nodiscard]] TextPos cursor() const;
    void setCursor(TextPos pos, bool clearSel = true);

    [[nodiscard]] bool hasSelection() const;
    [[nodiscard]] TextPos selectionStartPos() const;
    [[nodiscard]] TextPos selectionEndPos() const;

    void startSelection();
    void updateSelection();
    void clearSelection();

    void moveLeft(bool extendSelection = false);
    void moveRight(bool extendSelection = false);
    void moveUp(bool extendSelection = false);
    void moveDown(bool extendSelection = false);

    struct State
    {
        TextPos cursorPos, selectionStart, selectionEnd;
        bool selecting = false;
    };

    [[nodiscard]] State getState() const;
    void setState(const State& state);

private:
    const TextBuffer* textBuffer = nullptr;
    TextPos cursorPos, selectionStart, selectionEnd;
    bool selecting = false;

    void clampCursor();
};

class TextEditor
{
public:
    TextEditor();

    void setText(const std::string& text);
    [[nodiscard]] std::string getText() const;
    [[nodiscard]] const TextBuffer& buffer() const;
    [[nodiscard]] TextCursor& cursor();
    [[nodiscard]] const TextCursor& cursor() const;

    void insertText(const std::string& text);
    void backspace();
    void newLine();

    struct Range
    {
        TextPos start, end;
    };

    [[nodiscard]] TextCursor::State cursorState() const;
    void setCursorState(const TextCursor::State& state);

    [[nodiscard]] Range selectionRange() const;
    [[nodiscard]] std::string getTextInRange(const Range& range) const;
    void deleteRange(const Range& range);
    void insertTextAt(TextPos pos, const std::string& text);

private:
    TextBuffer textBuffer;
    TextCursor textCursor;

    void deleteAnySelection();
};
