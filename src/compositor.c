#define G_LOG_DOMAIN "ShoyuCompositor"

#include <shoyu/output.h>

#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_subcompositor.h>

#include "compositor-private.h"

enum {
  kPropApplication = 1,
  kPropWlDisplay,
  kPropWlrBackend,
  kPropLast,

  kSignalCreateOutput = 0,
  kSignalLastSignal,
};

static GParamSpec* shoyu_compositor_properties[kPropLast] = { NULL, };
static guint shoyu_compositor_signals[kSignalLastSignal];

G_DEFINE_TYPE_WITH_CODE(ShoyuCompositor, shoyu_compositor, G_TYPE_OBJECT,
    G_ADD_PRIVATE(ShoyuCompositor));

static void shoyu_compositor_destroy_output(ShoyuOutput* output, ShoyuCompositor* self) {
  ShoyuCompositorPrivate* priv = SHOYU_COMPOSITOR_GET_PRIVATE(self);

  guint len = g_list_length(priv->outputs);
  priv->outputs = g_list_remove(priv->outputs, output);

  guint new_len = g_list_length(priv->outputs);
  g_debug("Outputs changed (old: %u, new: %u)", len, new_len);
  g_assert(new_len < len);

  if (priv->application != NULL) {
    g_application_release(priv->application);
  }

  g_debug("Destroyed ShoyuOutput#%p", output);
  g_object_unref(output);
}

static void shoyu_compositor_new_output(struct wl_listener* listener, void* data) {
  ShoyuCompositorPrivate* priv = wl_container_of(listener, priv, new_output);
  ShoyuCompositor* self = priv->self;

  struct wlr_output* wlr_output = (struct wlr_output*)data;

  wlr_output_init_render(wlr_output, priv->allocator, priv->wlr_renderer);

  struct wlr_output_state state;
  wlr_output_state_init(&state);
  wlr_output_state_set_enabled(&state, true);

  struct wlr_output_mode* mode = wlr_output_preferred_mode(wlr_output);
  if (mode != NULL) {
    wlr_output_state_set_mode(&state, mode);
  }

  wlr_output_commit_state(wlr_output, &state);
  wlr_output_state_finish(&state);

  guint len = g_list_length(priv->outputs);

  ShoyuOutput* output = NULL;
  g_signal_emit(self, shoyu_compositor_signals[kSignalCreateOutput], 0, wlr_output, &output);
  g_return_if_fail(output != NULL);

  priv->outputs = g_list_append(priv->outputs, output);

  guint new_len = g_list_length(priv->outputs);
  g_debug("Outputs changed (old: %u, new: %u)", len, new_len);
  g_assert(new_len > len);

  g_signal_connect(output, "wl-destroy", G_CALLBACK(shoyu_compositor_destroy_output), self);

  g_debug("Created ShoyuOutput#%p", output);

  if (priv->application != NULL) {
    g_application_hold(priv->application);
  }
}

static void shoyu_compositor_constructed(GObject* object) {
  G_OBJECT_CLASS(shoyu_compositor_parent_class)->constructed(object);

  ShoyuCompositor* self = SHOYU_COMPOSITOR(object);
  ShoyuCompositorPrivate* priv = SHOYU_COMPOSITOR_GET_PRIVATE(self);
  priv->self = self;

  g_assert(priv->wl_display != NULL);

  if (priv->wlr_backend == NULL) {
    priv->wlr_backend = wlr_backend_autocreate(wl_display_get_event_loop(priv->wl_display), NULL);
  }

  g_assert(priv->wlr_backend != NULL);

  priv->wlr_renderer = wlr_renderer_autocreate(priv->wlr_backend);
  g_assert(priv->wlr_renderer != NULL);

  wlr_renderer_init_wl_display(priv->wlr_renderer, priv->wl_display);

  priv->allocator = wlr_allocator_autocreate(priv->wlr_backend, priv->wlr_renderer);
  g_assert(priv->allocator != NULL);

  priv->outputs = NULL;
  priv->new_output.notify = shoyu_compositor_new_output;
  wl_signal_add(&priv->wlr_backend->events.new_output, &priv->new_output);

  if (!wlr_backend_start(priv->wlr_backend)) {
    g_critical("Failed to start the wlroots backend");
  }

  priv->compositor = wlr_compositor_create(priv->wl_display, 5, NULL);
  wlr_subcompositor_create(priv->wl_display);
}

static void shoyu_compositor_dispose(GObject* object) {
  ShoyuCompositor* self = SHOYU_COMPOSITOR(object);
  ShoyuCompositorPrivate* priv = SHOYU_COMPOSITOR_GET_PRIVATE(self);

  g_clear_list(&priv->outputs, g_object_unref);
  g_clear_pointer(&priv->wlr_backend, wlr_backend_destroy);

  G_OBJECT_CLASS(shoyu_compositor_parent_class)->dispose(object);
}

