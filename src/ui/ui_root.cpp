#include "./ui_root.h"
#include "../platform/path.h"

#include <sys/stat.h>

UIRoot::UIRoot(const float screenW, const float screenH, Font& codeFont, Font& uiFont)
{
    this->screenW = screenW;
    this->screenH = screenH;
    root->font = &uiFont;

    left = root->addChild<Panel>();
    center = root->addChild<Panel>();
    bottom = root->addChild<Panel>();

    editorScroll = center->addChild<ScrollView>();
    editor = editorScroll->addChild<TextInput>(codeFont);
    editor->onContextMenu = [this](const float x, const float y)
    {
        if (!contextMenu) return;
        contextMenu->openAt(x, y, {
                                {"Cut", [this] { if (editor) editor->cutText(); }},
                                {"Copy", [this] { if (editor) editor->copyText(); }},
                                {"Paste", [this] { if (editor) editor->pasteText(); }},
                                {"Select All", [this] { if (editor) editor->selectAll(); }}
                            }, this->screenW, this->screenH);
    };

    editorScroll->content = editor;
    editorScroll->barY = editorScroll->addChild<ScrollBar>(BoxDir::Vertical);
    editorScroll->barY->scrollAmount = codeFont.textHeight();

    fileListScroll = left->addChild<ScrollView>();
    fileList = fileListScroll->addChild<List>();
    fileList->onItemSelected = [this](const std::string&)
    {
        if (!fileList) return;
        const int i = fileList->selected;
        if (i < 0 || i >= static_cast<int>(currentEntries.size())) return;

        const auto& it = currentEntries[i];
        if (it.isUp)
        {
            if (!inSubdir()) return;
            std::string d = currentDir;
            while (d.size() > 1 && d.back() == '/') d.pop_back();

            if (const auto slash = d.find_last_of('/'); slash != std::string::npos)
            {
                d = d.substr(0, slash + 1);
                if (d.rfind(FileSystem::workspaceRoot, 0) == 0) currentDir = d;
            }

            refreshFileList();
            return;
        }

        const std::string path = FileSystem::join(currentDir, it.name);
        if (it.isDir)
        {
            std::string next = FileSystem::normalize(path);
            if (next.back() != '/') next.push_back('/');
            currentDir = next;
            refreshFileList();

            return;
        }

        if (editor) editor->loadFile(path);
    };
    fileList->onContextMenu = [this](const float x, const float y)
    {
        if (!contextMenu) return;
        auto selectedIndex = [this]() -> int
        {
            if (!fileList) return -1;
            const int i = fileList->selected;
            if (i < 0 || i >= static_cast<int>(currentEntries.size())) return -1;
            if (currentEntries[i].isUp) return -1;

            return i;
        };

        auto selectedName = [this, selectedIndex]() -> const ListItem*
        {
            const int i = selectedIndex();
            if (i < 0) return nullptr;

            return &currentEntries[i];
        };

        auto selectedPath = [this, selectedName]() -> std::string
        {
            const auto* item = selectedName();
            if (!item) return "";

            return FileSystem::join(currentDir, item->name);
        };

        auto newFileAction = [this](const std::string& raw)
        {
            std::string err;
            const std::string name = sanitize(raw, &err);

            if (name.empty())
            {
                if (modal) modal->showMessage("Invalid Name", err);
                return;
            }

            const std::string path = currentDir + uniqueName(currentDir, name);
            if (const std::vector<uint8_t> data; !FileSystem::writeFile(path, data))
            {
                if (modal) modal->showMessage("Error", "Failed to create file.");
                return;
            }

            refreshFileList();
        };

        auto newFolderAction = [this](const std::string& raw)
        {
            std::string err;
            const std::string name = sanitize(raw, &err);

            if (name.empty())
            {
                if (modal) modal->showMessage("Invalid Name", err);
                return;
            }

            if (!FileSystem::makeDir(currentDir + uniqueName(currentDir, name)))
            {
                if (modal) modal->showMessage("Error", "Failed to create folder.");
                return;
            }

            refreshFileList();
        };

        auto renameAction = [this, selectedPath, selectedName]
        {
            const std::string oldPath = selectedPath();
            const auto* item = selectedName();
            if (oldPath.empty() || !item || !modal) return;

            modal->showInput("Rename", "Enter new name:", item->name, [this, oldPath, item](const std::string& raw)
            {
                std::string err;
                const std::string newName = sanitize(raw, &err);

                if (newName.empty())
                {
                    if (modal) modal->showMessage("Invalid Name", err);
                    return;
                }
                if (newName == item->name) return;

                const std::string newPath = currentDir + newName;
                if (FileSystem::exists(newPath))
                {
                    if (modal) modal->showMessage("Rename", "A file/folder with that name already exists.");
                    return;
                }

                if (!FileSystem::renamePath(oldPath, newPath))
                {
                    if (modal) modal->showMessage("Rename", "Failed to rename file/folder.");
                    return;
                }

                refreshFileList();
            });
        };

        auto copyAction = [this, selectedPath, selectedName]
        {
            const auto* item = selectedName();
            if (!item) return;

            fileClipboard.path = selectedPath();
            fileClipboard.name = item->name;
            fileClipboard.isDir = item->isDir;
            fileClipboard.valid = true;
        };

        auto pasteAction = [this]
        {
            if (!fileClipboard.valid) return;

            std::string err;
            const std::string base = sanitize(fileClipboard.name, &err);
            if (base.empty()) return;

            const std::string dest = currentDir + uniqueName(currentDir, base);
            if (fileClipboard.isDir)
            {
                std::string src = FileSystem::normalize(fileClipboard.path), d = FileSystem::normalize(dest);

                if (src.back() != '/') src.push_back('/');
                if (d.back() != '/') d.push_back('/');

                if (d.rfind(src, 0) == 0)
                {
                    if (modal) modal->showMessage("Error", "Cannot paste a folder into itself.");
                    return;
                }
            }

            if (!FileSystem::copyPath(fileClipboard.path, dest))
            {
                if (modal) modal->showMessage("Error", "Failed to paste file/folder.");
                return;
            }

            refreshFileList();
        };

        auto deleteAction = [this, selectedPath, selectedName]
        {
            const std::string path = selectedPath();
            const auto* item = selectedName();
            if (path.empty() || !item || !modal) return;

            modal->showConfirm("Delete", "Delete \"" + item->name + (item->isDir ? "/\" ?" : "\" ?"), [this, path]
            {
                if (!FileSystem::removePath(path))
                {
                    if (modal) modal->showMessage("Delete", "Failed to delete file/folder.");
                    return;
                }
                refreshFileList();
            });
        };

        auto propertiesAction = [this, selectedPath, selectedName]
        {
            const std::string path = selectedPath();
            const auto* item = selectedName();
            if (path.empty() || !item || !modal) return;

            std::string msg = "Name: " + item->name + (item->isDir ? "/" : "") + "\nFolder: " + currentDir + "\nPath: "
                + path + "\nType: " + (item->isDir ? "Folder\n" : "File\n");
            struct stat st = {};
            if (stat(path.c_str(), &st) == 0)
                msg += "Size: " + std::to_string(static_cast<uint64_t>(st.st_size)) + " B\n";

            modal->showMessage("Properties", msg);
        };

        contextMenu->openAt(x, y, {
                                {
                                    "New File",
                                    [this, newFileAction]
                                    {
                                        if (!modal) return;
                                        modal->showInput("New File", "Enter file name:", "New File.txt", newFileAction);
                                    }
                                },
                                {
                                    "New Folder",
                                    [this, newFolderAction]
                                    {
                                        if (!modal) return;
                                        modal->showInput("New Folder", "Enter folder name:", "New Folder",
                                                         newFolderAction);
                                    }
                                },
                                {"", nullptr},
                                {"Rename", renameAction},
                                {"Copy", copyAction},
                                {"Paste", pasteAction},
                                {"Delete", deleteAction},
                                {"", nullptr},
                                {"Properties", propertiesAction}
                            }, this->screenW, this->screenH);
    };

    fileListScroll->content = fileList;
    fileListScroll->barY = fileListScroll->addChild<ScrollBar>(BoxDir::Vertical);
    fileListScroll->barY->scrollAmount = fileList->rowH;

    keyboard = bottom->addChild<Keyboard>(uiFont);
    keyboard->onKey = [this](const char* key, const KeyAction action)
    {
        if (modal && modal->isOpen())
        {
            modal->onKey(key, action);
            return;
        }

        if (editor && !editor->focused) setFocus(editor, false);
        if (editor) editor->onKey(key, action);
    };
    contextMenu = root->addChild<ContextMenu>();
    modal = root->addChild<Modal>();

    refreshFileList();
    rebuildFocusList();

    if (editor && editor->isFocusable()) setFocus(editor, false);
    else if (!focusableWidgets.empty()) setFocus(focusableWidgets[0], false);
}

