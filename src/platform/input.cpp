#include "./platform.h"
#include <wiiuse/wpad.h>

static Input::PointerState lastPtr = {};

static void emitPointer(std::vector<Input::InputEvent>& ev, const Input::PointerState& p)
{
    if (p.valid == lastPtr.valid && p.x == lastPtr.x && p.y == lastPtr.y) return;

    ev.push_back({.type = Input::InputEvent::Type::PointerMove, .pointer = p});
    lastPtr = p;
}

static void emitCommand(std::vector<Input::InputEvent>& ev, const Input::Command c)
{
    ev.push_back({.type = Input::InputEvent::Type::Command, .cmd = c});
}

static void emitKey(std::vector<Input::InputEvent>& ev, const Input::InputEvent::Type t, const Input::Key k)
{
    ev.push_back({.type = t, .key = k});
}


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

    PointerState p = {};
    p.valid = ir.valid != 0;
    p.x = ir.x;
    p.y = ir.y;
    emitPointer(outEvents, p);

    if (outFrame)
    {
        outFrame->pointer = p;
        outFrame->wpadDown = down;
        outFrame->wpadHeld = held;
        outFrame->wpadUp = up;
    }
    if (down & WPAD_BUTTON_HOME) emitCommand(outEvents, Command::Quit);
    if (down & WPAD_BUTTON_PLUS) emitCommand(outEvents, Command::Run);
    if (down & WPAD_BUTTON_MINUS) emitCommand(outEvents, Command::Stop);
    if (down & WPAD_BUTTON_1) emitCommand(outEvents, Command::ToggleFileBrowser);
    if (down & WPAD_BUTTON_2) emitCommand(outEvents, Command::ToggleConsole);

    if (down & WPAD_BUTTON_UP) emitKey(outEvents, InputEvent::Type::KeyDown, Key::Up);
    if (up & WPAD_BUTTON_UP) emitKey(outEvents, InputEvent::Type::KeyUp, Key::Up);
    if (down & WPAD_BUTTON_DOWN) emitKey(outEvents, InputEvent::Type::KeyDown, Key::Down);
    if (up & WPAD_BUTTON_DOWN) emitKey(outEvents, InputEvent::Type::KeyUp, Key::Down);
    if (down & WPAD_BUTTON_LEFT) emitKey(outEvents, InputEvent::Type::KeyDown, Key::Left);
    if (up & WPAD_BUTTON_LEFT) emitKey(outEvents, InputEvent::Type::KeyUp, Key::Left);
    if (down & WPAD_BUTTON_RIGHT) emitKey(outEvents, InputEvent::Type::KeyDown, Key::Right);
    if (up & WPAD_BUTTON_RIGHT) emitKey(outEvents, InputEvent::Type::KeyUp, Key::Right);

    if (down & WPAD_BUTTON_A) emitKey(outEvents, InputEvent::Type::KeyDown, Key::A);
    if (up & WPAD_BUTTON_A) emitKey(outEvents, InputEvent::Type::KeyUp, Key::A);
    if (down & WPAD_BUTTON_B) emitKey(outEvents, InputEvent::Type::KeyDown, Key::B);
    if (up & WPAD_BUTTON_B) emitKey(outEvents, InputEvent::Type::KeyUp, Key::B);
}
