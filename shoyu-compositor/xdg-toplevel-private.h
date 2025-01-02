#pragma once

#include "xdg-toplevel.h"

#include <wlr/types/wlr_xdg_shell.h>

struct _ShoyuXdgToplevel {
    GObject parent_instance;

    ShoyuCompositor *compositor;

    struct wlr_xdg_toplevel *wlr_xdg_toplevel;

    bool is_invalidated;
    bool is_focused;
    guint x;
    guint y;

    struct wl_listener destroy;
};

struct _ShoyuXdgToplevelClass {
    GObjectClass parent_class;

    void (*realized)(ShoyuXdgToplevel *self,
                     struct wlr_xdg_toplevel *wlr_xdg_toplevel);
    void (*unrealized)(ShoyuXdgToplevel *self);
};

void shoyu_xdg_toplevel_realize(ShoyuXdgToplevel *self,
                                struct wlr_xdg_toplevel *wlr_xdg_toplevel);
void shoyu_xdg_toplevel_unrealize(ShoyuXdgToplevel *self);

void shoyu_shell_xdg_toplevel_bind_shell(ShoyuXdgToplevel *self);
void shoyu_shell_xdg_toplevel_unbind_shell(ShoyuXdgToplevel *self);

void shoyu_xdg_toplevel_set_geometry(ShoyuXdgToplevel *self, uint32_t x,
                                     uint32_t y, uint32_t width,
                                     uint32_t height);
