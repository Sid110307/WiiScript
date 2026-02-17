#include <cstdlib>

#include <wiiuse/wpad.h>
#include <grrlib.h>

#include "./platform/wii_platform.h"
#include "./ui/ui_root.h"

static void drawCross(const float x, const float y)
{
    GRRLIB_Line(x - 10, y, x + 10, y, 0xFFFFFFFF);
    GRRLIB_Line(x, y - 10, x, y + 10, 0xFFFFFFFF);
}

static void pushKeyDown(std::vector<InputEvent>& ev, const Key k)
{
    ev.push_back({.type = InputEvent::Type::KeyDown, .key = k});
}

int main()
{
    if (!videoInit()) return EXIT_FAILURE;
    if (!inputInit()) return EXIT_FAILURE;
    if (!timeInit()) return EXIT_FAILURE;

    InputFrame frame = {};
    std::vector<InputEvent> events;
    KeyRepeat repLeft = {}, repRight = {}, repUp = {}, repDown = {};

    UIRoot ui;
    ui.init();
    static double last = timeSecs();

    while (true)
    {
        inputPoll(&frame, events);
        const double now = timeSecs();
        const double dt = now - last;
        last = now;

        if (repLeft.update((frame.wpadHeld & WPAD_BUTTON_LEFT) != 0, now)) pushKeyDown(events, Key::Left);
        if (repRight.update((frame.wpadHeld & WPAD_BUTTON_RIGHT) != 0, now)) pushKeyDown(events, Key::Right);
        if (repUp.update((frame.wpadHeld & WPAD_BUTTON_UP) != 0, now)) pushKeyDown(events, Key::Up);
        if (repDown.update((frame.wpadHeld & WPAD_BUTTON_DOWN) != 0, now)) pushKeyDown(events, Key::Down);

        bool quit = false;
        for (const auto& e : events)
            if (e.type == InputEvent::Type::Command && e.cmd == Command::Quit)
            {
                quit = true;
                break;
            }

        if (quit) break;
        GRRLIB_FillScreen(0x101018FF);

        if (frame.pointer.valid)
        {
            drawCross(frame.pointer.x, frame.pointer.y);
            GRRLIB_Circle(frame.pointer.x, frame.pointer.y, 3, 0xFFCC00FF, true);
        }
        else GRRLIB_Rectangle(10, 10, 20, 20, 0xFF0000FF, true);

        ui.layout(640, 480);
        ui.update(dt);
        for (const auto& e : events) ui.routeEvent(e);
        ui.draw();

        videoRender();
    }

    videoExit();
    return EXIT_SUCCESS;
}
