#include "display-private.h"
#include <gdk/gdkwayland.h>

enum {
  PROP_0 = 0,
  PROP_DISPLAY,
  N_PROPERTIES,
};

static GParamSpec *shoyu_shell_gtk_display_props[N_PROPERTIES] = {
  NULL,
};

G_DEFINE_TYPE(ShoyuShellGtkDisplay, shoyu_shell_gtk_display, G_TYPE_OBJECT)

static void
shoyu_shell_display_toplevel_added(void *data, struct shoyu_shell *shoyu_shell,
                                   struct shoyu_shell_toplevel *toplevel) {}

static const struct shoyu_shell_listener shoyu_shell_listener = {
  .toplevel_added = shoyu_shell_display_toplevel_added,
};

static void shoyu_shell_gtk_display_wl_registry_global(
    void *data, struct wl_registry *wl_registry, uint32_t id, const char *iface,
    uint32_t version) {
  ShoyuShellGtkDisplay *self = SHOYU_SHELL_GTK_DISPLAY(data);

  if (g_strcmp0(iface, shoyu_shell_interface.name) == 0) {
    self->shoyu_shell =
        wl_registry_bind(wl_registry, id, &shoyu_shell_interface,
                         MIN(shoyu_shell_interface.version, version));
    shoyu_shell_add_listener(self->shoyu_shell, &shoyu_shell_listener, self);
  }
}

static void shoyu_shell_gtk_display_wl_registry_global_remove(
    void *data, struct wl_registry *wl_registry, uint32_t id) {}

static const struct wl_registry_listener wl_registry_listener = {
  .global = shoyu_shell_gtk_display_wl_registry_global,
  .global_remove = shoyu_shell_gtk_display_wl_registry_global_remove,
};

static void shoyu_shell_gtk_display_constructed(GObject *object) {
  G_OBJECT_CLASS(shoyu_shell_gtk_display_parent_class)->constructed(object);

  ShoyuShellGtkDisplay *self = SHOYU_SHELL_GTK_DISPLAY(object);

  struct wl_display *wl_display =
      gdk_wayland_display_get_wl_display(self->display);
  struct wl_registry *wl_registry = wl_display_get_registry(wl_display);

  wl_registry_add_listener(wl_registry, &wl_registry_listener, self);
  wl_display_roundtrip(wl_display);
}

static void shoyu_shell_gtk_display_finalize(GObject *object) {
  ShoyuShellGtkDisplay *self = SHOYU_SHELL_GTK_DISPLAY(object);

  if (self->display != NULL) {
    if (g_object_get_data(G_OBJECT(self->display),
                          SHOYU_SHELL_GTK_DISPLAY_KEY) != NULL) {
      g_debug("Unlinking ShoyuShellGtkDisplay#%p to GdkDisplay#%p", self,
              self->display);
      g_object_unref(self);
      g_object_set_data(G_OBJECT(self), SHOYU_SHELL_GTK_DISPLAY_KEY, NULL);
    }

    g_object_unref(self->display);
    self->display = NULL;
  }

  G_OBJECT_CLASS(shoyu_shell_gtk_display_parent_class)->finalize(object);
}

static void shoyu_shell_gtk_display_set_property(GObject *object, guint prop_id,
                                                 const GValue *value,
                                                 GParamSpec *pspec) {
  ShoyuShellGtkDisplay *self = SHOYU_SHELL_GTK_DISPLAY(object);

  switch (prop_id) {
    case PROP_DISPLAY:
      self->display = g_value_dup_object(value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static void shoyu_shell_gtk_display_get_property(GObject *object, guint prop_id,
                                                 GValue *value,
                                                 GParamSpec *pspec) {
  ShoyuShellGtkDisplay *self = SHOYU_SHELL_GTK_DISPLAY(object);

  switch (prop_id) {
    case PROP_DISPLAY:
      g_value_set_object(value, G_OBJECT(self->display));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static void
shoyu_shell_gtk_display_class_init(ShoyuShellGtkDisplayClass *class) {
  GObjectClass *object_class = G_OBJECT_CLASS(class);

  object_class->constructed = shoyu_shell_gtk_display_constructed;
  object_class->finalize = shoyu_shell_gtk_display_finalize;
  object_class->set_property = shoyu_shell_gtk_display_set_property;
  object_class->get_property = shoyu_shell_gtk_display_get_property;

  shoyu_shell_gtk_display_props[PROP_DISPLAY] = g_param_spec_object(
      "display", "GDK Display", "The display the shell is connected to.",
      GDK_TYPE_WAYLAND_DISPLAY, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  g_object_class_install_properties(object_class, N_PROPERTIES,
                                    shoyu_shell_gtk_display_props);
}

static void shoyu_shell_gtk_display_init(ShoyuShellGtkDisplay *self) {}

ShoyuShellGtkDisplay *shoyu_shell_gtk_display_get(GdkDisplay *display) {
  ShoyuShellGtkDisplay *self =
      g_object_get_data(G_OBJECT(display), SHOYU_SHELL_GTK_DISPLAY_KEY);
  if (self != NULL)
    return self;

  g_debug("Creating a new Shoyu Shell for GdkDisplay#%p", display);

  self = SHOYU_SHELL_GTK_DISPLAY(
      g_object_new(SHOYU_SHELL_GTK_TYPE_DISPLAY, "display", display, NULL));
  g_return_val_if_fail(self != NULL, NULL);

  if (self->shoyu_shell == NULL) {
    g_object_unref(self);
    g_warning("GdkDisplay#%p does not have a shoyu_shell", display);
    return NULL;
  }

  g_object_set_data_full(G_OBJECT(display), SHOYU_SHELL_GTK_DISPLAY_KEY,
                         g_object_ref(self), (GDestroyNotify)g_object_unref);
  return self;
}