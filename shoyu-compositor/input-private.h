#pragma once

#include "input.h"

#include <wlr/types/wlr_input_device.h>

struct _ShoyuInput {
    GObject parent_instance;

    ShoyuCompositor *compositor;

    struct wlr_input_device *wlr_input_device;

    bool is_invalidated;

    struct wl_listener destroy;
};

struct _ShoyuInputClass {
    GObjectClass parent_class;

    void (*realized)(ShoyuInput *self,
                     struct wlr_input_device *wlr_input_device);
    void (*unrealized)(ShoyuInput *self);
};

void shoyu_input_realize(ShoyuInput *self,
                         struct wlr_input_device *wlr_input_device);
void shoyu_input_unrealize(ShoyuInput *self);
