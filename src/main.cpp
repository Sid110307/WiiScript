#include <cstdlib>

#include <wiiuse/wpad.h>
#include <grrlib.h>
#include <ogc/video.h>

#include "./platform/wii_platform.h"
#include "./gfx/font.h"
#include "./ui/ui_root.h"

int main()
{
    if (!inputInit()) return EXIT_FAILURE;
    if (!timeInit()) return EXIT_FAILURE;

    VIDEO_Init();
    GRRLIB_Init();
    GRRLIB_SetBackgroundColour(0, 0, 0, 255);

    static double last = timeSecs();

    InputFrame frame = {};
    std::vector<InputEvent> events;
    KeyRepeat repLeft = {}, repRight = {}, repUp = {}, repDown = {};

    UIRoot ui;
    ui.init();

    Font font;
    if (!font.load("sd:/apps/WiiScript/font.ttf", 16)) return EXIT_FAILURE;

    while (true)
    {
        inputPoll(&frame, events);
        const double now = timeSecs();
        const double dt = now - last;
        last = now;

        if (repLeft.update((frame.wpadHeld & WPAD_BUTTON_LEFT) != 0, now))
            events.push_back({.type = InputEvent::Type::KeyDown, .key = Key::Left});
        if (repRight.update((frame.wpadHeld & WPAD_BUTTON_RIGHT) != 0, now))
            events.push_back({.type = InputEvent::Type::KeyDown, .key = Key::Right});
        if (repUp.update((frame.wpadHeld & WPAD_BUTTON_UP) != 0, now))
            events.push_back({.type = InputEvent::Type::KeyDown, .key = Key::Up});
        if (repDown.update((frame.wpadHeld & WPAD_BUTTON_DOWN) != 0, now))
            events.push_back({.type = InputEvent::Type::KeyDown, .key = Key::Down});

        bool quit = false;
        for (const auto& e : events)
            if (e.type == InputEvent::Type::Command && e.cmd == Command::Quit)
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
            font.drawText("Testing color...", 20, 30, 0xFF00FFFF);
        }

        if (frame.pointer.valid) GRRLIB_Circle(frame.pointer.x, frame.pointer.y, 3, 0xFFCC00FF, true);
        else GRRLIB_Rectangle(10, 10, 10, 10, 0xFFCC00FF, true);

        GRRLIB_Render();
    }

    GRRLIB_Exit();
    return EXIT_SUCCESS;
}
