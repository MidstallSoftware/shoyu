#include "compositor-private.h"
#include "output-private.h"

/**
 * ShoyuOutput:
 *
 * An output which represents a monitor for #ShoyuCompositor.
 */

enum {
  PROP_0 = 0,
  PROP_COMPOSITOR,
  PROP_SURFACE,
  N_PROPERTIES,

  SIG_DESTROY = 0,
  SIG_REALIZED,
  SIG_UNREALIZED,
  N_SIGNALS,
};

static GParamSpec* shoyu_output_props[N_PROPERTIES] = { NULL, };
static guint shoyu_output_sigs[N_SIGNALS];

G_DEFINE_TYPE(ShoyuOutput, shoyu_output, G_TYPE_OBJECT)

static void shoyu_output_destroy(struct wl_listener* listener, void* data) {
  ShoyuOutput* self = wl_container_of(listener, self, destroy);

  if (!self->is_invalidated) {
    shoyu_output_unrealize(self);
    g_signal_emit(self, shoyu_output_sigs[SIG_DESTROY], 0);
  }
}

static void shoyu_output_frame(struct wl_listener* listener, void* data) {
  ShoyuOutput* self = wl_container_of(listener, self, frame);

  struct wlr_output_state state;
  wlr_output_state_init(&state);

  struct wlr_render_pass* pass = wlr_output_begin_render_pass(self->wlr_output, &state, NULL, NULL);

  if (self->wlr_surface != NULL) {
    ShoyuSurface* surface = shoyu_compositor_get_surface(self->compositor, self->wlr_surface);
    if (surface != NULL) {
      struct wlr_texture* texture = wlr_surface_get_texture(self->wlr_surface);
      if (texture != NULL) {
        wlr_render_pass_add_texture(pass, &(struct wlr_render_texture_options){
          .texture = texture,
          .dst_box = { 0, 0 },
        });
      }
    }
  } else {
    wlr_render_pass_add_rect(pass, &(struct wlr_render_rect_options){
      .box = { self->wlr_output->width, self->wlr_output->height },
      .color = { 0, 0, 0, 1.0 },
    });
  }

  wlr_render_pass_submit(pass);
  wlr_output_commit_state(self->wlr_output, &state);
  wlr_output_state_finish(&state);

  if (self->wlr_surface != NULL) {
	  struct timespec now;
	  clock_gettime(CLOCK_MONOTONIC, &now);
    wlr_surface_send_frame_done(self->wlr_surface, &now);
  }
}

static void shoyu_output_request_state(struct wl_listener* listener, void* data) {
  ShoyuOutput* self = wl_container_of(listener, self, request_state);

  const struct wlr_output_event_request_state* event = data;

  struct wlr_output_state state;
  wlr_output_state_init(&state);
  wlr_output_state_copy(&state, event->state);

  // TODO: pass this to a signal to determine what should be done.

  wlr_output_commit_state(self->wlr_output, &state);

  if (self->wlr_surface != NULL) {
    struct wlr_xdg_toplevel* xdg_toplevel = wlr_xdg_toplevel_try_from_wlr_surface(self->wlr_surface);
    if (xdg_toplevel != NULL) {
      wlr_xdg_toplevel_set_size(xdg_toplevel, self->wlr_output->width, self->wlr_output->height);
    }
  }
}

static void shoyu_output_finalize(GObject* object) {
  ShoyuOutput* self = SHOYU_OUTPUT(object);

  if (!self->is_invalidated) g_clear_pointer(&self->wlr_output, (GDestroyNotify) wlr_output_destroy);
  g_clear_object(&self->compositor);

  G_OBJECT_CLASS(shoyu_output_parent_class)->finalize(object);
}

