#include "compositor-private.h"
#include "shell-private.h"
#include "xdg-toplevel-private.h"
#include "shell-toplevel-private.h"

enum {
  PROP_0 = 0,
  PROP_COMPOSITOR,
  N_PROPERTIES,

  SIG_DESTROY = 0,
  SIG_REALIZED,
  SIG_UNREALIZED,
  N_SIGNALS,
};

static GParamSpec* shoyu_xdg_toplevel_props[N_PROPERTIES] = { NULL, };
static guint shoyu_xdg_toplevel_sigs[N_SIGNALS];

G_DEFINE_TYPE(ShoyuXdgToplevel, shoyu_xdg_toplevel, G_TYPE_OBJECT)

static void shoyu_xdg_toplevel_destroy(struct wl_listener* listener, void* data) {
  ShoyuXdgToplevel* self = wl_container_of(listener, self, destroy);

  if (!self->is_invalidated) {
    shoyu_xdg_toplevel_unrealize(self);
    g_signal_emit(self, shoyu_xdg_toplevel_sigs[SIG_DESTROY], 0);
  }
}

static void shoyu_xdg_toplevel_finalize(GObject* object) {
  ShoyuXdgToplevel* self = SHOYU_XDG_TOPLEVEL(object);

  g_clear_object(&self->compositor);

  G_OBJECT_CLASS(shoyu_xdg_toplevel_parent_class)->finalize(object);
}

