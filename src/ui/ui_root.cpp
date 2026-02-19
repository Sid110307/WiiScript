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
    fileListScroll->barY = fileListScroll->addChild<ScrollBar>(BoxDir::Vertical);
    keyboard = bottom->addChild<Keyboard>(uiFont);

    if (std::vector<FileSystem::DirEntry> entries; FileSystem::listDir(FileSystem::workspaceRoot, entries, true))
    {
        fileList->items.clear();
        for (const auto& e : entries) fileList->items.push_back(e.name + (e.isDir ? "/" : ""));
    }

    for (int i = 1; i <= 50; ++i) fileList->items.push_back("Item " + std::to_string(i));

    rebuildFocusList();
    if (!focusableWidgets.empty()) setFocus(focusableWidgets[0]);
}

void UIRoot::layout(const float screenW, const float screenH) const
{
    root->bounds = Rect({0, 0, screenW, screenH});
    Rect content = root->bounds;
    const float leftW = showLeft ? 200.0f : 0.0f, bottomH = showBottom ? 140.0f : 0.0f;

    left->visible = showLeft;
    left->bounds = leftW > 0.0f ? content.takeLeft(leftW) : Rect::empty();
    fileListScroll->visible = showLeft;
    fileListScroll->bounds = left->bounds.inset(10);
    if (fileListScroll->barY) fileListScroll->barY->layout.fixedHeight = fileListScroll->bounds.h;
    fileList->bounds = Rect({0, 0, fileListScroll->bounds.w, fileListScroll->bounds.h});

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
        focusedWidget = nullptr;
        if (!focusableWidgets.empty()) setFocus(focusableWidgets[0]);
    }
    if (hoverWidget && (!hoverWidget->visible || !hoverWidget->enabled)) hoverWidget = nullptr;

    root->update(dt);
}

void UIRoot::routeEvent(const Input::InputEvent& e)
{
    if (e.type == Input::InputEvent::Type::Pointer)
    {
        pointer = e.pointer;
        if (captureWidget) captureWidget->onEvent(e);

        Widget* prevHover = hoverWidget;
        hoverWidget = root->hitTest(pointer.x, pointer.y);

        if (prevHover && prevHover != hoverWidget) prevHover->onEvent(e);
        if (hoverWidget) hoverWidget->onEvent(e);

        return;
    }

    if (e.type == Input::InputEvent::Type::PointerDown)
    {
        pointer = e.pointer;
        captureWidget = root->hitTest(pointer.x, pointer.y);

        if (captureWidget)
        {
            if (captureWidget->isFocusable()) setFocus(captureWidget);
            captureWidget->onEvent(e);
        }

        return;
    }

    if (e.type == Input::InputEvent::Type::PointerUp)
    {
        pointer = e.pointer;
        if (captureWidget)
        {
            captureWidget->onEvent(e);
            captureWidget = nullptr;
        }

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
            if (next) setFocus(next);

            return;
        }

        if (e.key == Input::Key::A)
        {
            Input::InputEvent pe = e;
            pe.pointer = pointer;
            pe.type = Input::InputEvent::Type::PointerDown;

            captureWidget = root->hitTest(pointer.x, pointer.y);
            if (captureWidget)
            {
                if (captureWidget->isFocusable()) setFocus(captureWidget);
                captureWidget->onEvent(pe);
            }

            return;
        }
    }

    if (e.type == Input::InputEvent::Type::KeyUp && e.key == Input::Key::A)
    {
        if (captureWidget)
        {
            Input::InputEvent pe = e;
            pe.pointer = pointer;
            pe.type = Input::InputEvent::Type::PointerUp;

            captureWidget->onEvent(pe);
            captureWidget = nullptr;
        }
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

void UIRoot::setFocus(Widget* w)
{
    if (!w || !w->isFocusable()) return;
    if (focusedWidget == w) return;
    if (focusedWidget) focusedWidget->focused = false;

    focusedWidget = w;
    focusedWidget->focused = true;
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

    Widget* best = nullptr;
    float bestDist = std::numeric_limits<float>::max();

    const Rect from = focusedWidget->worldBounds();
    const float fromCx = from.x + from.w / 2.0f, fromCy = from.y + from.h / 2.0f;

    for (Widget* w : focusableWidgets)
    {
        if (w == focusedWidget) continue;

        const Rect to = w->worldBounds();
        const float dx = to.x + to.w / 2.0f - fromCx, dy = to.y + to.h / 2.0f - fromCy;

        if (dx >= 0) continue;
        if (dirX > 0 && dx <= 0) continue;
        if (dy >= 0) continue;
        if (dirY > 0 && dy <= 0) continue;

        if (const float dist = dx * dx + dy * dy; dist < bestDist) bestDist = dist, best = w;
    }

    if (!best)
        if (const auto it = std::find(focusableWidgets.begin(), focusableWidgets.end(), focusedWidget); it !=
            focusableWidgets.end())
        {
            int i = it - focusableWidgets.begin();
            int step = dirX + dirY;
            if (step == 0) step = 1;

            i = (i + step) % static_cast<int>(focusableWidgets.size());
            if (i < 0) i += static_cast<int>(focusableWidgets.size());

            best = focusableWidgets[i];
        }
    return best ? best : focusedWidget;
}
