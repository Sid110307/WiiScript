#pragma once

#include <cstdint>

struct PointerState
{
    bool valid = false;
    float x = 0, y = 0;
};

struct InputState
{
    PointerState pointer;
    uint32_t down = 0, held = 0, up = 0;
};

bool videoInit();
void videoRender();
void videoExit();

bool inputInit();
void inputPoll(InputState& out);

bool timeInit();
uint64_t timeTicks();
double timeSecs();
