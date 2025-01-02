#pragma once

#include "input-private.h"
#include "pointer-input.h"

#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_pointer.h>

struct _ShoyuPointerInput {
    ShoyuInput parent_instance;

    struct wlr_cursor *cursor;

    struct wl_listener cursor_motion;
    struct wl_listener cursor_motion_absolute;
    struct wl_listener cursor_button;
    struct wl_listener cursor_axis;
    struct wl_listener cursor_frame;
};

struct _ShoyuPointerInputClass {
    ShoyuInputClass parent_class;
};
