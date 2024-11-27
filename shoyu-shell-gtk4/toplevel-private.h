#pragma once

#include "display.h"
#include "toplevel.h"
#include <shoyu-shell-client-protocol.h>

struct _ShoyuShellGtkToplevel {
    GObject parent_instance;
    ShoyuShellGtkDisplay *display;
    struct shoyu_shell_toplevel *shoyu_shell_toplevel;
    gboolean is_invalidated;
};

struct _ShoyuShellGtkToplevelClass {
    GObjectClass parent_class;

    void (*realized)(ShoyuShellGtkToplevel *self);
    void (*unrealized)(ShoyuShellGtkToplevel *self);
};

void shoyu_shell_gtk_toplevel_realize(
    ShoyuShellGtkToplevel *self,
    struct shoyu_shell_toplevel *shoyu_shell_toplevel);
void shoyu_shell_gtk_toplevel_unrealize(ShoyuShellGtkToplevel *self);
