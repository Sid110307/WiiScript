#include "./wii_platform.h"
#include <wiiuse/wpad.h>

bool inputInit()
{
    WPAD_Init();
    WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR);
    WPAD_SetVRes(WPAD_CHAN_0, 640, 480);

    return true;
}

void inputPoll(InputState& out)
{
    WPAD_ScanPads();

    out.down = WPAD_ButtonsDown(WPAD_CHAN_0);
    out.held = WPAD_ButtonsHeld(WPAD_CHAN_0);
    out.up = WPAD_ButtonsUp(WPAD_CHAN_0);

    ir_t ir;
    WPAD_IR(WPAD_CHAN_0, &ir);

    out.pointer.valid = ir.valid != 0;
    out.pointer.x = ir.x;
    out.pointer.y = ir.y;
}
