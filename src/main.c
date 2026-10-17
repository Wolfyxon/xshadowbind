#include "main.h"

int main(int argc, char **argv) {
    AppContext ctx = {0};
    
    x11_init(&ctx);
    parse_args(&ctx, argc, argv);
    
    if(ctx.abort_run) {
        return 0;
    }

    if(ctx.binds_len == 0) {
        fprintf(stderr, "error: No valid keybinds configured.\n");
        print_help();
        return 1;
    }

    while(true) {
        main_loop(&ctx);
    }
}

void parse_args(AppContext *ctx, int argc, char **argv) {
    for(size_t i = 1; i < argc; i++) {
        char *arg = argv[i];

        if(strcmp(arg, "--help") == 0) {
            print_help();
            exit(0);
        }
        
        if(strcmp(arg, "--check-key") == 0) {
            if(argc - 1 < i + 1) {
                fprintf(stderr, "error: --check-key requires an argument.\n");
                exit(1);
            }

            check_key(ctx, argv[i + 1]);
            ctx->abort_run = true;
            i++;
            continue;
        }

        if(strcmp(arg, "--bind") == 0) {
            if(argc - 1 < i + 2) {
                fprintf(stderr, "error: --bind requires 2 arguments: --bind <combo> <command>\n");
                exit(1);
            }
            
            char *combo = argv[i + 1];
            char *command = argv[i + 2];
            
            add_bind(ctx, combo, command);

            i += 2;
            continue;
        }

        fprintf(stderr, "error: Unknown argument: '%s'\n", arg);
        fprintf(stderr,"Use the command with --help for help.\n");
        exit(1);
    }
}

void check_key(AppContext *ctx, const char *key_name) {
    int keysym = XStringToKeysym(key_name);

    if(keysym == 0) {
        fprintf(stderr, "%s: UNKNOWN\n", key_name);
    }

    int keycode = XKeysymToKeycode(ctx->x11_display, keysym);

    printf("%s: OK keysym: %d keycode: %d\n", key_name, keysym, keycode);
}

void add_bind(AppContext *ctx, const char *combo, const char *command) {
    if(ctx->binds_len >= MAX_BINDS) {
        fprintf(stderr, "error: Too many keybinds! Max is %d.\n", MAX_BINDS);
        exit(1);
    }

    KeyBind bind = {0};

    size_t combo_str_len = strlen(combo);
    size_t command_str_len = strlen(command);

    if(combo_str_len == 0 && command_str_len == 0 ) {
        fprintf(stderr, "error: Empty keybind (no key combo and no command). Skipping.\n");
        return;
    }

    if(combo_str_len == 0) {
        fprintf(stderr, "error: Key combo is empty for command: '%s'. Skipping.\n", command);
        return;
    }

    if(command_str_len == 0) {
        fprintf(stderr, "error: Command is empty for: '%s'. Skipping.\n", combo);
        return;
    }

    bind.command = command;

    char combo_buf[command_str_len];
    strcpy(combo_buf, combo);

    const char *DELIM = "+ \n\t";

    char *current_key = strtok(combo_buf, DELIM);

    while(current_key != NULL) {
        if(bind.combo_len > MAX_COMBO_LENGTH) {
            fprintf(stderr, "error: Key combo too long! Maximum is %d keys.\n", MAX_COMBO_LENGTH);
            return;
        }

        int keysym = XStringToKeysym(current_key);

        if(keysym == 0) {
            fprintf(stderr, "error: Invalid key: '%s'\n", current_key);
            return;
        }

        int keycode = XKeysymToKeycode(ctx->x11_display, keysym);
        
        bind.combo[bind.combo_len++] = keycode;
        current_key = strtok(NULL, DELIM);
    }

    ctx->binds[ctx->binds_len++] = bind;
}

void print_help() {
    puts("Usage: xshadowbind [OPTIONS]...");
    puts("");
    puts("Available options:");
    puts("  --help:                  Shows this help");
    puts("  --check-key: [key name]: Checks if a key is valid and returns its info.");
    puts("  --bind [combo] [command]: Binds a command to a key combination");
    puts("");
    puts("Example:");
    puts("  xshadowbind \\");
    puts("  --bind \"Super_L+Shift+Q\" \"shutdown now\" \\");
    puts("  --bind \"Alt+F4\" \"echo I am a stinker\"");
    puts("");
}

void main_loop(AppContext *ctx) {
    KeyEvent ev = x11_poll_key(ctx);

    if(!ev.valid) {
        return;
    }

    if(ev.pressed) {
        add_pressed_key(ctx, ev.code);
    } else {
        remove_pressed_key(ctx, ev.code);
    }

    print_pressed_keys(ctx);
    printf("%d\n", ev.code);
}