void UIRoot::layout() const
{
    root->bounds = Rect({0, 0, screenW, screenH});
    Rect content = root->bounds;
    const float leftW = showLeft ? 200.0f : 0.0f, bottomH = showBottom ? 140.0f : 0.0f;

    left->visible = showLeft;
    left->bounds = leftW > 0.0f ? content.takeLeft(leftW) : Rect::empty();
    fileListScroll->visible = showLeft;
    fileListScroll->bounds = Rect({0, 0, left->bounds.w, left->bounds.h}).inset(10);
    if (fileListScroll->barY) fileListScroll->barY->layout.fixedHeight = fileListScroll->bounds.h;

    bottom->visible = showBottom;
    bottom->bounds = bottomH > 0.0f ? content.takeBottom(bottomH) : Rect::empty();
    keyboard->visible = showBottom;
    keyboard->bounds = Rect({0, 0, bottom->bounds.w, bottom->bounds.h}).inset(10);

    center->bounds = content;
    editorScroll->bounds = Rect({0, 0, center->bounds.w, center->bounds.h}).inset(10);
    if (editorScroll->barY) editorScroll->barY->layout.fixedHeight = editorScroll->bounds.h;
    editor->bounds.w = editorScroll->bounds.w;
}

void UIRoot::update(const double dt)
{
    rebuildFocusList();

    if (focusedWidget && (!focusedWidget->visible || !focusedWidget->enabled))
    {
        focusedWidget->focused = false;
        focusedWidget->showFocus = false;
        focusedWidget = nullptr;

        if (editor && editor->isFocusable()) setFocus(editor, false);
        else if (!focusableWidgets.empty()) setFocus(focusableWidgets[0], false);
    }
    if (hoverWidget && (!hoverWidget->visible || !hoverWidget->enabled)) hoverWidget = nullptr;

    root->update(dt);
}

