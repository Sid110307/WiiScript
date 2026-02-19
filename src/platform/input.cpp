#include "./platform.h"

#include <cmath>
#include <algorithm>
#include <wiiuse/wpad.h>

static Input::PointerState lastPtr = {};

void Input::KeyRepeat::generate(const InputFrame& frame, const double dt, std::vector<InputEvent>& outEvents)
{
    repeatKey(dt, outEvents, Key::Up, (frame.wpadHeld & WPAD_BUTTON_UP) != 0);
    repeatKey(dt, outEvents, Key::Down, (frame.wpadHeld & WPAD_BUTTON_DOWN) != 0);
    repeatKey(dt, outEvents, Key::Left, (frame.wpadHeld & WPAD_BUTTON_LEFT) != 0);
    repeatKey(dt, outEvents, Key::Right, (frame.wpadHeld & WPAD_BUTTON_RIGHT) != 0);
    repeatKey(dt, outEvents, Key::Plus, (frame.wpadHeld & WPAD_BUTTON_PLUS) != 0);
    repeatKey(dt, outEvents, Key::Minus, (frame.wpadHeld & WPAD_BUTTON_MINUS) != 0);
}

void Input::KeyRepeat::repeatKey(const double dt, std::vector<InputEvent>& outEvents, const Key key, const bool held)
{
    State& s = keyStates[static_cast<size_t>(key)];

    if (!held)
    {
        s = {};
        return;
    }

    if (!s.wasHeld)
    {
        s.wasHeld = true;
        s.heldTime = 0.0f;
        s.nextTime = 0.33f;

        return;
    }

    s.heldTime += dt;
    while (s.heldTime >= s.nextTime)
    {
        outEvents.push_back({.type = InputEvent::Type::KeyDown, .key = key});
        s.nextTime += 0.05f;
    }
}

bool Input::init()
{
    if (WPAD_Init() != WPAD_ERR_NONE) return false;
    if (WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR) != WPAD_ERR_NONE) return false;
    if (WPAD_SetVRes(WPAD_CHAN_0, 640, 480) != WPAD_ERR_NONE) return false;

    return true;
}

void Input::poll(InputFrame* outFrame, std::vector<InputEvent>& outEvents)
{
    outEvents.clear();
    WPAD_ScanPads();

    if (WPAD_Probe(WPAD_CHAN_0, nullptr) != WPAD_ERR_NONE)
    {
        if (lastPtr.valid)
        {
            lastPtr = {};
            outEvents.push_back({.type = InputEvent::Type::Pointer, .pointer = {}});
        }
        return;
    }

    const uint32_t down = WPAD_ButtonsDown(WPAD_CHAN_0), held = WPAD_ButtonsHeld(WPAD_CHAN_0), up =
                       WPAD_ButtonsUp(WPAD_CHAN_0);
    ir_t ir;
    WPAD_IR(WPAD_CHAN_0, &ir);

    const PointerState p = {.valid = ir.valid != 0, .x = ir.x, .y = ir.y,};
    if (p.valid != lastPtr.valid || p.x != lastPtr.x || p.y != lastPtr.y)
    {
        outEvents.push_back({.type = InputEvent::Type::Pointer, .pointer = p});
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

    if (down & WPAD_NUNCHUK_BUTTON_C) outEvents.push_back({.type = InputEvent::Type::KeyDown, .key = Key::C});
    if (up & WPAD_NUNCHUK_BUTTON_C) outEvents.push_back({.type = InputEvent::Type::KeyUp, .key = Key::C});
    if (down & WPAD_NUNCHUK_BUTTON_Z) outEvents.push_back({.type = InputEvent::Type::KeyDown, .key = Key::Z});
    if (up & WPAD_NUNCHUK_BUTTON_Z) outEvents.push_back({.type = InputEvent::Type::KeyUp, .key = Key::Z});

    expansion_t exp;
    WPAD_Expansion(WPAD_CHAN_0, &exp);

    if (exp.type == WPAD_EXP_NUNCHUK)
    {
        constexpr float deadZone = 10.0f;
        auto dx = static_cast<float>(exp.nunchuk.js.pos.x - exp.nunchuk.js.center.x),
             dy = static_cast<float>(exp.nunchuk.js.pos.y - exp.nunchuk.js.center.y);

        if (float mag = std::sqrt(dx * dx + dy * dy); mag > deadZone)
        {
            float strength = (mag - deadZone) / (100.0f - deadZone);
            strength = std::clamp(strength, 0.0f, 1.0f);

            static float scrollAccumX = 0.0f, scrollAccumY = 0.0f;
            scrollAccumX += strength * 3.0f * (dx > 0 ? 1.0f : -1.0f);
            scrollAccumY += strength * 3.0f * (dy > 0 ? -1.0f : 1.0f);

            int wholeX = static_cast<int>(scrollAccumX), wholeY = static_cast<int>(scrollAccumY);
            scrollAccumX -= static_cast<float>(wholeX);
            scrollAccumY -= static_cast<float>(wholeY);

            if (wholeX != 0) outEvents.push_back({.type = InputEvent::Type::Scroll, .scrollX = wholeX});
            if (wholeY != 0) outEvents.push_back({.type = InputEvent::Type::Scroll, .scrollY = wholeY});
        }
    }
}