static void shoyu_output_set_property(GObject* object, guint prop_id, const GValue* value, GParamSpec* pspec) {
  ShoyuOutput* self = SHOYU_OUTPUT(object);

  switch (prop_id) {
    case PROP_COMPOSITOR:
      self->compositor = g_value_dup_object(value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static void shoyu_output_get_property(GObject* object, guint prop_id, GValue* value, GParamSpec* pspec) {
  ShoyuOutput* self = SHOYU_OUTPUT(object);

  switch (prop_id) {
    case PROP_COMPOSITOR:
      g_value_set_object(value, G_OBJECT(self->compositor));
      break;
    case PROP_SURFACE:
      g_value_set_pointer(value, self->wlr_surface);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static void shoyu_output_class_init(ShoyuOutputClass* class) {
  GObjectClass* object_class = G_OBJECT_CLASS(class);

  object_class->finalize = shoyu_output_finalize;
  object_class->set_property = shoyu_output_set_property;
  object_class->get_property = shoyu_output_get_property;

  shoyu_output_props[PROP_COMPOSITOR] = g_param_spec_object(
      "compositor", "Shoyu Compositor",
      "The compositor the output comes from.",
      SHOYU_TYPE_COMPOSITOR, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  shoyu_output_props[PROP_SURFACE] = g_param_spec_pointer(
      "surface", "Wayland surface",
      "The surface which is to be rendered onto the output.",
      G_PARAM_READABLE);

  g_object_class_install_properties(object_class, N_PROPERTIES, shoyu_output_props);

  /**
   * ShoyuOutput::destroy:
   * @output: a #ShoyuOutput
   */
  shoyu_output_sigs[SIG_DESTROY] = g_signal_new(
      "destroy", SHOYU_TYPE_OUTPUT, G_SIGNAL_RUN_LAST,
      0, NULL, NULL, NULL, G_TYPE_NONE, 0);

  /**
   * ShoyuOutput::realized:
   * @output: a #ShoyuOutput
   * @wlr_output: A wlroots output
   */
  shoyu_output_sigs[SIG_REALIZED] = g_signal_new(
      "realized", SHOYU_TYPE_OUTPUT, G_SIGNAL_RUN_LAST,
      G_STRUCT_OFFSET(ShoyuOutputClass, realized), NULL, NULL, NULL, G_TYPE_NONE, 1, G_TYPE_POINTER);

  /**
   * ShoyuOutput::unrealized:
   * @output: a #ShoyuOutput
   */
  shoyu_output_sigs[SIG_UNREALIZED] = g_signal_new(
      "unrealized", SHOYU_TYPE_OUTPUT, G_SIGNAL_RUN_LAST,
      G_STRUCT_OFFSET(ShoyuOutputClass, unrealized), NULL, NULL, NULL, G_TYPE_NONE, 0);
}

static void shoyu_output_init(ShoyuOutput* self) {
  self->is_invalidated = TRUE;
}

/**
 * shoyu_output_new: (constructor)
 *
 * Creates a #ShoyuOutput
 *
 * Returns: (transfer full): A #ShoyuOutput
 */
ShoyuOutput* shoyu_output_new(ShoyuCompositor* compositor) {
  return SHOYU_OUTPUT(g_object_new(SHOYU_TYPE_OUTPUT, "compositor", compositor, NULL));
}

/**
 * shoyu_output_get_compositor:
 * @self: A #ShoyuOutput
 *
 * Gets the #ShoyuCompositor which the output comes from.
 *
 * Returns: (transfer none) (nullable): A #ShoyuCompositor
 */
ShoyuCompositor* shoyu_output_get_compositor(ShoyuOutput* self) {
  g_return_val_if_fail(SHOYU_IS_OUTPUT(self), NULL);
  return self->compositor;
}

/**
 * shoyu_output_realize:
 * @self: A #ShoyuOutput
 * @wlr_output: The wlroots output
 */
void shoyu_output_realize(ShoyuOutput* self, struct wlr_output* wlr_output) {
  g_return_if_fail(SHOYU_IS_OUTPUT(self));
  g_return_if_fail(self->wlr_output == NULL && self->is_invalidated);

  self->wlr_output = wlr_output;
  self->is_invalidated = FALSE;

  self->destroy.notify = shoyu_output_destroy;
  wl_signal_add(&self->wlr_output->events.destroy, &self->destroy);

  self->frame.notify = shoyu_output_frame;
  wl_signal_add(&self->wlr_output->events.frame, &self->frame);

  self->request_state.notify = shoyu_output_request_state;
  wl_signal_add(&self->wlr_output->events.request_state, &self->request_state);

  self->wlr_output_layout_output = wlr_output_layout_add_auto(self->compositor->output_layout, wlr_output);
  g_assert(self->wlr_output_layout_output != NULL);

  g_signal_emit(self, shoyu_output_sigs[SIG_REALIZED], 0, wlr_output);
}

/**
 * shoyu_output_unrealize:
 * @self: A #ShoyuOutput
 */
void shoyu_output_unrealize(ShoyuOutput* self) {
  g_return_if_fail(SHOYU_IS_OUTPUT(self));
  g_return_if_fail(self->wlr_output != NULL && !self->is_invalidated);

  wl_list_remove(&self->destroy.link);
  wl_list_remove(&self->frame.link);
  wl_list_remove(&self->request_state.link);

  self->wlr_output = NULL;
  self->is_invalidated = TRUE;

  g_signal_emit(self, shoyu_output_sigs[SIG_UNREALIZED], 0);
}

void shoyu_output_set_surface(ShoyuOutput* self, struct wlr_surface* wlr_surface) {
  g_return_if_fail(SHOYU_IS_OUTPUT(self));

  self->wlr_surface = wlr_surface;

  g_object_notify_by_pspec(G_OBJECT(self), shoyu_output_props[PROP_SURFACE]);
}
