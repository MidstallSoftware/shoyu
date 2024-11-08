#define G_LOG_DOMAIN "ShoyuOutput"

#include <drm_fourcc.h>
#include <wlr/render/drm_format_set.h>
#include <wlr/types/wlr_output.h>
#include <cairo.h>
#include "compositor-private.h"
#include "output-private.h"

enum {
  kPropCompositor = 1,
  kPropWlrOutput,
  kPropLast,

  kSignalDestroy = 0,
  kSignalRequestState,
  kSignalLastSignal,
};

static GParamSpec* shoyu_output_properties[kPropLast] = { NULL, };
static guint shoyu_output_signals[kSignalLastSignal];

#if GTK_MAJOR_VERSION == 4
// NOTE: keep in sync with "gtk/gtknativeprivate.h"
struct _GtkNativeInterface {
  GTypeInterface g_iface;

  GdkSurface* (*get_surface)(GtkNative* self);
  GskRenderer* (*get_renderer)(GtkNative* self);
  void (*get_surface_transform)(GtkNative* self, double* x, double* y);
  void (*layout)(GtkNative* self, int width, int height);
};

static void shoyu_output_native_iface_init(GtkNativeInterface* iface);

G_DEFINE_TYPE_WITH_CODE(ShoyuOutput, shoyu_output, GTK_TYPE_WIDGET,
    G_ADD_PRIVATE(ShoyuOutput)
    G_IMPLEMENT_INTERFACE(GTK_TYPE_NATIVE, shoyu_output_native_iface_init));
#elif GTK_MAJOR_VERSION == 3
G_DEFINE_TYPE_WITH_PRIVATE(ShoyuOutput, shoyu_output, GTK_TYPE_BIN);
#endif

static gboolean shoyu_output_request_state_default(ShoyuOutput* self, struct wlr_output_state* event) {
  return TRUE;
}

#if GTK_MAJOR_VERSION == 4
static void shoyu_output_native_iface_init(GtkNativeInterface* iface) {}
#elif GTK_MAJOR_VERSION == 3
static gboolean shoyu_output_draw(GtkWidget* widget, cairo_t* cr) {
  gboolean ret = FALSE;

  GtkStyleContext* context = gtk_widget_get_style_context(widget);

  GtkAllocation allocation;
  gtk_widget_get_allocation(widget, &allocation);

  gtk_render_background(context, cr, 0, 0, allocation.width, allocation.height);
  gtk_render_frame(context, cr, 0, 0, allocation.width, allocation.height);

  if (GTK_WIDGET_CLASS(shoyu_output_parent_class)->draw) {
    ret = GTK_WIDGET_CLASS(shoyu_output_parent_class)->draw(widget, cr);
  }

  return ret;
}
#endif

static void shoyu_output_destroy(struct wl_listener* listener, void* data) {
  ShoyuOutputPrivate* priv = wl_container_of(listener, priv, destroy);
  ShoyuOutput* self = priv->self;

  if (!priv->is_destroying) {
    priv->is_destroying = TRUE;
    g_signal_emit(self, shoyu_output_signals[kSignalDestroy], 0);
  }
}

static void shoyu_output_frame(struct wl_listener* listener, void* data) {
  ShoyuOutputPrivate* priv = wl_container_of(listener, priv, frame);
  ShoyuOutput* self = priv->self;

  struct wlr_output_state state;
  wlr_output_state_init(&state);

  struct wlr_render_pass* pass = wlr_output_begin_render_pass(priv->wlr_output, &state, NULL, NULL);

#if GTK_MAJOR_VERSION == 3
  cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, priv->wlr_output->width, priv->wlr_output->height);
  cairo_t* cr = cairo_create(surface);

  cairo_set_source_rgb(cr, 0, 0, 0);
  cairo_rectangle(cr, 0, 0, priv->wlr_output->width * 1.0, priv->wlr_output->height * 1.0);
  cairo_fill(cr);

  GtkAllocation alloc = {
    .x = 0,
    .y = 0,
    .width = priv->wlr_output->width,
    .height = priv->wlr_output->height,
  };

  gtk_widget_size_allocate(GTK_WIDGET(self), &alloc);
  gtk_widget_draw(GTK_WIDGET(self), cr);

  cairo_destroy(cr);
