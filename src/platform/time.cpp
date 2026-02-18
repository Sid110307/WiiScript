#include "./platform.h"
#include <ogc/lwp_watchdog.h>

static uint64_t startTicks = 0;

bool Time::init()
{
    startTicks = gettime();
    return true;
}

uint64_t Time::ticks() { return gettime(); }

double Time::seconds()
{
    const uint64_t dt = gettime() - startTicks;
    const auto ms = static_cast<double>(ticks_to_millisecs(dt));

    return ms / 1000.0;
}
