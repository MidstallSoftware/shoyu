#include "main.h"
#include "display-private.h"
#include "display.h"
#include "main-private.h"
#include "shoyu-config.h"

#include <gdk/wayland/gdkwayland.h>
#include <glib/gi18n-lib.h>
#include <wayland-client.h>

static gboolean shoyu_shell_gtk_initialized = FALSE;

static void setlocale_initialization(void) {
  static gboolean initialized = FALSE;

  if (initialized)
    return;
  initialized = TRUE;

  if (!setlocale(LC_ALL, "")) {
    g_warning(
        "Locale not supported by C library.\n\tUsing the fallback 'C' locale.");
  }
}

static void gettext_initialization(void) {
  setlocale_initialization();

  bindtextdomain(GETTEXT_PACKAGE, SHOYU_LOCALEDIR);
#ifdef HAVE_BIND_TEXTDOMAIN_CODESET
  bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
#endif
}

void shoyu_shell_gtk_init(void) {
  if (!shoyu_shell_gtk_init_check()) {
    g_warning("Failed to initialize");
  }
}

gboolean shoyu_shell_gtk_init_check(void) {
  if (shoyu_shell_gtk_initialized)
    return TRUE;

  gettext_initialization();

  GdkDisplay *display = gdk_display_get_default();
  g_return_val_if_fail(display != NULL, FALSE);
  g_return_val_if_fail(shoyu_shell_gtk_display_get(display) != NULL, FALSE);

  shoyu_shell_gtk_initialized = TRUE;
  return TRUE;
}

gboolean shoyu_shell_gtk_is_initialized(void) {
  return shoyu_shell_gtk_initialized;
}

struct shoyu_shell_output *shoyu_shell_gtk_get_output(GdkMonitor *monitor) {
  struct shoyu_shell_output *shoyu_shell_output =
      g_object_get_data(G_OBJECT(monitor), SHOYU_SHELL_GTK_MONITOR_KEY);
  if (shoyu_shell_output != NULL) {
    return shoyu_shell_output;
  }

  GdkDisplay *display = gdk_monitor_get_display(monitor);
  g_return_val_if_fail(display != NULL, NULL);

  ShoyuShellGtkDisplay *self = shoyu_shell_gtk_display_get(display);
  g_return_val_if_fail(self != NULL, NULL);

  struct wl_output *wl_output = gdk_wayland_monitor_get_wl_output(monitor);

  shoyu_shell_output = shoyu_shell_get_output(self->shoyu_shell, wl_output);
  g_object_set_data(G_OBJECT(monitor), SHOYU_SHELL_GTK_MONITOR_KEY,
                    shoyu_shell_output);
  return shoyu_shell_output;
}

gboolean shoyu_shell_gtk_monitor_set_surface(GdkMonitor *monitor,
                                             GdkSurface *surface) {
  struct shoyu_shell_output *shoyu_shell_output =
      shoyu_shell_gtk_get_output(monitor);
  g_return_val_if_fail(shoyu_shell_output != NULL, FALSE);

  g_return_val_if_fail(GDK_IS_WAYLAND_SURFACE(surface), FALSE);
  struct wl_surface *wl_surface = gdk_wayland_surface_get_wl_surface(surface);
  g_return_val_if_fail(wl_surface != NULL, FALSE);

  shoyu_shell_output_set_surface(shoyu_shell_output, wl_surface);
  return TRUE;
}
