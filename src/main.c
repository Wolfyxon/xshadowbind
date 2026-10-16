#include "main.h"

int main(int argc, char **argv) {
    AppContext ctx = {0};
    
    x11_init(&ctx);
    parse_args(&ctx, argc, argv);
    
    if(ctx.abort_run) {
        return 0;
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
            fprintf(stderr, "error: --bind not implemented\n");
            exit(1);
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

    printf("%d\n", ev.code);
}
