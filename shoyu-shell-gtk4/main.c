#include "shoyu-config.h"
#include "main.h"
#include "main-private.h"

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
    exit(1);
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
  } else if (g_strcmp0(iface, wl_output_interface.name) == 0) {
    ShoyuShellGtkOutput* output = g_malloc0(sizeof (ShoyuShellGtkOutput));
    g_assert(output != NULL);

    output->wl_output_id = id;
    output->display = display;
    output->wl_output = wl_registry_bind(wl_registry, id, &wl_output_interface, MIN(wl_output_interface.version, version));

    if (!shoyu_shell_output_ensure_shell(output)) {
      g_warning("Failed to ensure the Shoyu Shell Output");
    }

    self->outputs = g_list_append(self->outputs, output);
  }
}

static void shoyu_shell_gtk_wl_registry_global_remove(void* data, struct wl_registry* wl_registry, uint32_t id) {
  GdkDisplay* display = GDK_DISPLAY(data);
  ShoyuShellGtkDisplay* self = g_object_get_data(G_OBJECT(display), SHOYU_SHELL_GTK_DISPLAY_KEY);

  for (GList* item = self->outputs; item != NULL; item = item->next) {
    ShoyuShellGtkOutput* output = item->data;
    if (output->wl_output_id == id) {
      self->outputs = g_list_remove_link(self->outputs, item);
      g_list_free(item);
      break;
    }
  }
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

gboolean shoyu_shell_output_ensure_shell(ShoyuShellGtkOutput* output) {
  ShoyuShellGtkDisplay* self = g_object_get_data(G_OBJECT(output->display), SHOYU_SHELL_GTK_DISPLAY_KEY);
  g_return_val_if_fail(self->shoyu_shell != NULL, FALSE);

  output->shoyu_shell_output = shoyu_shell_get_output(self->shoyu_shell, output->wl_output);
  return TRUE;
}
