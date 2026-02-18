#include "./platform.h"
#include <wiiuse/wpad.h>

static Input::PointerState lastPtr = {};

bool Input::init()
{
    WPAD_Init();
    WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR);
    WPAD_SetVRes(WPAD_CHAN_0, 640, 480);

    return true;
}

void Input::poll(InputFrame* outFrame, std::vector<InputEvent>& outEvents)
{
    outEvents.clear();
    WPAD_ScanPads();

    const uint32_t down = WPAD_ButtonsDown(WPAD_CHAN_0), held = WPAD_ButtonsHeld(WPAD_CHAN_0), up =
                       WPAD_ButtonsUp(WPAD_CHAN_0);
    ir_t ir;
    WPAD_IR(WPAD_CHAN_0, &ir);

    const PointerState p = {.valid = ir.valid != 0, .x = ir.x, .y = ir.y,};
    if (p.valid != lastPtr.valid || p.x != lastPtr.x || p.y != lastPtr.y)
    {
        outEvents.push_back({.type = InputEvent::Type::PointerMove, .pointer = p});
        lastPtr = p;
    }

    if (outFrame)
    {
        outFrame->pointer = p;
        outFrame->wpadDown = down;
        outFrame->wpadHeld = held;
        outFrame->wpadUp = up;
    }

    if (down & WPAD_BUTTON_HOME) outEvents.push_back({.type = InputEvent::Type::KeyDown, .key = Key::Home});
    if (up & WPAD_BUTTON_HOME) outEvents.push_back({.type = InputEvent::Type::KeyUp, .key = Key::Home});

    if (down & WPAD_BUTTON_UP) outEvents.push_back({.type = InputEvent::Type::KeyDown, .key = Key::Up});
    if (up & WPAD_BUTTON_UP) outEvents.push_back({.type = InputEvent::Type::KeyUp, .key = Key::Up});
    if (down & WPAD_BUTTON_DOWN) outEvents.push_back({.type = InputEvent::Type::KeyDown, .key = Key::Down});
    if (up & WPAD_BUTTON_DOWN) outEvents.push_back({.type = InputEvent::Type::KeyUp, .key = Key::Down});
    if (down & WPAD_BUTTON_LEFT) outEvents.push_back({.type = InputEvent::Type::KeyDown, .key = Key::Left});
    if (up & WPAD_BUTTON_LEFT) outEvents.push_back({.type = InputEvent::Type::KeyUp, .key = Key::Left});
    if (down & WPAD_BUTTON_RIGHT) outEvents.push_back({.type = InputEvent::Type::KeyDown, .key = Key::Right});
    if (up & WPAD_BUTTON_RIGHT) outEvents.push_back({.type = InputEvent::Type::KeyUp, .key = Key::Right});

    if (down & WPAD_BUTTON_A) outEvents.push_back({.type = InputEvent::Type::KeyDown, .key = Key::A});
    if (up & WPAD_BUTTON_A) outEvents.push_back({.type = InputEvent::Type::KeyUp, .key = Key::A});
    if (down & WPAD_BUTTON_B) outEvents.push_back({.type = InputEvent::Type::KeyDown, .key = Key::B});
    if (up & WPAD_BUTTON_B) outEvents.push_back({.type = InputEvent::Type::KeyUp, .key = Key::B});

    if (down & WPAD_BUTTON_PLUS) outEvents.push_back({.type = InputEvent::Type::KeyDown, .key = Key::Plus});
    if (up & WPAD_BUTTON_PLUS) outEvents.push_back({.type = InputEvent::Type::KeyUp, .key = Key::Plus});
    if (down & WPAD_BUTTON_MINUS) outEvents.push_back({.type = InputEvent::Type::KeyDown, .key = Key::Minus});
    if (up & WPAD_BUTTON_MINUS) outEvents.push_back({.type = InputEvent::Type::KeyUp, .key = Key::Minus});

    if (down & WPAD_BUTTON_1) outEvents.push_back({.type = InputEvent::Type::KeyDown, .key = Key::One});
    if (up & WPAD_BUTTON_1) outEvents.push_back({.type = InputEvent::Type::KeyUp, .key = Key::One});
    if (down & WPAD_BUTTON_2) outEvents.push_back({.type = InputEvent::Type::KeyDown, .key = Key::Two});
    if (up & WPAD_BUTTON_2) outEvents.push_back({.type = InputEvent::Type::KeyUp, .key = Key::Two});

    if (down & NUNCHUK_BUTTON_C) outEvents.push_back({.type = InputEvent::Type::KeyDown, .key = Key::C});
    if (up & NUNCHUK_BUTTON_C) outEvents.push_back({.type = InputEvent::Type::KeyUp, .key = Key::C});
    if (down & NUNCHUK_BUTTON_Z) outEvents.push_back({.type = InputEvent::Type::KeyDown, .key = Key::Z});
    if (up & NUNCHUK_BUTTON_Z) outEvents.push_back({.type = InputEvent::Type::KeyUp, .key = Key::Z});
}
