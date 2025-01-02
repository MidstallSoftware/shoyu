#include "compositor-private.h"
#include "shell-output-private.h"
#include "shell-private.h"
#include "shell-toplevel-private.h"
#include "xdg-toplevel-private.h"

/**
 * ShoyuShell:
 *
 * An object which provides the Shoyu Shell Wayland protocol.
 */

enum {
  PROP_0 = 0,
  PROP_COMPOSITOR,
  N_PROPERTIES,
};

static GParamSpec *shoyu_shell_props[N_PROPERTIES] = {
  NULL,
};

G_DEFINE_TYPE(ShoyuShell, shoyu_shell, G_TYPE_OBJECT)

static void shoyu_shell_clear_focus(struct wl_client *wl_client,
                                    struct wl_resource *shell_resource) {
  ShoyuShell *shell = wl_resource_get_user_data(shell_resource);

  for (GList *item = shell->compositor->xdg_toplevels; item != NULL;
       item = item->next) {
    ShoyuXdgToplevel *xdg_toplevel = SHOYU_XDG_TOPLEVEL(item->data);

    g_object_set(xdg_toplevel, "focus", FALSE, NULL);
  }
}

static const struct shoyu_shell_interface shoyu_shell_impl = {
  .get_output = shoyu_shell_get_output,
  .clear_focus = shoyu_shell_clear_focus,
};

static void shoyu_shell_resource_destroy(struct wl_resource *resource) {
  ShoyuShell *self = wl_resource_get_user_data(resource);

  for (GList *item = self->compositor->xdg_toplevels; item != NULL;
       item = item->next) {
    ShoyuXdgToplevel *xdg_toplevel = SHOYU_XDG_TOPLEVEL(item->data);
    if (xdg_toplevel->is_invalidated)
      continue;

    shoyu_shell_xdg_toplevel_unbind_shell(xdg_toplevel);
  }

  self->client = NULL;
  self->resource = NULL;
  self->version = 0;
}

static void shoyu_shell_bind(struct wl_client *wl_client, void *data,
                             uint32_t version, uint32_t id) {
  ShoyuShell *self = SHOYU_SHELL(data);

  g_debug("wl_client#%p wants to bind to ShoyuShell#%p", wl_client, self);

  struct wl_resource *resource =
      wl_resource_create(wl_client, &shoyu_shell_interface, version, id);

  if (self->resource != NULL) {
    wl_resource_post_error(
        resource, WL_DISPLAY_ERROR_INVALID_OBJECT,
        "Only a single client can bind to the Shoyu Shell protocol");
    return;
  }

  wl_resource_set_implementation(resource, &shoyu_shell_impl, self,
                                 shoyu_shell_resource_destroy);

  self->resource = resource;
  self->version = version;
  self->client = wl_client;

  for (GList *item = self->compositor->xdg_toplevels; item != NULL;
       item = item->next) {
    ShoyuXdgToplevel *xdg_toplevel = SHOYU_XDG_TOPLEVEL(item->data);
    if (xdg_toplevel->is_invalidated)
      continue;

    shoyu_shell_xdg_toplevel_bind_shell(xdg_toplevel);
  }
}

static void shoyu_shell_display_destroy(struct wl_listener *listener,
                                        void *data) {
  ShoyuShell *self = wl_container_of(listener, self, display_destroy);
  g_object_unref(self);
}

static void shoyu_shell_constructed(GObject *object) {
  G_OBJECT_CLASS(shoyu_shell_parent_class)->constructed(object);

  ShoyuShell *self = SHOYU_SHELL(object);

  g_assert(self->compositor != NULL);

  self->global =
      wl_global_create(self->compositor->wl_display, &shoyu_shell_interface, 1,
                       g_object_ref(self), shoyu_shell_bind);
  g_assert(self->global);

  self->display_destroy.notify = shoyu_shell_display_destroy;
  wl_display_add_destroy_listener(self->compositor->wl_display,
                                  &self->display_destroy);
}

static void shoyu_shell_finalize(GObject *object) {
  ShoyuShell *self = SHOYU_SHELL(object);

  wl_list_remove(&self->display_destroy.link);

  g_clear_pointer(&self->global, (GDestroyNotify)wl_global_destroy);
  g_clear_object(&self->compositor);

  G_OBJECT_CLASS(shoyu_shell_parent_class)->finalize(object);
}

static void shoyu_shell_set_property(GObject *object, guint prop_id,
                                     const GValue *value, GParamSpec *pspec) {
  ShoyuShell *self = SHOYU_SHELL(object);

  switch (prop_id) {
    case PROP_COMPOSITOR:
      self->compositor = g_value_dup_object(value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static void shoyu_shell_get_property(GObject *object, guint prop_id,
                                     GValue *value, GParamSpec *pspec) {
  ShoyuShell *self = SHOYU_SHELL(object);

  switch (prop_id) {
    case PROP_COMPOSITOR:
      g_value_set_object(value, G_OBJECT(self->compositor));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static void shoyu_shell_class_init(ShoyuShellClass *class) {
  GObjectClass *object_class = G_OBJECT_CLASS(class);

  object_class->constructed = shoyu_shell_constructed;
  object_class->finalize = shoyu_shell_finalize;
  object_class->set_property = shoyu_shell_set_property;
  object_class->get_property = shoyu_shell_get_property;

  shoyu_shell_props[PROP_COMPOSITOR] = g_param_spec_object(
      "compositor", "Shoyu Compositor", "The compositor the shell comes from.",
      SHOYU_TYPE_COMPOSITOR, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  g_object_class_install_properties(object_class, N_PROPERTIES,
                                    shoyu_shell_props);
}

static void shoyu_shell_init(ShoyuShell *self) {}

/**
 * shoyu_shell_new: (constructor)
 *
 * Creates a #ShoyuShell
 *
 * Returns: (transfer full): A #ShoyuShell
 */
ShoyuShell *shoyu_shell_new(ShoyuCompositor *compositor) {
  return SHOYU_SHELL(
      g_object_new(SHOYU_TYPE_SHELL, "compositor", compositor, NULL));
}
