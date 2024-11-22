#pragma once

#include "shell.h"
#include <shoyu-shell-server-protocol.h>

struct _ShoyuShell {
  GObject parent_instance;

  ShoyuCompositor* compositor;

  struct wl_global* global;
  struct wl_listener display_destroy;

  struct wl_resource* resource;
  uint32_t version;
};

struct _ShoyuShellClass {
  GObjectClass parent_class;
};

ShoyuShell* shoyu_shell_new(ShoyuCompositor* compositor);
