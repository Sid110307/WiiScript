#pragma once

#include <vector>
#include <memory>

#include "./text.h"

struct EditCommand
{
    virtual ~EditCommand() = default;
    virtual void execute(TextEditor& editor) = 0;
    virtual void undo(TextEditor& editor) = 0;
};

class InsertCommand : public EditCommand
{
public:
    InsertCommand(const TextPos at, std::string text, const TextCursor::State& before)
        : at(at), text(std::move(text)), before(before)
    {
    }

    void execute(TextEditor& editor) override
    {
        editor.setCursorState(before);
        editor.insertTextAt(at, text);
        after = editor.cursorState();
    }

    void undo(TextEditor& editor) override
    {
        editor.setCursorState(after);
        editor.deleteRange({at, after.cursorPos});
        editor.setCursorState(before);
    }

    TextPos at = {};
    std::string text;
    TextCursor::State before = {}, after = {};
};

class DeleteCommand : public EditCommand
{
public:
    DeleteCommand(const TextPos from, const TextPos to, std::string text, const TextCursor::State& before)
        : from(minPos(from, to)), to(maxPos(from, to)), text(std::move(text)), before(before)
    {
    }

    void execute(TextEditor& editor) override
    {
        editor.setCursorState(before);
        editor.deleteRange({from, to});
        after = editor.cursorState();
    }

    void undo(TextEditor& editor) override
    {
        editor.setCursorState(after);
        editor.insertTextAt(from, text);
        editor.setCursorState(before);
    }

    TextPos from, to;
    std::string text;
    TextCursor::State before = {}, after = {};
};

class CommandHistory
{
public:
    size_t maxSize = 128;

    void clear()
    {
        undoStack.clear();
        redoStack.clear();
    }

    void execute(TextEditor& editor, std::unique_ptr<EditCommand> command)
    {
        if (!command) return;

        command->execute(editor);
        undoStack.push_back(std::move(command));
        redoStack.clear();

        if (undoStack.size() > maxSize) undoStack.erase(undoStack.begin());
    }

    [[nodiscard]] bool canUndo() const { return !undoStack.empty(); }
    [[nodiscard]] bool canRedo() const { return !redoStack.empty(); }

    void undo(TextEditor& editor)
    {
        if (undoStack.empty()) return;

        auto command = std::move(undoStack.back());
        undoStack.pop_back();
        command->undo(editor);
        redoStack.push_back(std::move(command));
    }

    void redo(TextEditor& editor)
    {
        if (redoStack.empty()) return;

        auto command = std::move(redoStack.back());
        redoStack.pop_back();
        command->execute(editor);
        undoStack.push_back(std::move(command));
    }

private:
    std::vector<std::unique_ptr<EditCommand>> undoStack, redoStack;
};