#elif GTK_MAJOR_VERSION == 4
  graphene_rect_t bounds;
  graphene_rect_init(&bounds, 0, 0, priv->wlr_output->width, priv->wlr_output->height);

  gtk_widget_queue_draw(GTK_WIDGET(self));

  GtkSnapshot* snapshot = gtk_snapshot_new();

  GdkRGBA color = { 0, 1.0, 0, 1.0 };
  gtk_snapshot_append_color(snapshot, &color, &bounds);

  GTK_WIDGET_GET_CLASS(self)->snapshot(GTK_WIDGET(self), snapshot);

  GskRenderNode* node = gtk_snapshot_free_to_node(snapshot);

  cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, priv->wlr_output->width, priv->wlr_output->height);
  cairo_t* cr = cairo_create(surface);

  cairo_set_source_rgb(cr, 0, 0, 0);
  cairo_rectangle(cr, 0, 0, priv->wlr_output->width * 1.0, priv->wlr_output->height * 1.0);
  cairo_fill(cr);

  gsk_render_node_draw(node, cr);
  gsk_render_node_unref(node);

  cairo_destroy(cr);
#endif

  struct wlr_texture* texture = wlr_texture_from_pixels(shoyu_compositor_get_wlr_renderer(priv->compositor), DRM_FORMAT_ARGB8888, cairo_image_surface_get_stride(surface), priv->wlr_output->width, priv->wlr_output->height, (const void*)cairo_image_surface_get_data(surface));

#if GTK_MAJOR_VERSION == 3
  cairo_surface_destroy(surface);
#endif

  wlr_render_pass_add_texture(pass, &(struct wlr_render_texture_options){
    .texture = texture,
    .dst_box = { .x = 0, .y = 0 },
  });

  wlr_render_pass_submit(pass);

  wlr_output_commit_state(priv->wlr_output, &state);
  wlr_output_state_finish(&state);

  wlr_texture_destroy(texture);
}

static void shoyu_output_request_state(struct wl_listener* listener, void* data) {
  ShoyuOutputPrivate* priv = wl_container_of(listener, priv, request_state);
  ShoyuOutput* self = priv->self;

  const struct wlr_output_event_request_state* event = data;

  struct wlr_output_state state;
  wlr_output_state_init(&state);
  wlr_output_state_copy(&state, event->state);

  gboolean should_commit = TRUE;
  g_signal_emit(self, shoyu_output_signals[kSignalRequestState], 0, &state, &should_commit);

  if (should_commit) wlr_output_commit_state(priv->wlr_output, &state);
}

static void shoyu_output_constructed(GObject* object) {
  G_OBJECT_CLASS(shoyu_output_parent_class)->constructed(object);

  ShoyuOutput* self = SHOYU_OUTPUT(object);
  ShoyuOutputPrivate* priv = SHOYU_OUTPUT_GET_PRIVATE(self);
  priv->self = self;

  priv->destroy.notify = shoyu_output_destroy;
  wl_signal_add(&priv->wlr_output->events.destroy, &priv->destroy);

  priv->frame.notify = shoyu_output_frame;
  wl_signal_add(&priv->wlr_output->events.frame, &priv->frame);

  priv->request_state.notify = shoyu_output_request_state;
  wl_signal_add(&priv->wlr_output->events.request_state, &priv->request_state);

#if GTK_MAJOR_VERSION == 3
  GtkWidgetPath* widget_path = gtk_widget_get_path(GTK_WIDGET(self));
  gtk_widget_path_append_type(widget_path, GTK_TYPE_WINDOW);
#endif

  GtkStyleContext* context = gtk_widget_get_style_context(GTK_WIDGET(self));

#if GTK_MAJOR_VERSION == 3
  gtk_style_context_add_class(context, GTK_STYLE_CLASS_BACKGROUND);
#elif GTK_MAJOR_VERSION == 4
  gtk_style_context_add_class(context, "background");
#endif
}

