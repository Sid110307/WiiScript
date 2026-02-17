#include "./ui_root.h"

void UIRoot::init()
{
    left = root->addChild<Panel>();
    center = root->addChild<Panel>();
    bottom = root->addChild<Panel>();

    btnRun = center->addChild<Button>();
    btnStop = center->addChild<Button>();
    btnSave = center->addChild<Button>();

    fileList = left->addChild<List>();
    fileList->items.resize(20);
    for (int i = 0; i < 20; ++i) fileList->items[i] = "File " + std::to_string(i + 1);
}

void UIRoot::layout(const float screenW, const float screenH) const
{
    root->bounds = {0, 0, screenW, screenH};
    const float leftW = showLeft ? 200.0f : 0.0f, bottomH = showBottom ? 140.0f : 0.0f;

    if (left)
    {
        left->visible = showLeft;
        left->bounds = {0, 0, leftW, screenH};
    }
    if (bottom)
    {
        bottom->visible = showBottom;
        bottom->bounds = {leftW, screenH - bottomH, screenW - leftW, bottomH};
    }
    if (center) center->bounds = {leftW, 0, screenW - leftW, screenH - bottomH};

    if (btnRun && center) btnRun->bounds = {center->bounds.x + 10, center->bounds.y + 10, 60, 28};
    if (btnStop && center) btnStop->bounds = {center->bounds.x + 80, center->bounds.y + 10, 60, 28};
    if (btnSave && center) btnSave->bounds = {center->bounds.x + 150, center->bounds.y + 10, 60, 28};

    if (fileList && left) fileList->bounds = {left->bounds.x + 10, left->bounds.y + 10, leftW - 20, screenH - 20};
}

void UIRoot::update(const double dt) const { root->update(dt); }

void UIRoot::routeEvent(const InputEvent& e)
{
    if (e.type == InputEvent::Type::PointerMove) pointer = e.pointer;

    if (e.type == InputEvent::Type::Command)
    {
        if (e.cmd == Command::ToggleFileBrowser) showLeft = !showLeft;
        if (e.cmd == Command::ToggleConsole) showBottom = !showBottom;

        return;
    }

    if (e.type == InputEvent::Type::KeyDown && e.key == Key::A)
    {
        InputEvent e2 = e;
        e2.pointer = pointer;

        capture = root->hitTest(pointer.x, pointer.y);
        if (capture) capture->onEvent(e2);

        return;
    }

    if (e.type == InputEvent::Type::KeyUp && e.key == Key::A)
    {
        InputEvent e2 = e;
        e2.pointer = pointer;

        if (capture)
        {
            capture->onEvent(e2);
            capture = nullptr;

            return;
        }
    }

    if (e.type == InputEvent::Type::PointerMove)
        if (Widget* hit = root->hitTest(pointer.x, pointer.y)) hit->onEvent(e);
}

void UIRoot::draw() const { root->draw(); }
