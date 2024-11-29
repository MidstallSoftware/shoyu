#pragma once

#include "display.h"
#include "dmabuf-private.h"
#include "toplevel.h"

#include <linux-dmabuf-v1-client-protocol.h>
#include <shoyu-shell-client-protocol.h>
#include <wayland-client.h>

#define SHOYU_SHELL_GTK_DISPLAY_KEY "shoyu-shell-gtk4-display"

struct _ShoyuShellGtkDisplay {
    GObject parent_instance;
    GdkDisplay *display;
    struct shoyu_shell *shoyu_shell;
    struct wl_shm *wl_shm;
    struct zwp_linux_dmabuf_v1 *zwp_linux_dmabuf_v1;
    DmabufFormatsInfo *dmabuf_formats_info;
    GListStore *toplevels;
};

struct _ShoyuShellGtkDisplayClass {
    GObjectClass parent_class;

    GType toplevel_type;

    ShoyuShellGtkToplevel *(*create_toplevel)(
        ShoyuShellGtkDisplay *self,
        struct shoyu_shell_toplevel *shoyu_shell_toplevel);
};
