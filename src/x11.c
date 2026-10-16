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