static void shoyu_compositor_set_property(GObject* object, guint prop_id, const GValue* value, GParamSpec* pspec) {
  ShoyuCompositor* self = SHOYU_COMPOSITOR(object);
  ShoyuCompositorPrivate* priv = SHOYU_COMPOSITOR_GET_PRIVATE(self);

  switch (prop_id) {
    case kPropApplication:
      priv->application = g_value_dup_object(value);
      break;
    case kPropWlDisplay:
      priv->wl_display = g_value_get_pointer(value);
      break;
    case kPropWlrBackend:
      priv->wlr_backend = g_value_get_pointer(value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static void shoyu_compositor_get_property(GObject* object, guint prop_id, GValue* value, GParamSpec* pspec) {
  ShoyuCompositor* self = SHOYU_COMPOSITOR(object);
  ShoyuCompositorPrivate* priv = SHOYU_COMPOSITOR_GET_PRIVATE(self);

  switch (prop_id) {
    case kPropApplication:
      g_value_set_object(value, G_OBJECT(priv->application));
      break;
    case kPropWlDisplay:
      g_value_set_pointer(value, priv->wl_display);
      break;
    case kPropWlrBackend:
      g_value_set_pointer(value, priv->wlr_backend);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static void shoyu_compositor_class_init(ShoyuCompositorClass* klass) {
  GObjectClass* object_class = G_OBJECT_CLASS(klass);

  klass->create_output = shoyu_output_new;

  object_class->constructed = shoyu_compositor_constructed;
  object_class->dispose = shoyu_compositor_dispose;
  object_class->set_property = shoyu_compositor_set_property;
  object_class->get_property = shoyu_compositor_get_property;

  shoyu_compositor_properties[kPropApplication] = g_param_spec_object("application", "Gio Application", "The application to run the compositor on.", g_application_get_type(), G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
  shoyu_compositor_properties[kPropWlDisplay] = g_param_spec_pointer("wl-display", "Wayland Display", "The Wayland display server instance.", G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
  shoyu_compositor_properties[kPropWlrBackend] = g_param_spec_pointer("wlr-backend", "Wlroots Backend", "The wlroots backend for the compositor.", G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  g_object_class_install_properties(object_class, kPropLast, shoyu_compositor_properties);

  /**
   * ShoyuCompositor::create-output:
   * @compositor: the object which received the signal
   * @wlr_output: the wlroots output which was recently created
   *
   * Returns: (transfer full): a #ShoyuOutput
   */
  shoyu_compositor_signals[kSignalCreateOutput] = g_signal_new(
      "create-output", shoyu_compositor_get_type(), G_SIGNAL_RUN_LAST,
      G_STRUCT_OFFSET(ShoyuCompositorClass, create_output),
      g_signal_accumulator_first_wins, NULL, NULL, shoyu_output_get_type(), 1, G_TYPE_POINTER);
}

static void shoyu_compositor_init(ShoyuCompositor* self) {}

ShoyuCompositor* shoyu_compositor_new(struct wl_display* wl_display) {
  return shoyu_compositor_new_with_application(wl_display, NULL);
}

ShoyuCompositor* shoyu_compositor_new_with_application(struct wl_display* wl_display, GApplication* application) {
  return shoyu_compositor_new_with_wlr_backend_with_application(wl_display, wlr_backend_autocreate(wl_display_get_event_loop(wl_display), NULL), application);
}

ShoyuCompositor* shoyu_compositor_new_with_wlr_backend(struct wl_display* wl_display, struct wlr_backend* wlr_backend) {
  return shoyu_compositor_new_with_wlr_backend_with_application(wl_display, wlr_backend, NULL);
}

ShoyuCompositor* shoyu_compositor_new_with_wlr_backend_with_application(struct wl_display* wl_display, struct wlr_backend* wlr_backend, GApplication* application) {
  return SHOYU_COMPOSITOR(g_object_new(shoyu_compositor_get_type(), "wl-display", wl_display, "wlr-backend", wlr_backend, "application", application, NULL));
}

struct wlr_renderer* shoyu_compositor_get_wlr_renderer(ShoyuCompositor* self) {
  g_return_val_if_fail(SHOYU_IS_COMPOSITOR(self), NULL);

  ShoyuCompositorPrivate* priv = SHOYU_COMPOSITOR_GET_PRIVATE(self);
  g_return_val_if_fail(priv != NULL, NULL);

  return priv->wlr_renderer;
}

/**
 * shoyu_compositor_get_outputs:
 *
 * Returns: (element-type ShoyuOutput) (transfer full): a list of #ShoyuOutput
 */
GList* shoyu_compositor_get_outputs(ShoyuCompositor* self) {
  g_return_val_if_fail(SHOYU_IS_COMPOSITOR(self), NULL);

  ShoyuCompositorPrivate* priv = SHOYU_COMPOSITOR_GET_PRIVATE(self);
  g_return_val_if_fail(priv != NULL, NULL);

  return g_list_copy_deep(priv->outputs, (GCopyFunc)g_object_ref, NULL);
}
