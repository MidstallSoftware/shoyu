#pragma once

#include "shell.h"
#include <wayland-server.h>

void shoyu_shell_get_surface(struct wl_client* wl_client, struct wl_resource* shell_resource, uint32_t id, struct wl_resource* surface);