void UIRoot::routeEvent(const Input::InputEvent& e)
{
    if (modal && modal->isOpen())
    {
        modal->onEvent(e);
        if (e.type != Input::InputEvent::Type::Pointer) return;
    }

    if (e.type == Input::InputEvent::Type::Pointer)
    {
        pointer = e.pointer;
        if (captureWidget)
        {
            captureWidget->onEvent(e);
            hoverWidget = captureWidget;

            return;
        }

        Widget* prevHover = hoverWidget;
        hoverWidget = root->hitTest(pointer.x, pointer.y);

        if (prevHover && prevHover != hoverWidget) prevHover->onEvent(e);
        if (hoverWidget) hoverWidget->onEvent(e);
        if (focusedWidget) focusedWidget->showFocus = false;

        return;
    }

    if (e.type == Input::InputEvent::Type::Scroll)
    {
        for (Widget* w = hoverWidget; w; w = w->parent) if (w->onEvent(e)) return;
        for (Widget* w = focusedWidget; w; w = w->parent) if (w->onEvent(e)) return;

        return;
    }

    if (e.type == Input::InputEvent::Type::KeyUp || e.type == Input::InputEvent::Type::KeyDown)
    {
        Input::InputEvent ke = e;
        ke.pointer = pointer;

        if (e.key == Input::Key::A)
        {
            if (contextMenu && contextMenu->isOpen() && e.type == Input::InputEvent::Type::KeyDown && !(pointer.valid &&
                contextMenu->worldBounds().contains(pointer.x, pointer.y)))
            {
                contextMenu->close();
                return;
            }

            if (e.type == Input::InputEvent::Type::KeyDown)
            {
                if (!pointer.valid) return;
                captureWidget = root->hitTest(pointer.x, pointer.y);
                if (!captureWidget) return;

                bool clickedKeyboard = false;
                for (const Widget* p = captureWidget; p; p = p->parent)
                    if (p == keyboard)
                    {
                        clickedKeyboard = true;
                        break;
                    }

                if (captureWidget->isFocusable() && !clickedKeyboard) setFocus(captureWidget, false);
                captureWidget->onEvent(ke);

                return;
            }

            if (captureWidget)
            {
                captureWidget->onEvent(ke);
                captureWidget = nullptr;
            }
        }

        if (captureWidget && captureWidget->onEvent(ke)) return;
        if (focusedWidget && focusedWidget->onEvent(ke)) return;
        if (hoverWidget && hoverWidget->onEvent(ke)) return;

        if (e.type == Input::InputEvent::Type::KeyDown)
        {
            if (e.key == Input::Key::Home)
            {
                quit = true;
                return;
            }

            if (e.key == Input::Key::One)
            {
                if (showLeft && showBottom)
                {
                    showLeft = true;
                    showBottom = false;
                }
                else if (showLeft && !showBottom)
                {
                    showLeft = false;
                    showBottom = true;
                }
                else if (!showLeft && showBottom)
                {
                    showLeft = false;
                    showBottom = false;
                }
                else
                {
                    showLeft = true;
                    showBottom = true;
                }

                return;
            }

            if (e.key == Input::Key::Two)
            {
                if (!pointer.valid) return;

                Widget* w = root->hitTest(pointer.x, pointer.y);
                if (!w) w = hoverWidget;

                for (Widget* p = w; p; p = p->parent)
                {
                    if (const auto* textInput = dynamic_cast<TextInput*>(p))
                    {
                        if (textInput->onContextMenu) textInput->onContextMenu(pointer.x, pointer.y);
                        return;
                    }
                    if (const auto* list = dynamic_cast<List*>(p))
                    {
                        if (list->onContextMenu) list->onContextMenu(pointer.x, pointer.y);
                        return;
                    }
                }

                return;
            }

            if (e.key == Input::Key::Up || e.key == Input::Key::Down || e.key == Input::Key::Left ||
                e.key == Input::Key::Right)
            {
                if (focusedWidget == editor || focusedWidget == fileList) return;

                Widget* next = findNextFocusable(e.key == Input::Key::Left ? -1 : e.key == Input::Key::Right ? 1 : 0,
                                                 e.key == Input::Key::Up ? -1 : e.key == Input::Key::Down ? 1 : 0);
                if (next) setFocus(next, true);
            }
        }
    }
}

