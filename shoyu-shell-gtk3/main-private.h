#pragma once

#include "main.h"
#include <shoyu-shell-client-protocol.h>

#define SHOYU_SHELL_GTK_DISPLAY_KEY "shoyu-shell-gtk3-display"
#define SHOYU_SHELL_GTK_MONITOR_KEY "shoyu-shell-gtk3-monitor"

typedef struct _ShoyuShellGtkDisplay {
  struct shoyu_shell* shoyu_shell;
  GList* surfaces;
} ShoyuShellGtkDisplay;

typedef struct _ShoyuShellGtkToplevel {
  GdkDisplay* display;
  struct shoyu_shell_toplevel* shoyu_shell_toplevel;
} ShoyuShellGtkToplevel;

struct shoyu_shell_output* shoyu_shell_gtk_get_output(GdkMonitor* monitor);
