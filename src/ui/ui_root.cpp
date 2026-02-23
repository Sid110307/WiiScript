#include "./ui_root.h"

UIRoot::UIRoot(Font& codeFont, Font& uiFont)
{
    root->font = &uiFont;

    left = root->addChild<Panel>();
    center = root->addChild<Panel>();
    bottom = root->addChild<Panel>();

    editorScroll = center->addChild<ScrollView>();
    editor = editorScroll->addChild<TextInput>(codeFont);
    editorScroll->content = editor;
    editorScroll->barY = editorScroll->addChild<ScrollBar>(BoxDir::Vertical);
    editorScroll->barY->scrollAmount = codeFont.textHeight();

    fileListScroll = left->addChild<ScrollView>();
    fileList = fileListScroll->addChild<List>();
    fileListScroll->content = fileList;
    fileListScroll->barY = fileListScroll->addChild<ScrollBar>(BoxDir::Vertical);
    fileListScroll->barY->scrollAmount = fileList->rowH;

    keyboard = bottom->addChild<Keyboard>(uiFont);
    keyboard->onKey = [this](const char* key, const KeyAction action) { if (editor) editor->onKey(key, action); };

    if (std::vector<FileSystem::DirEntry> entries; FileSystem::listDir(FileSystem::workspaceRoot, entries, true))
    {
        fileList->items.clear();
        for (const auto& e : entries) fileList->items.push_back(e.name + (e.isDir ? "/" : ""));
    }

    for (int i = 1; i <= 150; ++i) fileList->items.push_back("File " + std::to_string(i));

    rebuildFocusList();
    if (editor && editor->isFocusable()) setFocus(editor, false);
    else if (!focusableWidgets.empty()) setFocus(focusableWidgets[0], false);
}

void UIRoot::layout(const float screenW, const float screenH) const
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

    if (e.type == Input::InputEvent::Type::KeyDown || e.type == Input::InputEvent::Type::KeyUp)
    {
        Input::InputEvent ke = e;
        ke.pointer = pointer;

        if (e.key == Input::Key::A)
        {
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

    int idx = it - focusableWidgets.begin(), step = 0;

    if (dxDir > 0 || dyDir > 0) step = 1;
    else step = -1;

    idx = (idx + step) % static_cast<int>(focusableWidgets.size());
    if (idx < 0) idx += static_cast<int>(focusableWidgets.size());

    return focusableWidgets[idx];
}
