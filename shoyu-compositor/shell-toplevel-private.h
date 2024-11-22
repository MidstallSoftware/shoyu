#pragma once

#include "shell.h"
#include <wayland-server.h>
#include <wlr/types/wlr_xdg_shell.h>

void shoyu_shell_toplevel_create(struct wl_client* wl_client, struct wl_resource* shell_resource, struct wlr_xdg_toplevel* xdg_toplevel);
void shoyu_shell_toplevel_delete(ShoyuShell* shell, struct wlr_xdg_toplevel* xdg_toplevel);
