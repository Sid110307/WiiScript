#include "./ui_root.h"

void UIRoot::init(Font& font)
{
    root->font = &font;

    left = root->addChild<Panel>();
    center = root->addChild<Panel>();
    bottom = root->addChild<Panel>();

    btnRun = center->addChild<Button>();
    btnStop = center->addChild<Button>();
    btnSave = center->addChild<Button>();

    fileList = left->addChild<List>();
    if (std::vector<FileSystem::DirEntry> entries; FileSystem::listDir(FileSystem::workspaceRoot, entries, true))
        for (std::size_t i = 0; i < entries.size() && i < fileList->items.size(); ++i)
            fileList->items[i] = entries[i].name;

    keyboard = bottom->addChild<Keyboard>(font);
    keyboard->visible = showBottom;
}

void UIRoot::layout(const float screenW, const float screenH) const
{
    Rect content({0, 0, screenW, screenH});
    root->bounds = content;
    const float leftW = showLeft ? 200.0f : 0.0f, bottomH = showBottom ? 140.0f : 0.0f;

    if (left)
    {
        left->visible = showLeft;
        left->bounds = leftW > 0.0f ? content.takeLeft(leftW) : Rect::empty();

        if (fileList) fileList->bounds = left->bounds.inset(10);
    }

    if (bottom)
    {
        bottom->visible = showBottom;
        bottom->bounds = bottomH > 0.0f ? content.takeBottom(bottomH) : Rect::empty();

        if (keyboard) keyboard->bounds = bottom->bounds.inset(10);
    }

    if (center)
    {
        center->bounds = content;
        Rect toolbar = center->bounds.inset(10).takeTop(28);

        if (btnRun) btnRun->bounds = toolbar.takeRowItem(60, 28, 10);
        if (btnStop) btnStop->bounds = toolbar.takeRowItem(60, 28, 10);
        if (btnSave) btnSave->bounds = toolbar.takeRowItem(60, 28, 10);
    }
}

void UIRoot::update(const double dt) const { root->update(dt); }

void UIRoot::routeEvent(const Input::InputEvent& e)
{
    if (e.type == Input::InputEvent::Type::PointerMove) pointer = e.pointer;

    if (e.type == Input::InputEvent::Type::KeyDown)
    {
        if (e.key == Input::Key::Home)
        {
            quit = true;
            return;
        }

        if (e.key == Input::Key::Plus) return;
        if (e.key == Input::Key::Minus) return;

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

        if (e.key == Input::Key::A)
        {
            Input::InputEvent event = e;
            event.pointer = pointer;

            capture = root->hitTest(pointer.x, pointer.y);
            if (capture) capture->onEvent(event);

            return;
        }
    }

    if (e.type == Input::InputEvent::Type::KeyUp && e.key == Input::Key::A)
    {
        Input::InputEvent event = e;
        event.pointer = pointer;

        if (capture)
        {
            capture->onEvent(event);
            capture = nullptr;

            return;
        }
    }

    if (e.type == Input::InputEvent::Type::PointerMove)
        if (Widget* hit = root->hitTest(pointer.x, pointer.y)) hit->onEvent(e);
}

void UIRoot::draw() const { root->draw(); }
