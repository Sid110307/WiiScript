#include "./ui_root.h"
#include "./widgets/layout.h"
#include "./widgets/scrollbars.h"

UIRoot::UIRoot(Font& codeFont, Font& uiFont)
{
    (void)codeFont;
    root->font = &uiFont;

    left = root->addChild<Panel>();
    center = root->addChild<Panel>();
    bottom = root->addChild<Panel>();

    toolbar = center->addChild<Box>(BoxDir::Horizontal);
    toolbar->padding = 10;
    toolbar->gap = 10;

    btnRun = toolbar->addChild<Button>("Run");
    btnStop = toolbar->addChild<Button>("Stop");
    btnSave = toolbar->addChild<Button>("Save");

    fileListScroll = left->addChild<ScrollView>();
    fileList = fileListScroll->addChild<List>();
    fileListScroll->content = fileList;
    fileListScroll->barY = fileListScroll->addChild<ScrollBar>(BoxDir::Vertical);
    fileListScroll->barY->scrollAmount = fileList->rowH;
    keyboard = bottom->addChild<Keyboard>(uiFont);

    if (std::vector<FileSystem::DirEntry> entries; FileSystem::listDir(FileSystem::workspaceRoot, entries, true))
    {
        fileList->items.clear();
        for (const auto& e : entries) fileList->items.push_back(e.name + (e.isDir ? "/" : ""));
    }

    for (int i = 1; i <= 150; ++i) fileList->items.push_back("Item " + std::to_string(i));

    rebuildFocusList();
    if (!focusableWidgets.empty()) setFocus(focusableWidgets[0], false);
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
    toolbar->bounds = Rect({0, 0, center->bounds.w, toolbar->layout.fixedHeight});
}

void UIRoot::update(const double dt)
{
    rebuildFocusList();

    if (focusedWidget && (!focusedWidget->visible || !focusedWidget->enabled))
    {
        focusedWidget->focused = false;
        focusedWidget->showFocus = false;
        focusedWidget = nullptr;
        if (!focusableWidgets.empty()) setFocus(focusableWidgets[0], false);
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

    if (e.type == Input::InputEvent::Type::KeyDown)
    {
        if (e.key == Input::Key::Home)
        {
            quit = true;
            return;
        }

        if (e.key == Input::Key::One)
        {
            showLeft = !showLeft;
            return;
        }

        if (e.key == Input::Key::Two)
        {
            showBottom = !showBottom;
            return;
        }

        if (e.key == Input::Key::Up || e.key == Input::Key::Down || e.key == Input::Key::Left ||
            e.key == Input::Key::Right)
        {
            if (focusedWidget)
            {
                Input::InputEvent nav = e;
                nav.pointer = pointer;

                if (focusedWidget->onEvent(nav)) return;
            }

            Widget* next = findNextFocusable(e.key == Input::Key::Left ? -1 : e.key == Input::Key::Right ? 1 : 0,
                                             e.key == Input::Key::Up ? -1 : e.key == Input::Key::Down ? 1 : 0);
            if (next) setFocus(next, true);

            return;
        }

        if (e.key == Input::Key::A)
        {
            Input::InputEvent ke = e;
            ke.pointer = pointer;

            if (pointer.valid)
            {
                captureWidget = root->hitTest(pointer.x, pointer.y);
                if (captureWidget)
                {
                    if (captureWidget->isFocusable()) setFocus(captureWidget, false);
                    captureWidget->onEvent(ke);

                    return;
                }
            }

            if (focusedWidget && focusedWidget->onEvent(ke)) return;
            return;
        }
    }

    if (e.type == Input::InputEvent::Type::KeyUp && e.key == Input::Key::A)
    {
        Input::InputEvent ke = e;
        ke.pointer = pointer;

        if (captureWidget)
        {
            captureWidget->onEvent(ke);
            captureWidget = nullptr;

            return;
        }

        if (focusedWidget && focusedWidget->onEvent(ke)) return;
        return;
    }

    if (e.type == Input::InputEvent::Type::KeyDown || e.type == Input::InputEvent::Type::KeyUp)
    {
        Input::InputEvent ke = e;
        ke.pointer = pointer;

        if (captureWidget && captureWidget->onEvent(ke)) return;
        if (focusedWidget && focusedWidget->onEvent(ke)) return;
        if (hoverWidget) hoverWidget->onEvent(ke);
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
