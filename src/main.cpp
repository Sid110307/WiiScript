#include <cstdlib>

#include <gccore.h>
#include <wiiuse/wpad.h>
#include <grrlib.h>

#include "./platform/wii_platform.h"

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
    int flashCommand = 0, flashKey = 0, flashPointer = 0;

    while (true)
    {
        inputPoll(&frame, events);
        const double now = timeSecs();

        if (repLeft.update((frame.wpadHeld & WPAD_BUTTON_LEFT) != 0, now)) pushKeyDown(events, Key::Left);
        if (repRight.update((frame.wpadHeld & WPAD_BUTTON_RIGHT) != 0, now)) pushKeyDown(events, Key::Right);
        if (repUp.update((frame.wpadHeld & WPAD_BUTTON_UP) != 0, now)) pushKeyDown(events, Key::Up);
        if (repDown.update((frame.wpadHeld & WPAD_BUTTON_DOWN) != 0, now)) pushKeyDown(events, Key::Down);

        bool quit = false;
        for (const auto& e : events)
        {
            switch (e.type)
            {
            case InputEvent::Type::Command:
                flashCommand = 6;
                if (e.cmd == Command::Quit) quit = true;

                break;
            case InputEvent::Type::KeyDown:
            case InputEvent::Type::KeyUp:
                flashKey = 4;
                break;
            case InputEvent::Type::PointerMove:
            case InputEvent::Type::PointerDown:
            case InputEvent::Type::PointerUp:
                flashPointer = 3;
                break;
            default:
                break;
            }
        }

        if (quit) break;
        GRRLIB_FillScreen(0x101018FF);

        if (frame.pointer.valid)
        {
            drawCross(frame.pointer.x, frame.pointer.y);
            GRRLIB_Circle(frame.pointer.x, frame.pointer.y, 3, 0xFFCC00FF, true);
        }
        else GRRLIB_Rectangle(10, 10, 20, 20, 0xFF0000FF, true);

        const bool A = (frame.wpadHeld & WPAD_BUTTON_A) != 0;
        const bool B = (frame.wpadHeld & WPAD_BUTTON_B) != 0;
        const bool Plus = (frame.wpadHeld & WPAD_BUTTON_PLUS) != 0;
        const bool Minus = (frame.wpadHeld & WPAD_BUTTON_MINUS) != 0;

        GRRLIB_Rectangle(10, 40, 18, 18, A ? 0x00FF00FF : 0x003300FF, true);
        GRRLIB_Rectangle(34, 40, 18, 18, B ? 0xFF0000FF : 0x330000FF, true);
        GRRLIB_Rectangle(58, 40, 18, 18, Plus ? 0x0080FFFF : 0x001833FF, true);
        GRRLIB_Rectangle(82, 40, 18, 18, Minus ? 0xFFCC00FF : 0x332900FF, true);

        if (flashCommand > 0)
        {
            GRRLIB_Rectangle(10, 90, 20, 20, 0x00FFFFFF, true);
            flashCommand--;
        }
        else GRRLIB_Rectangle(10, 90, 20, 20, 0x003333FF, true);

        if (flashKey > 0)
        {
            GRRLIB_Rectangle(34, 90, 20, 20, 0xFF00FFFF, true);
            flashKey--;
        }
        else GRRLIB_Rectangle(34, 90, 20, 20, 0x330033FF, true);

        if (flashPointer > 0)
        {
            GRRLIB_Rectangle(58, 90, 20, 20, 0xFFFFFFFF, true);
            flashPointer--;
        }
        else GRRLIB_Rectangle(58, 90, 20, 20, 0x222222FF, true);

        videoRender();
    }

    videoExit();
    return EXIT_SUCCESS;
}
