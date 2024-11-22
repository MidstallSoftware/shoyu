#include "shoyu-config.h"
#include "main.h"
#include "main-private.h"

#include <gdk/gdkwayland.h>
#include <glib/gi18n-lib.h>
#include <wayland-client.h>

static gboolean shoyu_shell_gtk_initialized = FALSE;

static void setlocale_initialization(void) {
  static gboolean initialized = FALSE;

  if (initialized)
    return;
  initialized = TRUE;

  if (!setlocale(LC_ALL, "")) {
    g_warning("Locale not supported by C library.\n\tUsing the fallback 'C' locale.");
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
  if (shoyu_shell_gtk_initialized) return TRUE;

  gettext_initialization();

  GdkDisplay* display = gdk_display_get_default();
  g_return_val_if_fail(display != NULL, FALSE);
  g_return_val_if_fail(shoyu_shell_gtk_init_display(display), FALSE);

  shoyu_shell_gtk_initialized = TRUE;
  return TRUE;
}

gboolean shoyu_shell_gtk_is_initialized(void) {
  return shoyu_shell_gtk_initialized;
}

static void shoyu_shell_toplevel_added(void* data, struct shoyu_shell* shoyu_shell, struct shoyu_shell_toplevel* toplevel) {
  g_debug("Toplevel added %p", toplevel);

  // TODO: listen for toplevel destroy
}

static const struct shoyu_shell_listener shoyu_shell_listener = {
  .toplevel_added = shoyu_shell_toplevel_added,
};

static void shoyu_shell_gtk_wl_registry_global(void* data, struct wl_registry* wl_registry, uint32_t id, const char* iface, uint32_t version) {
  GdkDisplay* display = GDK_DISPLAY(data);
  ShoyuShellGtkDisplay* self = g_object_get_data(G_OBJECT(display), SHOYU_SHELL_GTK_DISPLAY_KEY);

  g_debug("Found global %s.%d#%d", iface, version, id);

  if (g_strcmp0(iface, shoyu_shell_interface.name) == 0) {
    self->shoyu_shell = wl_registry_bind(wl_registry, id, &shoyu_shell_interface, MIN(shoyu_shell_interface.version, version));
    shoyu_shell_add_listener(self->shoyu_shell, &shoyu_shell_listener, self);
  }
}

static void shoyu_shell_gtk_wl_registry_global_remove(void* data, struct wl_registry* wl_registry, uint32_t id) {
  //GdkDisplay* display = GDK_DISPLAY(data);
  //ShoyuShellGtkDisplay* self = g_object_get_data(G_OBJECT(display), SHOYU_SHELL_GTK_DISPLAY_KEY);
}

static const struct wl_registry_listener shoyu_shell_gtk_wl_registry_listener = {
  .global = shoyu_shell_gtk_wl_registry_global,
  .global_remove = shoyu_shell_gtk_wl_registry_global_remove,
};

gboolean shoyu_shell_gtk_init_display(GdkDisplay* display) {
  if (g_object_get_data(G_OBJECT(display), SHOYU_SHELL_GTK_DISPLAY_KEY) != NULL) {
    return TRUE;
  }

  g_return_val_if_fail(GDK_IS_WAYLAND_DISPLAY(display), FALSE);

  ShoyuShellGtkDisplay* self = g_malloc0(sizeof (ShoyuShellGtkDisplay));
  g_assert(self != NULL);

  g_object_set_data(G_OBJECT(display), SHOYU_SHELL_GTK_DISPLAY_KEY, self);

  struct wl_display* wl_display = gdk_wayland_display_get_wl_display(display);
  struct wl_registry* wl_registry = wl_display_get_registry(wl_display);

  wl_registry_add_listener(wl_registry, &shoyu_shell_gtk_wl_registry_listener, display);
  wl_display_roundtrip(wl_display);

  if (self->shoyu_shell == NULL) {
    g_object_set_data(G_OBJECT(display), SHOYU_SHELL_GTK_DISPLAY_KEY, NULL);
    free(self);
    return FALSE;
  }

  return TRUE;
}

struct shoyu_shell_output* shoyu_shell_gtk_get_output(GdkMonitor* monitor) {
  struct shoyu_shell_output* shoyu_shell_output = g_object_get_data(G_OBJECT(monitor), SHOYU_SHELL_GTK_MONITOR_KEY);
  if (shoyu_shell_output != NULL) {
    return shoyu_shell_output;
  }

  GdkDisplay* display = gdk_monitor_get_display(monitor);
  g_return_val_if_fail(display != NULL, NULL);

  ShoyuShellGtkDisplay* self = g_object_get_data(G_OBJECT(display), SHOYU_SHELL_GTK_DISPLAY_KEY);
  g_return_val_if_fail(self != NULL, NULL);

  struct wl_output* wl_output = gdk_wayland_monitor_get_wl_output(monitor);

  shoyu_shell_output = shoyu_shell_get_output(self->shoyu_shell, wl_output);
  g_object_set_data(G_OBJECT(monitor), SHOYU_SHELL_GTK_MONITOR_KEY, shoyu_shell_output);
  return shoyu_shell_output;
}

gboolean shoyu_shell_gtk_monitor_set_window(GdkMonitor* monitor, GdkWindow* window) {
  struct shoyu_shell_output* shoyu_shell_output = shoyu_shell_gtk_get_output(monitor);
  g_return_val_if_fail(shoyu_shell_output != NULL, FALSE);

  g_return_val_if_fail(GDK_IS_WAYLAND_WINDOW(window), FALSE);
  struct wl_surface* wl_surface = gdk_wayland_window_get_wl_surface(window);
  g_return_val_if_fail(wl_surface != NULL, FALSE);

  shoyu_shell_output_set_surface(shoyu_shell_output, wl_surface);
  return TRUE;
}
