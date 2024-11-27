#pragma once

#include "display.h"
#include "toplevel.h"
#include <shoyu-shell-client-protocol.h>

#define SHOYU_SHELL_GTK_DISPLAY_KEY "shoyu-shell-gtk4-display"

struct _ShoyuShellGtkDisplay {
    GObject parent_instance;
    GdkDisplay *display;
    struct shoyu_shell *shoyu_shell;
    GListStore *toplevels;
};

struct _ShoyuShellGtkDisplayClass {
    GObjectClass parent_class;

    GType toplevel_type;

    ShoyuShellGtkToplevel *(*create_toplevel)(
        ShoyuShellGtkDisplay *self,
        struct shoyu_shell_toplevel *shoyu_shell_toplevel);
};
