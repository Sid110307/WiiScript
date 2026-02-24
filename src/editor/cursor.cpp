#include "./text.h"
#include <algorithm>

TextCursor::TextCursor(const TextBuffer& buffer) { textBuffer = &buffer; }
TextPos TextCursor::cursor() const { return cursorPos; }

void TextCursor::setCursor(const TextPos pos, const bool clearSel)
{
    cursorPos = pos;
    clampCursor();

    if (clearSel) clearSelection();
    if (selecting) updateSelection();
}

bool TextCursor::hasSelection() const { return selecting && selectionStart != selectionEnd; }
TextPos TextCursor::selectionStartPos() const { return minPos(selectionStart, selectionEnd); }
TextPos TextCursor::selectionEndPos() const { return maxPos(selectionStart, selectionEnd); }

void TextCursor::startSelection()
{
    selecting = true;
    selectionStart = selectionEnd = cursorPos;
}

void TextCursor::updateSelection() { selectionEnd = cursorPos; }

void TextCursor::clearSelection()
{
    selecting = false;
    selectionStart = selectionEnd = cursorPos;
}

void TextCursor::moveLeft(const bool extendSelection)
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

void TextCursor::moveRight(const bool extendSelection)
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

void TextCursor::moveUp(const bool extendSelection)
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

void TextCursor::moveDown(const bool extendSelection)
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

TextCursor::State TextCursor::getState() const { return {cursorPos, selectionStart, selectionEnd, selecting}; }

void TextCursor::setState(const State& state)
{
    cursorPos = state.cursorPos;
    selectionStart = state.selectionStart;
    selectionEnd = state.selectionEnd;
    selecting = state.selecting;

    clampCursor();
}

void TextCursor::clampCursor()
{
    if (!textBuffer || textBuffer->lineCount() == 0)
    {
        cursorPos = {0, 0};
        selectionStart = selectionEnd = cursorPos;
        selecting = false;

        return;
    }

    cursorPos.line = std::min(cursorPos.line, textBuffer->lineCount() - 1);
    cursorPos.col = std::min(cursorPos.col, textBuffer->lineLength(cursorPos.line));
}
