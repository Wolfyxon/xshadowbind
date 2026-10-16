#include "globals.h"

bool has_pressed_key(AppContext *ctx, int code) {
    for(size_t i = 0; i < ctx->pressed_keys_len; i++) {
        if(ctx->pressed_keys[i] == code) {
            return true;
        }
    }

    return false;
}

void add_pressed_key(AppContext *ctx, int code) {
    if(ctx->pressed_keys_len < MAX_COMBO_LENGTH && !has_pressed_key(ctx, code)) {
        ctx->pressed_keys[ctx->pressed_keys_len++] = code;
    }
}

void remove_pressed_key_at_idx(AppContext *ctx, int pos) {
    for(size_t i = pos; i < ctx->pressed_keys_len - 1; i++) {
        ctx->pressed_keys[i] = ctx->pressed_keys[i + 1];
    }

    ctx->pressed_keys_len -= 1;
}

void remove_pressed_key(AppContext *ctx, int code) {
    for(size_t i = 0; i < ctx->pressed_keys_len; i++) {
        if(ctx->pressed_keys[i] == code) {
            remove_pressed_key_at_idx(ctx, i);
        }
    }
}

void print_pressed_keys(AppContext *ctx) {
    for(size_t i = 0; i < ctx->pressed_keys_len; i++) {
        printf("%d ", ctx->pressed_keys[i]);
    }

    puts("");
}