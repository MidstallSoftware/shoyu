#pragma once

#if !defined(__SHOYU_SHELL_GTK_H_INSIDE__) &&                                  \
    !defined(SHOYU_SHELL_GTK_COMPILATION)
#error "Only <shoyu-shell-gtk4/shoyu-shell-gtk4.h> can be included directly."
#endif

#include <gdk/gdk.h>
#include <glib.h>
#include <shoyu-shell-gtk4/version.h>

G_BEGIN_DECLS

SHOYU_SHELL_GTK_AVAILABLE_IN_ALL
void shoyu_shell_gtk_init(void);

SHOYU_SHELL_GTK_AVAILABLE_IN_ALL
gboolean shoyu_shell_gtk_init_check(void);

SHOYU_SHELL_GTK_AVAILABLE_IN_ALL
gboolean shoyu_shell_gtk_is_initialized(void);

SHOYU_SHELL_GTK_AVAILABLE_IN_ALL
gboolean shoyu_shell_gtk_init_display(GdkDisplay *display);

SHOYU_SHELL_GTK_AVAILABLE_IN_ALL
gboolean shoyu_shell_gtk_monitor_set_surface(GdkMonitor *monitor,
                                             GdkSurface *surface);

G_END_DECLS
