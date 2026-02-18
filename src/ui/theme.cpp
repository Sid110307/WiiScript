#include "./theme.h"

static Font gFont;
static Theme gTheme;

const Font& font() { return gFont; }
const Theme& theme() { return gTheme; }
