#pragma once

#include <stdlib.h>
#include <stdio.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>

#include "globals.h"

void x11_init(AppContext *ctx);
void x11_init_events(AppContext *ctx);

KeyEvent x11_poll_key(AppContext *ctx); // blocking
bool x11_has_lockscreen(AppContext *ctx);
