#pragma once

#include <stdbool.h>
#include <stdio.h>

#include <X11/Xlib.h>

#define MAX_COMBO_LENGTH 16

typedef struct {
    Display *x11_display;
    int x11_opcode;
    int pressed_keys[MAX_COMBO_LENGTH];
    int pressed_keys_len;
    bool debug;
    bool abort_run;
} AppContext;

typedef struct {
    int code;
    bool pressed;
    bool valid;
} KeyEvent;

bool has_pressed_key(AppContext *ctx, int code);
void add_pressed_key(AppContext *ctx, int code);
void remove_pressed_key_at_idx(AppContext *ctx, int pos);
void remove_pressed_key(AppContext *ctx, int code);
void print_pressed_keys(AppContext *ctx);