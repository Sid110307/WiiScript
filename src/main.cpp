#include <gccore.h>
#include <wiiuse/wpad.h>
#include <grrlib.h>

#include "./platform/wii_platform.h"

static void drawCross(const float x, const float y)
{
    GRRLIB_Line(x - 10, y, x + 10, y, 0xFFFFFFFF);
    GRRLIB_Line(x, y - 10, x, y + 10, 0xFFFFFFFF);
}

int main(const int argc, char** argv)
{
    (void)argc;
    (void)argv;

    if (!videoInit()) return 1;
    if (!inputInit()) return 1;
    if (!timeInit()) return 1;

    InputState input;
    double lastFpsTime = timeSecs();
    int frames = 0;
    int fps = 0;

    while (true)
    {
        inputPoll(input);
        if (input.down & WPAD_BUTTON_HOME) break;

        frames++;
        if (const double now = timeSecs(); now - lastFpsTime >= 1.0)
        {
            fps = frames;
            frames = 0;
            lastFpsTime = now;
        }
        GRRLIB_FillScreen(0x101018FF);

        if (input.pointer.valid)
        {
            drawCross(input.pointer.x, input.pointer.y);
            GRRLIB_Circle(input.pointer.x, input.pointer.y, 3, 0xFFCC00FF, true);
        }
        else GRRLIB_Rectangle(10, 10, 20, 20, 0xFF0000FF, true);

        const bool A = (input.held & WPAD_BUTTON_A) != 0;
        const bool B = (input.held & WPAD_BUTTON_B) != 0;
        const bool Plus = (input.held & WPAD_BUTTON_PLUS) != 0;
        const bool Minus = (input.held & WPAD_BUTTON_MINUS) != 0;

        GRRLIB_Rectangle(10, 40, 18, 18, A ? 0x00FF00FF : 0x003300FF, true);
        GRRLIB_Rectangle(34, 40, 18, 18, B ? 0xFF0000FF : 0x330000FF, true);
        GRRLIB_Rectangle(58, 40, 18, 18, Plus ? 0x0080FFFF : 0x001833FF, true);
        GRRLIB_Rectangle(82, 40, 18, 18, Minus ? 0xFFCC00FF : 0x332900FF, true);

        int w = fps;
        if (w < 0) w = 0;
        if (w > 60) w = 60;
        GRRLIB_Rectangle(10, 70, 120, 10, 0x222222FF, true);
        GRRLIB_Rectangle(10, 70, static_cast<float>(w * 2), 10, 0xFFFFFFFF, true);

        videoRender();
    }

    videoExit();
    return 0;
}
