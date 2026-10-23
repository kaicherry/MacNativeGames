/* Language of the host's own text on screen and in diagnostics: English by default; TOTK_UI_LANG=zh selects Chinese.
   Read on every call, so tests can switch. The game's own language is chosen separately (TOTK_LANG, services.c
   desired_language). */
#pragma once
#include <CoreFoundation/CoreFoundation.h>
#include <stdlib.h>
#include <string.h>

static inline int totk_ui_zh(void) {
    const char *e = getenv("TOTK_UI_LANG");
    return e && *e && !strncmp(e, "zh", 2);
}
#define TOTK_UI(zh, en) (totk_ui_zh() ? (zh) : (en))
