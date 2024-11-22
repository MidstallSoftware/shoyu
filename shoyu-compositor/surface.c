#include "compositor-private.h"
#include "output-private.h"
#include "surface-private.h"

/**
 * ShoyuSurface:
 *
 * An object which represents a surface for #ShoyuCompositor.
 */

enum {
  PROP_0 = 0,
  PROP_COMPOSITOR,
  N_PROPERTIES,

  SIG_DESTROY = 0,
  SIG_REALIZED,
  SIG_UNREALIZED,
  SIG_COMMIT,
  SIG_MAP,
  N_SIGNALS,
};

static GParamSpec *shoyu_surface_props[N_PROPERTIES] = {
  NULL,
};
static guint shoyu_surface_sigs[N_SIGNALS];

G_DEFINE_TYPE(ShoyuSurface, shoyu_surface, G_TYPE_OBJECT)

static void shoyu_surface_destroy(struct wl_listener *listener, void *data) {
  ShoyuSurface *self = wl_container_of(listener, self, destroy);

  if (!self->is_invalidated) {
    shoyu_surface_unrealize(self);
    g_signal_emit(self, shoyu_surface_sigs[SIG_DESTROY], 0);
  }
}

static void shoyu_surface_commit(struct wl_listener *listener, void *data) {
  ShoyuSurface *self = wl_container_of(listener, self, commit);

  struct wlr_xdg_toplevel *wlr_xdg_toplevel =
      wlr_xdg_toplevel_try_from_wlr_surface(self->wlr_surface);
  if (wlr_xdg_toplevel != NULL) {
    if (wlr_xdg_toplevel->base->initial_commit) {
      ShoyuOutput *output = shoyu_compositor_get_xdg_toplevel_claimed_output(
          self->compositor, wlr_xdg_toplevel);
      if (output != NULL) {
        wlr_xdg_toplevel_set_size(wlr_xdg_toplevel, output->wlr_output->width,
                                  output->wlr_output->height);
      } else {
        wlr_xdg_toplevel_set_size(wlr_xdg_toplevel, 0, 0);
      }
    }
  }

  struct wlr_buffer *buffer = NULL;
  if (self->wlr_surface->buffer != NULL) {
    buffer = wlr_buffer_lock(&self->wlr_surface->buffer->base);
  }

  wlr_buffer_unlock(self->buffer);
  self->buffer = buffer;

  g_signal_emit(self, shoyu_surface_sigs[SIG_COMMIT], 0);
}

static void shoyu_surface_map(struct wl_listener *listener, void *data) {
  ShoyuSurface *self = wl_container_of(listener, self, map);

  g_signal_emit(self, shoyu_surface_sigs[SIG_MAP], 0);
}

static void shoyu_surface_finalize(GObject *object) {
  ShoyuSurface *self = SHOYU_SURFACE(object);

  g_clear_object(&self->compositor);

  G_OBJECT_CLASS(shoyu_surface_parent_class)->finalize(object);
}

