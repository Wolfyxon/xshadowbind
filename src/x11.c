#include "x11.h"

void x11_init(AppContext *ctx) {
    Display* disp = XOpenDisplay(NULL);

    if(disp == NULL) {
        fprintf(stderr, "error: Failed to open X11 display\n");
        exit(1);
    }

    int opcode, event, error;

    if(!XQueryExtension(disp, "XInputExtension", &opcode, &event, &error)) {
        fprintf(stderr, "error: Failed to query X input extension. Error code: %d\n", error);
        exit(1);
    }

    ctx->x11_display = disp;
    ctx->x11_opcode = opcode;

    x11_init_events(ctx);
}

void x11_init_events(AppContext *ctx) {
    Display *disp = ctx->x11_display;

    XIEventMask mask;
    unsigned char mask_data[XIMaskLen(XI_LASTEVENT)] = {0};

    XISetMask(mask_data, XI_KeyPress);
    XISetMask(mask_data, XI_KeyRelease);

    mask.deviceid = XIAllDevices;
    mask.mask_len = sizeof(mask_data);
    mask.mask = mask_data;

    XISelectEvents(disp, DefaultRootWindow(disp), &mask, 1);
    XFlush(disp);
}

KeyEvent x11_poll_key(AppContext *ctx) {
    KeyEvent res = {
        .code = 0,
        .pressed = false,
        .valid = false,  
    };
    
    XEvent event = {};

    XNextEvent(ctx->x11_display, &event);
    XGenericEventCookie* cookie = &event.xcookie;

    if(
        (cookie->type != GenericEvent) ||
        (cookie->extension != ctx->x11_opcode) ||
        !XGetEventData(ctx->x11_display, cookie) ||
        (cookie->evtype != XI_KeyPress && cookie->evtype != XI_KeyRelease)
    ) {
        goto finish;
    }

    XIDeviceEvent* dev_event = (XIDeviceEvent*)cookie->data;
    
    res.code = dev_event->detail;
    res.valid = true;
    res.pressed = cookie->evtype == XI_KeyPress;
    
    finish:
    
    if(cookie != NULL) {
        XFreeEventData(ctx->x11_display, cookie);
    }

    return res;
}

bool x11_is_window_fullscreen(AppContext *ctx, Window w) {
    Window root = DefaultRootWindow(ctx->x11_display);

    XWindowAttributes root_attr = {0};
    XWindowAttributes attr = {0};
    
    if(XGetWindowAttributes(ctx->x11_display, root, &root_attr) == 0) {
        return false;
    }

    if(XGetWindowAttributes(ctx->x11_display, w, &attr) == 0) {
        return false;
    }

    if(attr.height == root_attr.height) {
        return true;
    }

    return false;
}

bool x11_has_lockscreen(AppContext *ctx) {
    Window root = XDefaultRootWindow(ctx->x11_display);

    Window parent;

    Window *windows;
    unsigned int windows_len;

    int status = XQueryTree(
        ctx->x11_display, 
        root,
        &root, // root
        &parent,  // parent
        &windows, 
        &windows_len
    );
    
    if(status == 0) {
        fprintf(stderr, "error: Failed to list windows.\n");
        return false;
    }

    if(windows == NULL) {
        fprintf(stderr, "error: Failed to list windows. Array is NULL.\n");
        return false;
    }

    for(size_t i = 0; i < windows_len; i++) {
        XWindowAttributes attr = {0};
        Window window = windows[i];

        int attr_status = XGetWindowAttributes(ctx->x11_display, window, &attr);
        
        if(attr_status == 0) {
            continue;
        }

        if(!x11_is_window_fullscreen(ctx, window)) {
            continue;
        }
        if(attr.map_state != IsViewable) {
            continue;
        } 

        if(!attr.override_redirect) {
            continue;
        }

        if(!attr.map_installed) {
            continue;
        }

        /*
        char *nam = malloc(1024);

        XFetchName(ctx->x11_display, windows[i], &nam);
        
        if(nam != NULL) {
            puts(nam);
        }*/
        
        XFree(windows);
        return true;
    }

    XFree(windows);

    return false;
}