void UIRoot::draw() const { root->draw(); }

void UIRoot::FileClipboard::clear()
{
    path.clear();
    name.clear();
    isDir = false;
    valid = false;
}

bool UIRoot::inSubdir() const { return currentDir != FileSystem::workspaceRoot; }

void UIRoot::refreshFileList()
{
    if (!fileList) return;

    currentEntries.clear();
    fileList->items.clear();

    if (inSubdir())
    {
        currentEntries.push_back({"../", "..", true, true});
        fileList->items.emplace_back("../");
    }

    std::vector<FileSystem::DirEntry> entries;
    if (!FileSystem::listDir(currentDir, entries, true)) return;

    for (const auto& e : entries)
    {
        ListItem item({e.name + (e.isDir ? "/" : ""), e.name, e.isDir, false});

        currentEntries.push_back(item);
        fileList->items.push_back(item.label);
    }
    fileList->selected = -1;
}

std::string UIRoot::uniqueName(const std::string& dir, const std::string& name)
{
    auto base = name;
    auto ext = std::string();

    if (name.find('.') != std::string::npos)
    {
        if (const auto dot = name.find_last_of('.'); dot != std::string::npos && dot != 0)
        {
            base = name.substr(0, dot);
            ext = name.substr(dot);
        }
    }

    std::string candidate = name;
    int n = 2;

    while (FileSystem::exists(dir + candidate))
    {
        candidate = base;
        candidate += " (";
        candidate += std::to_string(n++);
        candidate += ")";
        candidate += ext;
    }
    return candidate;
}

