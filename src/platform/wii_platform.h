#pragma once

#include <cstdint>
#include <vector>

#include "../core/input_events.h"

bool inputInit();
void inputPoll(InputFrame* outFrame, std::vector<InputEvent>& outEvents);

bool timeInit();
uint64_t timeTicks();
double timeSecs();
