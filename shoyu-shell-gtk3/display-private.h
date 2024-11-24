#pragma once

#include "display.h"
#include <shoyu-shell-client-protocol.h>

#define SHOYU_SHELL_GTK_DISPLAY_KEY "shoyu-shell-gtk4-display"

struct _ShoyuShellGtkDisplay {
    GObject parent_instance;
    GdkDisplay *display;
    struct shoyu_shell *shoyu_shell;
    GList *surfaces;
};

struct _ShoyuShellGtkDisplayClass {
    GObjectClass parent_class;
};