void UIRoot::setFocus(Widget* w, const bool show)
{
    if (!w || !w->isFocusable()) return;
    if (focusedWidget && focusedWidget == w)
    {
        focusedWidget->showFocus = show;
        return;
    }
    if (focusedWidget)
    {
        focusedWidget->focused = false;
        focusedWidget->showFocus = false;
    }

    focusedWidget = w;
    focusedWidget->focused = true;
    focusedWidget->showFocus = show;
}

void UIRoot::rebuildFocusList()
{
    focusableWidgets.clear();
    root->collectFocusable(focusableWidgets);
}

Widget* UIRoot::findNextFocusable(const int dirX, const int dirY) const
{
    if (focusableWidgets.empty()) return nullptr;
    if (!focusedWidget) return focusableWidgets[0];

    const int dxDir = dirX, dyDir = dirY;
    if (dxDir == 0 && dyDir == 0) return focusedWidget;

    const Rect from = focusedWidget->worldBounds();
    Widget* best = nullptr;
    float bestScore = std::numeric_limits<float>::infinity();

    for (Widget* w : focusableWidgets)
    {
        if (!w || w == focusedWidget) continue;

        const Rect to = w->worldBounds();
        const float dx = to.x + to.w * 0.5f - (from.x + from.w * 0.5f),
                    dy = to.y + to.h * 0.5f - (from.y + from.h * 0.5f);
        if ((dxDir > 0 && dx <= 0) || (dxDir < 0 && dx >= 0) || (dyDir > 0 && dy <= 0) || (dyDir < 0 && dy >= 0))
            continue;

        float forward = 0.0f, lateral = 0.0f;
        if (dxDir != 0)
        {
            forward = dxDir > 0 ? to.x - (from.x + from.w) : from.x - (to.x + to.w);
            lateral = std::abs(dy);
        }
        else
        {
            forward = dyDir > 0 ? to.y - (from.y + from.h) : from.y - (to.y + to.h);
            lateral = std::abs(dx);
        }

        if (forward <= 0.0f) continue;
        if (const float score = forward + lateral * 0.35f; score < bestScore)
        {
            bestScore = score;
            best = w;
        }
    }

    if (best) return best;
    const auto it = std::find(focusableWidgets.begin(), focusableWidgets.end(), focusedWidget);
    if (it == focusableWidgets.end()) return focusableWidgets[0];

    int i = it - focusableWidgets.begin(), step = 0;

    if (dxDir > 0 || dyDir > 0) step = 1;
    else step = -1;

    i = (i + step) % static_cast<int>(focusableWidgets.size());
    if (i < 0) i += static_cast<int>(focusableWidgets.size());

    return focusableWidgets[i];
}