static void shoyu_surface_set_property(GObject *object, guint prop_id,
                                       const GValue *value, GParamSpec *pspec) {
  ShoyuSurface *self = SHOYU_SURFACE(object);

  switch (prop_id) {
    case PROP_COMPOSITOR:
      self->compositor = g_value_dup_object(value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static void shoyu_surface_get_property(GObject *object, guint prop_id,
                                       GValue *value, GParamSpec *pspec) {
  ShoyuSurface *self = SHOYU_SURFACE(object);

  switch (prop_id) {
    case PROP_COMPOSITOR:
      g_value_set_object(value, G_OBJECT(self->compositor));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static void shoyu_surface_class_init(ShoyuSurfaceClass *class) {
  GObjectClass *object_class = G_OBJECT_CLASS(class);

  object_class->finalize = shoyu_surface_finalize;
  object_class->set_property = shoyu_surface_set_property;
  object_class->get_property = shoyu_surface_get_property;

  shoyu_surface_props[PROP_COMPOSITOR] = g_param_spec_object(
      "compositor", "Shoyu Compositor",
      "The compositor the surface comes from.", SHOYU_TYPE_COMPOSITOR,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  g_object_class_install_properties(object_class, N_PROPERTIES,
                                    shoyu_surface_props);

  /**
   * ShoyuSurface::destroy:
   * @surface: a #ShoyuSurface
   */
  shoyu_surface_sigs[SIG_DESTROY] =
      g_signal_new("destroy", SHOYU_TYPE_SURFACE, G_SIGNAL_RUN_LAST, 0, NULL,
                   NULL, NULL, G_TYPE_NONE, 0);

  /**
   * ShoyuSurface::realized:
   * @surface: a #ShoyuSurface
   * @wlr_surface: A wlroots surface
   */
  shoyu_surface_sigs[SIG_REALIZED] =
      g_signal_new("realized", SHOYU_TYPE_SURFACE, G_SIGNAL_RUN_LAST,
                   G_STRUCT_OFFSET(ShoyuSurfaceClass, realized), NULL, NULL,
                   NULL, G_TYPE_NONE, 1, G_TYPE_POINTER);

  /**
   * ShoyuSurface::unrealized:
   * @surface: a #ShoyuSurface
   */
  shoyu_surface_sigs[SIG_UNREALIZED] =
      g_signal_new("unrealized", SHOYU_TYPE_SURFACE, G_SIGNAL_RUN_LAST,
                   G_STRUCT_OFFSET(ShoyuSurfaceClass, unrealized), NULL, NULL,
                   NULL, G_TYPE_NONE, 0);

  /**
   * ShoyuSurface::commit:
   * @surface: a #ShoyuSurface
   */
  shoyu_surface_sigs[SIG_COMMIT] =
      g_signal_new("commit", SHOYU_TYPE_SURFACE, G_SIGNAL_RUN_LAST, 0, NULL,
                   NULL, NULL, G_TYPE_NONE, 0);

  /**
   * ShoyuSurface::map:
   * @surface: a #ShoyuSurface
   */
  shoyu_surface_sigs[SIG_MAP] =
      g_signal_new("map", SHOYU_TYPE_SURFACE, G_SIGNAL_RUN_LAST, 0, NULL, NULL,
                   NULL, G_TYPE_NONE, 0);
}

static void shoyu_surface_init(ShoyuSurface *self) {
  self->is_invalidated = TRUE;
}

/**
 * shoyu_surface_new: (constructor)
 *
 * Creates a #ShoyuSurface
 *
 * Returns: (transfer full): A #ShoyuSurface
 */
ShoyuSurface *shoyu_surface_new(ShoyuCompositor *compositor) {
  return SHOYU_SURFACE(
      g_object_new(SHOYU_TYPE_SURFACE, "compositor", compositor, NULL));
}

/**
 * shoyu_surface_get_compositor:
 * @self: A #ShoyuSurface
 *
 * Gets the #ShoyuCompositor which the surface comes from.
 *
 * Returns: (transfer none) (nullable): A #ShoyuCompositor
 */
ShoyuCompositor *shoyu_surface_get_compositor(ShoyuSurface *self) {
  g_return_val_if_fail(SHOYU_IS_SURFACE(self), NULL);
  return self->compositor;
}

/**
 * shoyu_surface_realize:
 * @self: A #ShoyuSurface
 * @wlr_surface: The wlroots surface
 */
void shoyu_surface_realize(ShoyuSurface *self,
                           struct wlr_surface *wlr_surface) {
  g_return_if_fail(SHOYU_IS_SURFACE(self));
  g_return_if_fail(self->wlr_surface == NULL && self->is_invalidated);

  self->wlr_surface = wlr_surface;
  self->is_invalidated = FALSE;

  self->destroy.notify = shoyu_surface_destroy;
  wl_signal_add(&self->wlr_surface->events.destroy, &self->destroy);

  self->commit.notify = shoyu_surface_commit;
  wl_signal_add(&self->wlr_surface->events.commit, &self->commit);

  self->map.notify = shoyu_surface_map;
  wl_signal_add(&self->wlr_surface->events.map, &self->map);

  g_signal_emit(self, shoyu_surface_sigs[SIG_REALIZED], 0, wlr_surface);
}

/**
 * shoyu_surface_unrealize:
 * @self: A #ShoyuSurface
 */
void shoyu_surface_unrealize(ShoyuSurface *self) {
  g_return_if_fail(SHOYU_IS_SURFACE(self));
  g_return_if_fail(self->wlr_surface != NULL && !self->is_invalidated);

  struct wlr_xdg_toplevel *wlr_xdg_toplevel =
      wlr_xdg_toplevel_try_from_wlr_surface(self->wlr_surface);
  if (wlr_xdg_toplevel != NULL) {
    ShoyuOutput *output = shoyu_compositor_get_xdg_toplevel_claimed_output(
        self->compositor, wlr_xdg_toplevel);
    if (output != NULL) {
      output->wlr_surface = NULL;
    }
  }

  wl_list_remove(&self->destroy.link);
  wl_list_remove(&self->commit.link);
  wl_list_remove(&self->map.link);

  g_clear_pointer(&self->buffer, (GDestroyNotify)wlr_buffer_unlock);

  self->wlr_surface = NULL;
  self->is_invalidated = TRUE;

  g_signal_emit(self, shoyu_surface_sigs[SIG_UNREALIZED], 0);
}
