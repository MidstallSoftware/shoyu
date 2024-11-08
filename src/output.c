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
  kSignalCreateView,
  kSignalLastSignal,
};

static GParamSpec* shoyu_output_properties[kPropLast] = { NULL, };
static guint shoyu_output_signals[kSignalLastSignal];

G_DEFINE_TYPE_WITH_CODE(ShoyuOutput, shoyu_output, G_TYPE_OBJECT,
    G_ADD_PRIVATE(ShoyuOutput));

static GtkWidget* shoyu_output_create_view(ShoyuOutput* self) {
  ShoyuOutputPrivate* priv = SHOYU_OUTPUT_GET_PRIVATE(self);

  return gtk_label_new(g_strdup_printf("Output for \"%s\"", priv->wlr_output->name));
}

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

  cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, priv->wlr_output->width, priv->wlr_output->height);
  cairo_t* cr = cairo_create(surface);

  cairo_set_source_rgb(cr, 0, 0, 0);
  cairo_rectangle(cr, 0, 0, priv->wlr_output->width * 1.0, priv->wlr_output->height * 1.0);
  cairo_fill(cr);

  if (priv->view == NULL) {
    g_signal_emit(self, shoyu_output_signals[kSignalCreateView], 0, &priv->view);
  }

  if (priv->view != NULL) {
    GtkAllocation alloc = {
      .x = 0,
      .y = 0,
      .width = priv->wlr_output->width,
      .height = priv->wlr_output->height,
    };

    gtk_widget_set_has_window(priv->view, TRUE);
    gtk_widget_set_visible(priv->view, TRUE);
    gtk_widget_set_mapped(priv->view, TRUE);
    gtk_widget_size_allocate(priv->view, &alloc);
    gtk_widget_draw(GTK_WIDGET(priv->view), cr);
  }

  cairo_destroy(cr);

  struct wlr_texture* texture = wlr_texture_from_pixels(shoyu_compositor_get_wlr_renderer(priv->compositor), DRM_FORMAT_ARGB8888, cairo_image_surface_get_stride(surface), priv->wlr_output->width, priv->wlr_output->height, (const void*)cairo_image_surface_get_data(surface));

  cairo_surface_destroy(surface);

  wlr_render_pass_add_texture(pass, &(struct wlr_render_texture_options){
    .texture = texture,
    .dst_box = { .x = 0, .y = 0 },
  });

  wlr_render_pass_submit(pass);

  wlr_output_commit_state(priv->wlr_output, &state);
  wlr_output_state_finish(&state);

  wlr_texture_destroy(texture);
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
}

static void shoyu_output_dispose(GObject* object) {
  ShoyuOutput* self = SHOYU_OUTPUT(object);
  ShoyuOutputPrivate* priv = SHOYU_OUTPUT_GET_PRIVATE(self);

  g_clear_object(&priv->compositor);
  g_clear_object(&priv->view);

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

  klass->create_view = shoyu_output_create_view;

  object_class->constructed = shoyu_output_constructed;
  object_class->dispose = shoyu_output_dispose;
  object_class->set_property = shoyu_output_set_property;
  object_class->get_property = shoyu_output_get_property;

  shoyu_output_properties[kPropCompositor] = g_param_spec_object("compositor", "Shoyu Compositor", "The compositor for the output.", shoyu_compositor_get_type(), G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
  shoyu_output_properties[kPropWlrOutput] = g_param_spec_pointer("wlr-output", "Wayland Output", "The wlroots output instance.", G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  g_object_class_install_properties(object_class, kPropLast, shoyu_output_properties);

  shoyu_output_signals[kSignalDestroy] = g_signal_new("destroy", shoyu_output_get_type(), G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL, G_TYPE_NONE, 0);
  shoyu_output_signals[kSignalCreateView] = g_signal_new(
      "create-view", shoyu_output_get_type(), G_SIGNAL_RUN_LAST,
      G_STRUCT_OFFSET(ShoyuOutputClass, create_view),
      g_signal_accumulator_first_wins, NULL, NULL, gtk_widget_get_type(), 0);
}

static void shoyu_output_init(ShoyuOutput* self) {}

ShoyuOutput* shoyu_output_new(ShoyuCompositor* compositor, struct wlr_output* wlr_output) {
  return SHOYU_OUTPUT(g_object_new(shoyu_output_get_type(), "compositor", compositor, "wlr-output", wlr_output, NULL));
}

void shoyu_output_invalidate_view(ShoyuOutput* self) {
  ShoyuOutputPrivate* priv = SHOYU_OUTPUT_GET_PRIVATE(self);
  g_clear_object(&priv->view);
  g_signal_emit(self, shoyu_output_signals[kSignalCreateView], 0, &priv->view);
}
