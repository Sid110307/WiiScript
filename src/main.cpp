#include <cstdlib>

#include <wiiuse/wpad.h>
#include <grrlib.h>
#include <ogc/video.h>

#include "./platform/platform.h"
#include "./gfx/font.h"
#include "./ui/ui_root.h"

int main()
{
    if (!Input::init()) return EXIT_FAILURE;
    if (!Time::init()) return EXIT_FAILURE;
    if (!FileSystem::init()) return EXIT_FAILURE;

    FileSystem::ensureDir(FileSystem::workspaceRoot());
    VIDEO_Init();
    GRRLIB_Init();
    GRRLIB_SetBackgroundColour(0, 0, 0, 255);

    double last = Time::seconds();
    Input::InputFrame frame = {};
    std::vector<Input::InputEvent> events;
    Input::KeyRepeat repLeft = {}, repRight = {}, repUp = {}, repDown = {};

    UIRoot ui;
    ui.init();

    Font font;
    if (!font.load(FileSystem::appRoot() + "font.ttf", 16)) return EXIT_FAILURE;

    while (true)
    {
        Input::poll(&frame, events);
        const double now = Time::seconds(), dt = now - last;
        last = now;

        if (repLeft.update((frame.wpadHeld & WPAD_BUTTON_LEFT) != 0, now))
            events.push_back({.type = Input::InputEvent::Type::KeyDown, .key = Input::Key::Left});
        if (repRight.update((frame.wpadHeld & WPAD_BUTTON_RIGHT) != 0, now))
            events.push_back({.type = Input::InputEvent::Type::KeyDown, .key = Input::Key::Right});
        if (repUp.update((frame.wpadHeld & WPAD_BUTTON_UP) != 0, now))
            events.push_back({.type = Input::InputEvent::Type::KeyDown, .key = Input::Key::Up});
        if (repDown.update((frame.wpadHeld & WPAD_BUTTON_DOWN) != 0, now))
            events.push_back({.type = Input::InputEvent::Type::KeyDown, .key = Input::Key::Down});

        bool quit = false;
        for (const auto& e : events)
            if (e.type == Input::InputEvent::Type::Command && e.cmd == Input::Command::Quit)
            {
                quit = true;
                break;
            }

        if (quit) break;
        GRRLIB_FillScreen(theme().bg);

        ui.layout(640, 480);
        ui.update(dt);
        for (const auto& e : events) ui.routeEvent(e);
        ui.draw();

        if (font.isValid())
        {
            font.drawText("Hello", 20, 10, 0xFFFFFFFF);
            font.drawText("Workspace " + FileSystem::workspaceRoot(), 20, 30, 0xFFFFFFFF);
        }

        if (frame.pointer.valid) GRRLIB_Circle(frame.pointer.x, frame.pointer.y, 10, 0xFFCC00FF, true);
        else GRRLIB_Rectangle(10, 10, 10, 10, 0xFFCC00FF, true);

        GRRLIB_Render();
    }

    GRRLIB_Exit();
    return EXIT_SUCCESS;
}
