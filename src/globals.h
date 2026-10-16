#pragma once

#include <stdbool.h>
#include <X11/Xlib.h>

typedef struct {
    Display *x11_display;
    int x11_opcode;
    bool debug;
    bool abort_run;
} AppContext;

typedef struct {
    int code;
    bool pressed;
    bool valid;
} KeyEvent;