static void shoyu_output_dispose(GObject* object) {
  ShoyuOutput* self = SHOYU_OUTPUT(object);
  ShoyuOutputPrivate* priv = SHOYU_OUTPUT_GET_PRIVATE(self);

  g_clear_object(&priv->compositor);

  if (!priv->is_destroying) {
    g_clear_pointer(&priv->wlr_output, wlr_output_destroy);
  }

  G_OBJECT_CLASS(shoyu_output_parent_class)->dispose(object);
}

static void shoyu_output_set_property(GObject* object, guint prop_id, const GValue* value, GParamSpec* pspec) {
  ShoyuOutput* self = SHOYU_OUTPUT(object);
  ShoyuOutputPrivate* priv = SHOYU_OUTPUT_GET_PRIVATE(self);

  switch (prop_id) {
    case kPropCompositor:
      priv->compositor = g_value_dup_object(value);
      break;
    case kPropWlrOutput:
      priv->wlr_output = g_value_get_pointer(value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static void shoyu_output_get_property(GObject* object, guint prop_id, GValue* value, GParamSpec* pspec) {
  ShoyuOutput* self = SHOYU_OUTPUT(object);
  ShoyuOutputPrivate* priv = SHOYU_OUTPUT_GET_PRIVATE(self);

  switch (prop_id) {
    case kPropCompositor:
      g_value_set_object(value, G_OBJECT(priv->compositor));
      break;
    case kPropWlrOutput:
      g_value_set_pointer(value, priv->wlr_output);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static void shoyu_output_class_init(ShoyuOutputClass* klass) {
  GObjectClass* object_class = G_OBJECT_CLASS(klass);
  GtkWidgetClass* widget_class = GTK_WIDGET_CLASS(klass);

  klass->request_state = shoyu_output_request_state_default;

#if GTK_MAJOR_VERSION == 3
  widget_class->draw = shoyu_output_draw;
#endif

  object_class->constructed = shoyu_output_constructed;
  object_class->dispose = shoyu_output_dispose;
  object_class->set_property = shoyu_output_set_property;
  object_class->get_property = shoyu_output_get_property;

  shoyu_output_properties[kPropCompositor] = g_param_spec_object("compositor", "Shoyu Compositor", "The compositor for the output.", shoyu_compositor_get_type(), G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
  shoyu_output_properties[kPropWlrOutput] = g_param_spec_pointer("wlr-output", "Wayland Output", "The wlroots output instance.", G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  g_object_class_install_properties(object_class, kPropLast, shoyu_output_properties);

  shoyu_output_signals[kSignalDestroy] = g_signal_new("wl-destroy", shoyu_output_get_type(), G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL, G_TYPE_NONE, 0);
  shoyu_output_signals[kSignalRequestState] = g_signal_new(
    "request-state", shoyu_output_get_type(), G_SIGNAL_RUN_LAST,
    G_STRUCT_OFFSET(ShoyuOutputClass, request_state),
    g_signal_accumulator_first_wins, NULL, NULL, G_TYPE_BOOLEAN, 1, G_TYPE_POINTER);

  gtk_widget_class_set_css_name(widget_class, "output");
}

static void shoyu_output_init(ShoyuOutput* self) {
#if GTK_MAJOR_VERSION == 3
  gtk_widget_set_has_window(GTK_WIDGET(self), TRUE);
  gtk_widget_set_mapped(GTK_WIDGET(self), TRUE);
#endif

  gtk_widget_set_visible(GTK_WIDGET(self), TRUE);
}

ShoyuOutput* shoyu_output_new(ShoyuCompositor* compositor, struct wlr_output* wlr_output) {
  return SHOYU_OUTPUT(g_object_new(shoyu_output_get_type(), "compositor", compositor, "wlr-output", wlr_output, NULL));
}
