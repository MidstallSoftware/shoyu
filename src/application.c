#define G_LOG_DOMAIN "ShoyuApplication"

#include "application-private.h"

#include <shoyu/wayland-event-source.h>
#include <wlr/util/log.h>
#include <wayland-server-core.h>

enum {
  PROP_0 = 0,
  PROP_WL_DISPLAY,
  N_PROPERTIES,
};

static GParamSpec* shoyu_application_props[N_PROPERTIES] = { NULL, };

G_DEFINE_TYPE_WITH_PRIVATE(ShoyuApplication, shoyu_application, g_application_get_type());

static void shoyu_wlroots_log_handler(enum wlr_log_importance importance, const char* fmt, va_list args) {
  switch (importance) {
    case WLR_SILENT:
    case WLR_LOG_IMPORTANCE_LAST:
      break;
    case WLR_ERROR:
      g_logv("wlroots", G_LOG_LEVEL_ERROR, fmt, args);
      break;
    case WLR_INFO:
      g_logv("wlroots", G_LOG_LEVEL_INFO, fmt, args);
      break;
    case WLR_DEBUG:
      g_logv("wlroots", G_LOG_LEVEL_DEBUG, fmt, args);
      break;
  }
}

static void shoyu_application_constructed(GObject* object) {
  G_OBJECT_CLASS(shoyu_application_parent_class)->constructed(object);

  ShoyuApplication* self = SHOYU_APPLICATION(object);
  ShoyuApplicationPrivate* priv = SHOYU_APPLICATION_GET_PRIVATE(self);

  wlr_log_init(WLR_DEBUG, shoyu_wlroots_log_handler);

#if GTK_MAJOR_VERSION == 3
  gtk_init(NULL, NULL);
#elif GTK_MAJOR_VERSION == 4
  gtk_init();
#endif

  priv->wl_display = wl_display_create();

  priv->wl_source = shoyu_wayland_event_source_new(priv->wl_display, wl_display_get_event_loop(priv->wl_display));
}

static void shoyu_application_dispose(GObject* object) {
  ShoyuApplication* self = SHOYU_APPLICATION(object);
  ShoyuApplicationPrivate* priv = SHOYU_APPLICATION_GET_PRIVATE(self);

  g_clear_object(&priv->compositor);
  g_clear_pointer(&priv->wl_display, wl_display_destroy);
  g_clear_pointer(&priv->wl_source, g_source_destroy);

  G_OBJECT_CLASS(shoyu_application_parent_class)->dispose(object);
}

static void shoyu_application_get_property(GObject* object, guint prop_id, GValue* value, GParamSpec* pspec) {
  ShoyuApplication* self = SHOYU_APPLICATION(object);
  ShoyuApplicationPrivate* priv = SHOYU_APPLICATION_GET_PRIVATE(self);

  switch (prop_id) {
    case PROP_WL_DISPLAY:
      g_value_set_pointer(value, priv->wl_display);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static void shoyu_application_class_init(ShoyuApplicationClass* klass) {
  GObjectClass* object_class = G_OBJECT_CLASS(klass);

  object_class->constructed = shoyu_application_constructed;
  object_class->dispose = shoyu_application_dispose;
  object_class->get_property = shoyu_application_get_property;

  /**
   * ShoyuApplication:wl-display: (getter get_wl_display)
   *
   * The Wayland display server instance associated with the application.
   */
  shoyu_application_props[PROP_WL_DISPLAY] = g_param_spec_pointer("wl-display", "Wayland Display", "The Wayland display server instance.", G_PARAM_READABLE);
  g_object_class_install_properties(object_class, N_PROPERTIES, shoyu_application_props);
}

static void shoyu_application_init(ShoyuApplication* self) {}

ShoyuApplication* shoyu_application_new(const gchar* application_id, GApplicationFlags flags) {
  return SHOYU_APPLICATION(g_object_new(shoyu_application_get_type(), "application-id", application_id, "flags", flags, NULL));
}

struct wl_display* shoyu_application_get_wl_display(ShoyuApplication* self) {
  ShoyuApplicationPrivate* priv = SHOYU_APPLICATION_GET_PRIVATE(self);
  return priv->wl_display;
}
