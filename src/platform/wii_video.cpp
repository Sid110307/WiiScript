#include "./wii_platform.h"

#include <grrlib.h>
#include <ogc/video.h>

bool videoInit()
{
    VIDEO_Init();
    GRRLIB_Init();
    GRRLIB_SetBackgroundColour(0, 0, 0, 255);

    return true;
}

void videoRender() { GRRLIB_Render(); }
void videoExit() { GRRLIB_Exit(); }
