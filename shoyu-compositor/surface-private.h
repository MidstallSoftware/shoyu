#pragma once

#include "surface.h"

#include <wlr/types/wlr_compositor.h>

struct _ShoyuSurface {
    GObject parent_instance;

    ShoyuCompositor *compositor;

    struct wlr_surface *wlr_surface;
    struct wlr_buffer *buffer;

    bool is_invalidated;

    struct wl_listener destroy;
    struct wl_listener commit;
    struct wl_listener map;
};

struct _ShoyuSurfaceClass {
    GObjectClass parent_class;

    void (*realized)(ShoyuSurface *self, struct wlr_surface *wlr_surface);
    void (*unrealized)(ShoyuSurface *self);
};

void shoyu_surface_realize(ShoyuSurface *self, struct wlr_surface *wlr_surface);
void shoyu_surface_unrealize(ShoyuSurface *self);