static void shoyu_xdg_toplevel_set_property(GObject* object, guint prop_id, const GValue* value, GParamSpec* pspec) {
  ShoyuXdgToplevel* self = SHOYU_XDG_TOPLEVEL(object);

  switch (prop_id) {
    case PROP_COMPOSITOR:
      self->compositor = g_value_dup_object(value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static void shoyu_xdg_toplevel_get_property(GObject* object, guint prop_id, GValue* value, GParamSpec* pspec) {
  ShoyuXdgToplevel* self = SHOYU_XDG_TOPLEVEL(object);

  switch (prop_id) {
    case PROP_COMPOSITOR:
      g_value_set_object(value, G_OBJECT(self->compositor));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static void shoyu_xdg_toplevel_class_init(ShoyuXdgToplevelClass* class) {
  GObjectClass* object_class = G_OBJECT_CLASS(class);

  object_class->finalize = shoyu_xdg_toplevel_finalize;
  object_class->set_property = shoyu_xdg_toplevel_set_property;
  object_class->get_property = shoyu_xdg_toplevel_get_property;

  shoyu_xdg_toplevel_props[PROP_COMPOSITOR] = g_param_spec_object(
      "compositor", "Shoyu Compositor",
      "The compositor the output comes from.",
      SHOYU_TYPE_COMPOSITOR, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  g_object_class_install_properties(object_class, N_PROPERTIES, shoyu_xdg_toplevel_props);

  /**
   * ShoyuXdgToplevel::destroy:
   * @output: a #ShoyuXdgToplevel
   */
  shoyu_xdg_toplevel_sigs[SIG_DESTROY] = g_signal_new(
      "destroy", SHOYU_TYPE_XDG_TOPLEVEL, G_SIGNAL_RUN_LAST,
      0, NULL, NULL, NULL, G_TYPE_NONE, 0);

  /**
   * ShoyuXdgToplevel::realized:
   * @output: a #ShoyuXdgToplevel
   * @wlr_output: A wlroots output
   */
  shoyu_xdg_toplevel_sigs[SIG_REALIZED] = g_signal_new(
      "realized", SHOYU_TYPE_XDG_TOPLEVEL, G_SIGNAL_RUN_LAST,
      G_STRUCT_OFFSET(ShoyuXdgToplevelClass, realized), NULL, NULL, NULL, G_TYPE_NONE, 1, G_TYPE_POINTER);

  /**
   * ShoyuXdgToplevel::unrealized:
   * @output: a #ShoyuXdgToplevel
   */
  shoyu_xdg_toplevel_sigs[SIG_UNREALIZED] = g_signal_new(
      "unrealized", SHOYU_TYPE_XDG_TOPLEVEL, G_SIGNAL_RUN_LAST,
      G_STRUCT_OFFSET(ShoyuXdgToplevelClass, unrealized), NULL, NULL, NULL, G_TYPE_NONE, 0);
}

static void shoyu_xdg_toplevel_init(ShoyuXdgToplevel* self) {
  self->is_invalidated = TRUE;
}

/**
 * shoyu_xdg_toplevel_new: (constructor)
 *
 * Creates a #ShoyuXdgToplevel
 *
 * Returns: (transfer full): A #ShoyuXdgToplevel
 */
ShoyuXdgToplevel* shoyu_xdg_toplevel_new(ShoyuCompositor* compositor) {
  return SHOYU_XDG_TOPLEVEL(g_object_new(SHOYU_TYPE_XDG_TOPLEVEL, "compositor", compositor, NULL));
}

/**
 * shoyu_xdg_toplevel_get_compositor:
 * @self: A #ShoyuXdgToplevel
 *
 * Gets the #ShoyuCompositor which the XDG toplevel comes from.
 *
 * Returns: (transfer none) (nullable): A #ShoyuCompositor
 */
ShoyuCompositor* shoyu_xdg_toplevel_get_compositor(ShoyuXdgToplevel* self) {
  g_return_val_if_fail(SHOYU_IS_XDG_TOPLEVEL(self), NULL);
  return self->compositor;
}

/**
 * shoyu_xdg_toplevel_realize:
 * @self: A #ShoyuXdgToplevel
 * @wlr_output: The wlroots output
 */
void shoyu_xdg_toplevel_realize(ShoyuXdgToplevel* self, struct wlr_xdg_toplevel* wlr_xdg_toplevel) {
  g_return_if_fail(SHOYU_IS_XDG_TOPLEVEL(self));
  g_return_if_fail(self->wlr_xdg_toplevel == NULL && self->is_invalidated);

  self->wlr_xdg_toplevel = wlr_xdg_toplevel;
  self->is_invalidated = FALSE;

  self->destroy.notify = shoyu_xdg_toplevel_destroy;
  wl_signal_add(&self->wlr_xdg_toplevel->events.destroy, &self->destroy);

  g_signal_emit(self, shoyu_xdg_toplevel_sigs[SIG_REALIZED], 0, wlr_xdg_toplevel);

  if (!shoyu_compositor_is_xdg_toplevel_claimed(self->compositor, wlr_xdg_toplevel) && self->compositor->shell->resource != NULL) {
    shoyu_shell_xdg_toplevel_bind_shell(self);
  }
}

/**
 * shoyu_xdg_toplevel_unrealize:
 * @self: A #ShoyuXdgToplevel
 */
void shoyu_xdg_toplevel_unrealize(ShoyuXdgToplevel* self) {
  g_return_if_fail(SHOYU_IS_XDG_TOPLEVEL(self));
  g_return_if_fail(self->wlr_xdg_toplevel != NULL && !self->is_invalidated);

  wl_list_remove(&self->destroy.link);

  self->wlr_xdg_toplevel = NULL;
  self->is_invalidated = TRUE;

  g_signal_emit(self, shoyu_xdg_toplevel_sigs[SIG_UNREALIZED], 0);

  if (!shoyu_compositor_is_xdg_toplevel_claimed(self->compositor, self->wlr_xdg_toplevel) && self->compositor->shell->resource != NULL) {
    shoyu_shell_xdg_toplevel_unbind_shell(self);
  }
}

void shoyu_shell_xdg_toplevel_bind_shell(ShoyuXdgToplevel* self) {
  shoyu_shell_toplevel_create(self->compositor->shell->client, self->compositor->shell->resource, self->wlr_xdg_toplevel);
}

void shoyu_shell_xdg_toplevel_unbind_shell(ShoyuXdgToplevel* self) {
  shoyu_shell_toplevel_delete(self->compositor->shell, self->wlr_xdg_toplevel);
}
