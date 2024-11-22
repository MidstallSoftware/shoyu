#include "input-private.h"

/**
 * ShoyuInput:
 *
 * An input which represents something like a keyboard, mouse,
 * or other input device for #ShoyuCompositor.
 */

enum {
  PROP_0 = 0,
  PROP_COMPOSITOR,
  N_PROPERTIES,

  SIG_DESTROY = 0,
  SIG_REALIZED,
  SIG_UNREALIZED,
  N_SIGNALS,
};

static GParamSpec *shoyu_input_props[N_PROPERTIES] = {
  NULL,
};
static guint shoyu_input_sigs[N_SIGNALS];

G_DEFINE_TYPE(ShoyuInput, shoyu_input, G_TYPE_OBJECT)

static void shoyu_input_destroy(struct wl_listener *listener, void *data) {
  ShoyuInput *self = wl_container_of(listener, self, destroy);

  if (!self->is_invalidated) {
    shoyu_input_unrealize(self);
    g_signal_emit(self, shoyu_input_sigs[SIG_DESTROY], 0);
  }
}

static void shoyu_input_finalize(GObject *object) {
  ShoyuInput *self = SHOYU_INPUT(object);

  g_clear_object(&self->compositor);

  G_OBJECT_CLASS(shoyu_input_parent_class)->finalize(object);
}

static void shoyu_input_set_property(GObject *object, guint prop_id,
                                     const GValue *value, GParamSpec *pspec) {
  ShoyuInput *self = SHOYU_INPUT(object);

  switch (prop_id) {
    case PROP_COMPOSITOR:
      self->compositor = g_value_dup_object(value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static void shoyu_input_get_property(GObject *object, guint prop_id,
                                     GValue *value, GParamSpec *pspec) {
  ShoyuInput *self = SHOYU_INPUT(object);

  switch (prop_id) {
    case PROP_COMPOSITOR:
      g_value_set_object(value, G_OBJECT(self->compositor));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static void shoyu_input_class_init(ShoyuInputClass *class) {
  GObjectClass *object_class = G_OBJECT_CLASS(class);

  object_class->finalize = shoyu_input_finalize;
  object_class->set_property = shoyu_input_set_property;
  object_class->get_property = shoyu_input_get_property;

  shoyu_input_props[PROP_COMPOSITOR] = g_param_spec_object(
      "compositor", "Shoyu Compositor", "The compositor the input comes from.",
      SHOYU_TYPE_COMPOSITOR, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  g_object_class_install_properties(object_class, N_PROPERTIES,
                                    shoyu_input_props);

  /**
   * ShoyuInput::destroy:
   * @input: a #ShoyuInput
   */
  shoyu_input_sigs[SIG_DESTROY] =
      g_signal_new("destroy", SHOYU_TYPE_INPUT, G_SIGNAL_RUN_LAST, 0, NULL,
                   NULL, NULL, G_TYPE_NONE, 0);

  /**
   * ShoyuInput::realized:
   * @input: a #ShoyuInput
   * @wlr_input_device: A wlroots input device
   */
  shoyu_input_sigs[SIG_REALIZED] =
      g_signal_new("realized", SHOYU_TYPE_INPUT, G_SIGNAL_RUN_LAST,
                   G_STRUCT_OFFSET(ShoyuInputClass, realized), NULL, NULL, NULL,
                   G_TYPE_NONE, 1, G_TYPE_POINTER);

  /**
   * ShoyuInput::unrealized:
   * @input: a #ShoyuInput
   */
  shoyu_input_sigs[SIG_UNREALIZED] =
      g_signal_new("unrealized", SHOYU_TYPE_INPUT, G_SIGNAL_RUN_LAST,
                   G_STRUCT_OFFSET(ShoyuInputClass, unrealized), NULL, NULL,
                   NULL, G_TYPE_NONE, 0);
}

static void shoyu_input_init(ShoyuInput *self) { self->is_invalidated = TRUE; }

/**
 * shoyu_input_new: (constructor)
 *
 * Creates a #ShoyuInput
 *
 * Returns: (transfer full): A #ShoyuInput
 */
ShoyuInput *shoyu_input_new(ShoyuCompositor *compositor) {
  return SHOYU_INPUT(
      g_object_new(SHOYU_TYPE_INPUT, "compositor", compositor, NULL));
}

/**
 * shoyu_input_get_compositor:
 * @self: A #ShoyuInput
 *
 * Gets the #ShoyuCompositor which the input comes from.
 *
 * Returns: (transfer none) (nullable): A #ShoyuCompositor
 */
ShoyuCompositor *shoyu_input_get_compositor(ShoyuInput *self) {
  g_return_val_if_fail(SHOYU_IS_INPUT(self), NULL);
  return self->compositor;
}

/**
 * shoyu_input_realize:
 * @self: A #ShoyuInput
 * @wlr_input: The wlroots input
 */
void shoyu_input_realize(ShoyuInput *self,
                         struct wlr_input_device *wlr_input_device) {
  g_return_if_fail(SHOYU_IS_INPUT(self));
  g_return_if_fail(self->wlr_input_device == NULL && self->is_invalidated);

  self->wlr_input_device = wlr_input_device;
  self->is_invalidated = FALSE;

  self->destroy.notify = shoyu_input_destroy;
  wl_signal_add(&self->wlr_input_device->events.destroy, &self->destroy);

  g_signal_emit(self, shoyu_input_sigs[SIG_REALIZED], 0, wlr_input_device);
}

/**
 * shoyu_input_unrealize:
 * @self: A #ShoyuInput
 */
void shoyu_input_unrealize(ShoyuInput *self) {
  g_return_if_fail(SHOYU_IS_INPUT(self));
  g_return_if_fail(self->wlr_input_device != NULL && !self->is_invalidated);

  wl_list_remove(&self->destroy.link);

  self->wlr_input_device = NULL;
  self->is_invalidated = TRUE;

  g_signal_emit(self, shoyu_input_sigs[SIG_UNREALIZED], 0);
}
