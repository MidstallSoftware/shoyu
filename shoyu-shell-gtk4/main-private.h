#pragma once

#include "main.h"
#include <shoyu-shell-client-protocol.h>

#define SHOYU_SHELL_GTK_DISPLAY_KEY "shoyu-shell-gtk4-display"

typedef struct _ShoyuShellGtkDisplay {
  struct shoyu_shell* shoyu_shell;
  GList* outputs;
  GList* surfaces;
} ShoyuShellGtkDisplay;

typedef struct _ShoyuShellGtkOutput {
  uint32_t wl_output_id;
  GdkDisplay* display;
  struct wl_output* wl_output;
  struct shoyu_shell_output* shoyu_shell_output;
} ShoyuShellGtkOutput;

typedef struct _ShoyuShellGtkToplevel {
  GdkDisplay* display;
  struct shoyu_shell_toplevel* shoyu_shell_toplevel;
} ShoyuShellGtkToplevel;

gboolean shoyu_shell_output_ensure_shell(ShoyuShellGtkOutput* output);
