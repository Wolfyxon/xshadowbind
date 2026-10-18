#include "main.h"

int main(int argc, char **argv) {
    AppContext ctx = {0};
    
    x11_init(&ctx);
    parse_args(&ctx, argc, argv);
    
    if(ctx.abort_run) {
        return 0;
    }

    if(ctx.binds_len == 0) {
        fprintf(stderr, "error: No valid keybinds configured. Use --help to see help.\n");
        return 1;
    }

    if(ctx.verbose) {
        printf("info: Running main loop\n");
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

        if(strcmp(arg, "--verbose") == 0) {
            ctx->verbose = true;
            printf("info: Verbose logging enabled\n");
            continue;
        }

        if(strcmp(arg, "--no-exec") == 0) {
            ctx->no_exec = true;
            continue;
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

    if(get_bind(ctx, bind.combo, bind.combo_len) != NULL) {
        fprintf(stderr, "error: A keybind is already configured for: %s.\n", combo);
        return;
    } 

    ctx->binds[ctx->binds_len++] = bind;

    if(ctx->verbose) {
        printf("info: Added bind: Keycodes: ");

        for(size_t i = 0; i < bind.combo_len; i++) {
            printf("%d ", bind.combo[i]);
        }

        printf("command: %s\n", bind.command);
    }
}

KeyBind *get_bind(AppContext *ctx, int *keycodes, size_t keycodes_len) {
    for(size_t bind_i = 0; bind_i < ctx->binds_len; bind_i++) {
        KeyBind *bind = &ctx->binds[bind_i];

        if(bind->combo_len != keycodes_len) {
            continue;
        }

        bool matches = true;

        for(size_t i = 0; i < keycodes_len; i++) {
            if(bind->combo[i] != keycodes[i]) {
                matches = false;
                break;
            } 
        }

        if(matches) {
            return bind;
        }
    }

    return NULL;
}

void print_help() {
    puts("Usage: xshadowbind <OPTIONS>...");
    puts("");
    puts("Available options:");
    puts("  --help:                   Shows this help");
    puts("  --check-key: <key name>:  Checks if a key is valid and returns its info.");
    puts("  --bind <combo> <command>: Binds a command to a key combination");
    puts("  --no-exec:                Disables command execution and prints them instead. Useful for testing.");
    puts("  --verbose:                Enables verbose logging.");
    puts("");
    puts("Example:");
    puts("  xshadowbind \\");
    puts("  --bind \"Super_L+Shift_L+Q\" \"shutdown now\" \\");
    puts("  --bind \"Super_L+L\" my_lockscreen_command");
    puts("");
    puts("For bugs and suggestions see:");
    puts("https://github.com/Wolfyxon/xshadowbind");
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
    
    KeyBind *current_bind = get_bind(ctx, ctx->pressed_keys, ctx->pressed_keys_len);

    if(current_bind == NULL) {
        return;
    }

    //memset(ctx->pressed_keys, 0, ctx->pressed_keys_len);

    //ctx->pressed_keys_len = 0;

    if(ctx->verbose && !ctx->no_exec) {
        printf("info: Executing command: %s\n", current_bind->command);
    }

    if(ctx->no_exec) {
        printf("info: Would execute: %s\n", current_bind->command);
        return;
    }

    int pid = fork();

    if(pid < 0) {
        fprintf(stderr, "error: Unable to create process to execute command.\n");
        return;
    }

    if(pid == 0) { // process is the fork
        exit(system(current_bind->command));
    }
}
