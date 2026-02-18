#include <cstdlib>

#include <grrlib.h>
#include <ogc/video.h>

#include "./platform/platform.h"
#include "./gfx/font.h"
#include "./ui/ui_root.h"

int main()
{
    if (!Input::init())
    {
        printf("Failed to initialize input!\n");
        return EXIT_FAILURE;
    }
    if (!Time::init())
    {
        printf("Failed to initialize time!\n");
        return EXIT_FAILURE;
    }
    if (!FileSystem::init())
    {
        printf("Failed to initialize filesystem!\n");
        return EXIT_FAILURE;
    }

    FileSystem::ensureDir(FileSystem::workspaceRoot);
    VIDEO_Init();
    GRRLIB_Init();
    GRRLIB_SetBackgroundColour(0, 0, 0, 255);

    double last = Time::seconds();
    Input::InputFrame frame = {};
    std::vector<Input::InputEvent> events;

    Font font;
    if (!font.load(FileSystem::appRoot + "font.ttf", 16))
    {
        printf("Failed to load font!\n");
        return EXIT_FAILURE;
    }

    UIRoot ui;
    ui.init(font);

    while (true)
    {
        Input::poll(&frame, events);
        const double now = Time::seconds(), dt = now - last;
        last = now;

        if (ui.quit) break;
        GRRLIB_FillScreen(theme().bg);

        ui.layout(640, 480);
        ui.update(dt);
        for (const auto& e : events) ui.routeEvent(e);
        ui.draw();

        if (frame.pointer.valid) GRRLIB_Circle(frame.pointer.x, frame.pointer.y, 10, theme().accent, true);
        else GRRLIB_Rectangle(10, 10, 10, 10, theme().accent, true);

        GRRLIB_Render();
    }

    GRRLIB_Exit();
    return EXIT_SUCCESS;
}
