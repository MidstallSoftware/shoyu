#include "display-private.h"
#include "toplevel-private.h"
#include <drm_fourcc.h>

enum {
  PROP_0 = 0,
  PROP_DISPLAY,
  PROP_TEXTURE,
  PROP_TITLE,
  PROP_APP_ID,
  N_PROPERTIES,

  SIG_DESTROY = 0,
  SIG_CONFIGURE,
  SIG_REALIZED,
  SIG_UNREALIZED,
  SIG_FRAME,
  N_SIGNALS,
};

static GParamSpec *shoyu_shell_gtk_toplevel_props[N_PROPERTIES] = {
  NULL,
};
static guint shoyu_shell_gtk_toplevel_sigs[N_SIGNALS];

G_DEFINE_TYPE(ShoyuShellGtkToplevel, shoyu_shell_gtk_toplevel, G_TYPE_OBJECT)

struct CreateBufferData {
    struct wl_buffer *buffer;
    gboolean done;
};

static void shoyu_shell_gtk_toplevel_linux_buffer_created(
    void *data, struct zwp_linux_buffer_params_v1 *zwp_linux_buffer_params_v1,
    struct wl_buffer *buffer) {
  struct CreateBufferData *cbd = data;
  cbd->buffer = buffer;
  cbd->done = TRUE;
}

static void shoyu_shell_gtk_toplevel_linux_buffer_failed(
    void *data, struct zwp_linux_buffer_params_v1 *zwp_linux_buffer_params_v1) {
  struct CreateBufferData *cbd = data;
  cbd->done = TRUE;
  g_warning("Failed to create a Wayland buffer for ShoyuShellGtkToplevel#%p",
            data);
}

static const struct zwp_linux_buffer_params_v1_listener
    zwp_linux_buffer_params_v1_listener = {
      .created = shoyu_shell_gtk_toplevel_linux_buffer_created,
      .failed = shoyu_shell_gtk_toplevel_linux_buffer_failed,
};

static void
shoyu_shell_gtk_toplevel_capture_done(void *data,
                                      struct shoyu_shell_capture *capture) {
  gboolean *capture_done_ptr = data;
  *capture_done_ptr = TRUE;
}

static const struct shoyu_shell_capture_listener shoyu_shell_capture_listener =
    {
      .done = shoyu_shell_gtk_toplevel_capture_done,
};

static void shoyu_shell_gtk_toplevel_drm_format(
    void *data, struct shoyu_shell_toplevel *shoyu_shell_toplevel,
    uint32_t drm_format) {
  ShoyuShellGtkToplevel *self = SHOYU_SHELL_GTK_TOPLEVEL(data);

  GdkDmabufFormats *formats =
      gdk_display_get_dmabuf_formats(self->display->display);
  if (gdk_dmabuf_formats_contains(formats, drm_format, 0)) {
    self->drm_format = drm_format;
  } else {
    self->drm_format = GBM_FORMAT_ARGB8888;
  }
}

static void shoyu_shell_gtk_toplevel_shm_format(
    void *data, struct shoyu_shell_toplevel *shoyu_shell_toplevel,
    uint32_t shm_format) {
  ShoyuShellGtkToplevel *toplevel = SHOYU_SHELL_GTK_TOPLEVEL(data);
  toplevel->shm_format = shm_format;
}

static void shoyu_shell_gtk_toplevel_damage(
    void *data, struct shoyu_shell_toplevel *shoyu_shell_toplevel, uint32_t x,
    uint32_t y, uint32_t width, uint32_t height) {
  ShoyuShellGtkToplevel *self = SHOYU_SHELL_GTK_TOPLEVEL(data);

  GdkRectangle *rect = g_new0(GdkRectangle, 1);
  rect->x = x;
  rect->y = y;
  rect->width = width;
  rect->height = height;

  self->damage = g_list_append(self->damage, rect);
}

static void shoyu_shell_gtk_toplevel_frame(
    void *data, struct shoyu_shell_toplevel *shoyu_shell_toplevel,
    uint32_t width, uint32_t height) {
  ShoyuShellGtkToplevel *self = SHOYU_SHELL_GTK_TOPLEVEL(data);

  if (self->drm_format > 0) {
    g_assert(self->display->zwp_linux_dmabuf_v1 != NULL);

    GList *damage = self->damage;
    self->damage = NULL;

    gboolean needs_updating = FALSE;
    if (self->gbm_bo != NULL) {
      needs_updating = gbm_bo_get_width(self->gbm_bo) != width ||
                       gbm_bo_get_height(self->gbm_bo) != height;
    }

    if (self->gbm_bo == NULL || needs_updating) {
      g_clear_pointer(&self->gbm_bo, gbm_bo_destroy);

      self->gbm_bo =
          gbm_bo_create(self->display->dmabuf_formats_info->gbm_device, width,
                        height, self->drm_format, GBM_BO_USE_RENDERING);
      self->gbm_bo_modifier = DRM_FORMAT_MOD_INVALID;
      g_assert(self->gbm_bo != NULL);
    }

    GdkDmabufTextureBuilder *builder = gdk_dmabuf_texture_builder_new();
    gdk_dmabuf_texture_builder_set_display(builder, self->display->display);
    gdk_dmabuf_texture_builder_set_fourcc(builder, self->drm_format);
    gdk_dmabuf_texture_builder_set_width(builder, width);
    gdk_dmabuf_texture_builder_set_height(builder, height);
    gdk_dmabuf_texture_builder_set_update_texture(builder, self->texture);

    if (damage != NULL) {
      cairo_region_t *regions = cairo_region_create();
      for (GList *item = damage; item != NULL; item = item->next) {
        GdkRectangle *rect = item->data;
        cairo_region_union_rectangle(regions, &(cairo_rectangle_int_t){
                                                .x = rect->x,
                                                .y = rect->y,
                                                .width = rect->width,
                                                .height = rect->height,
                                              });
      }

      gdk_dmabuf_texture_builder_set_update_region(builder, regions);
    }

    struct zwp_linux_buffer_params_v1 *params =
        zwp_linux_dmabuf_v1_create_params(self->display->zwp_linux_dmabuf_v1);

    int n_planes = gbm_bo_get_plane_count(self->gbm_bo);
    gdk_dmabuf_texture_builder_set_n_planes(builder, n_planes);
    for (int i = 0; i < n_planes; i++) {
      int fd = gbm_bo_get_fd_for_plane(self->gbm_bo, i);
      int offset = gbm_bo_get_offset(self->gbm_bo, i);
      int stride = gbm_bo_get_stride_for_plane(self->gbm_bo, i);

      gdk_dmabuf_texture_builder_set_fd(builder, i, fd);
      gdk_dmabuf_texture_builder_set_offset(builder, i, offset);
      gdk_dmabuf_texture_builder_set_stride(builder, i, stride);

      zwp_linux_buffer_params_v1_add(params, fd, i, offset, stride,
                                     self->gbm_bo_modifier >> 32,
                                     self->gbm_bo_modifier & 0xffffffff);
    }

    struct wl_display *wl_display =
        gdk_wayland_display_get_wl_display(self->display->display);
    struct wl_event_queue *eq = wl_display_create_queue(wl_display);
    wl_proxy_set_queue((struct wl_proxy *)params, eq);

    struct CreateBufferData cbd = {NULL, FALSE};

    zwp_linux_buffer_params_v1_add_listener(
        params, &zwp_linux_buffer_params_v1_listener, &cbd);
    zwp_linux_buffer_params_v1_create(params, width, height, self->drm_format,
                                      0);

    while (!cbd.done) {
      wl_display_dispatch_queue(wl_display, eq);
    }

    zwp_linux_buffer_params_v1_destroy(params);
    wl_event_queue_destroy(eq);

    struct shoyu_shell_capture *capture =
        shoyu_shell_toplevel_capture(self->shoyu_shell_toplevel, cbd.buffer);
    eq = wl_display_create_queue(wl_display);
    wl_proxy_set_queue((struct wl_proxy *)capture, eq);

    gboolean capture_done = FALSE;

    shoyu_shell_capture_add_listener(capture, &shoyu_shell_capture_listener,
                                     &capture_done);
    shoyu_shell_capture_capture(capture);

    while (!capture_done) {
      wl_display_dispatch_queue(wl_display, eq);
    }

    shoyu_shell_capture_destroy(capture);
    wl_buffer_destroy(cbd.buffer);
    wl_event_queue_destroy(eq);
    g_signal_emit(self, shoyu_shell_gtk_toplevel_sigs[SIG_FRAME], 0);

    GError *error = NULL;
    g_clear_object(&self->texture);
    self->texture =
        gdk_dmabuf_texture_builder_build(builder, NULL, NULL, &error);
    g_clear_object(&builder);
    if (self->texture == NULL) {
      g_warning("Failed to create texture: %s", error->message);
      return;
    }

    g_object_notify_by_pspec(G_OBJECT(self),
                             shoyu_shell_gtk_toplevel_props[PROP_TEXTURE]);

    g_clear_list(&damage, (GDestroyNotify)g_free);
  }
}

static void shoyu_shell_gtk_toplevel_configure(
    void *data, struct shoyu_shell_toplevel *shoyu_shell_toplevel, uint32_t x,
    uint32_t y, uint32_t width, uint32_t height) {
  ShoyuShellGtkToplevel *toplevel = SHOYU_SHELL_GTK_TOPLEVEL(data);
  g_signal_emit(toplevel, shoyu_shell_gtk_toplevel_sigs[SIG_CONFIGURE], 0, x, y,
                width, height);
}

static void shoyu_shell_gtk_toplevel_set_title(
    void *data, struct shoyu_shell_toplevel *shoyu_shell_toplevel,
    const char *title) {
  ShoyuShellGtkToplevel *self = SHOYU_SHELL_GTK_TOPLEVEL(data);

  g_clear_pointer(&self->title, (GDestroyNotify)g_free);
  self->title = g_strdup(title);
  g_object_notify_by_pspec(G_OBJECT(self),
                           shoyu_shell_gtk_toplevel_props[PROP_TITLE]);
}

static void shoyu_shell_gtk_toplevel_set_app_id(
    void *data, struct shoyu_shell_toplevel *shoyu_shell_toplevel,
    const char *app_id) {
  ShoyuShellGtkToplevel *self = SHOYU_SHELL_GTK_TOPLEVEL(data);

  g_clear_pointer(&self->app_id, (GDestroyNotify)g_free);
  self->app_id = g_strdup(app_id);
  g_object_notify_by_pspec(G_OBJECT(self),
                           shoyu_shell_gtk_toplevel_props[PROP_APP_ID]);
}

static void shoyu_shell_gtk_toplevel_destroy(
    void *data, struct shoyu_shell_toplevel *shoyu_shell_toplevel) {
  ShoyuShellGtkToplevel *toplevel = SHOYU_SHELL_GTK_TOPLEVEL(data);

  if (!toplevel->is_invalidated) {
    shoyu_shell_gtk_toplevel_unrealize(toplevel);
    g_signal_emit(toplevel, shoyu_shell_gtk_toplevel_sigs[SIG_DESTROY], 0);
  }
}

static const struct shoyu_shell_toplevel_listener
    shoyu_shell_toplevel_listener = {
      .drm_format = shoyu_shell_gtk_toplevel_drm_format,
      .shm_format = shoyu_shell_gtk_toplevel_shm_format,
      .damage = shoyu_shell_gtk_toplevel_damage,
      .frame = shoyu_shell_gtk_toplevel_frame,
      .configure = shoyu_shell_gtk_toplevel_configure,
      .set_title = shoyu_shell_gtk_toplevel_set_title,
      .set_app_id = shoyu_shell_gtk_toplevel_set_app_id,
      .destroy = shoyu_shell_gtk_toplevel_destroy,
};

static void shoyu_shell_gtk_toplevel_finalize(GObject *object) {
  ShoyuShellGtkToplevel *self = SHOYU_SHELL_GTK_TOPLEVEL(object);

  g_clear_object(&self->display);
  g_clear_list(&self->damage, (GDestroyNotify)g_free);
  g_clear_pointer(&self->title, (GDestroyNotify)g_free);
  g_clear_pointer(&self->app_id, (GDestroyNotify)g_free);

  G_OBJECT_CLASS(shoyu_shell_gtk_toplevel_parent_class)->finalize(object);
}

static void shoyu_shell_gtk_toplevel_set_property(GObject *object,
                                                  guint prop_id,
                                                  const GValue *value,
                                                  GParamSpec *pspec) {
  ShoyuShellGtkToplevel *self = SHOYU_SHELL_GTK_TOPLEVEL(object);

  switch (prop_id) {
    case PROP_DISPLAY:
      self->display = g_value_dup_object(value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static void shoyu_shell_gtk_toplevel_get_property(GObject *object,
                                                  guint prop_id, GValue *value,
                                                  GParamSpec *pspec) {
  ShoyuShellGtkToplevel *self = SHOYU_SHELL_GTK_TOPLEVEL(object);

  switch (prop_id) {
    case PROP_DISPLAY:
      g_value_set_object(value, G_OBJECT(self->display));
      break;
    case PROP_TEXTURE:
      g_value_set_object(value, G_OBJECT(self->texture));
      break;
    case PROP_TITLE:
      g_value_set_string(value, self->title);
      break;
    case PROP_APP_ID:
      g_value_set_string(value, self->app_id);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static void
shoyu_shell_gtk_toplevel_class_init(ShoyuShellGtkToplevelClass *class) {
  GObjectClass *object_class = G_OBJECT_CLASS(class);

  object_class->finalize = shoyu_shell_gtk_toplevel_finalize;
  object_class->set_property = shoyu_shell_gtk_toplevel_set_property;
  object_class->get_property = shoyu_shell_gtk_toplevel_get_property;

  shoyu_shell_gtk_toplevel_props[PROP_DISPLAY] = g_param_spec_object(
      "display", "Shoyu Shell Display",
      "The display the shell is connected to.", SHOYU_SHELL_GTK_TYPE_DISPLAY,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);

  shoyu_shell_gtk_toplevel_props[PROP_TEXTURE] = g_param_spec_object(
      "texture", "Gdk Texture",
      "The texture which contains the contents of the window", GDK_TYPE_TEXTURE,
      G_PARAM_READABLE);

  shoyu_shell_gtk_toplevel_props[PROP_TITLE] =
      g_param_spec_string("title", "Window title", "The title of the window",
                          NULL, G_PARAM_READABLE);

  shoyu_shell_gtk_toplevel_props[PROP_APP_ID] = g_param_spec_string(
      "app-id", "Application ID", "The app ID associated with the window", NULL,
      G_PARAM_READABLE);

  g_object_class_install_properties(object_class, N_PROPERTIES,
                                    shoyu_shell_gtk_toplevel_props);

  /**
   * ShoyuShellGtkToplevel::destroy:
   * @toplevel: the object which received the signal
   */
  shoyu_shell_gtk_toplevel_sigs[SIG_DESTROY] =
      g_signal_new("destroy", SHOYU_SHELL_GTK_TYPE_TOPLEVEL, G_SIGNAL_RUN_LAST,
                   0, NULL, NULL, NULL, G_TYPE_NONE, 0);

  /**
   * ShoyuShellGtkToplevel::configure:
   * @toplevel: the object which received the signal
   * @x: the x position
   * @y: the y position
   * @width: the width of the window
   * @height: the height of the window
   */
  shoyu_shell_gtk_toplevel_sigs[SIG_CONFIGURE] =
      g_signal_new("configure", SHOYU_SHELL_GTK_TYPE_TOPLEVEL,
                   G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL, G_TYPE_NONE, 4,
                   G_TYPE_UINT, G_TYPE_UINT, G_TYPE_UINT, G_TYPE_UINT);

  /**
   * ShoyuShellGtkToplevel::realized:
   * @toplevel: a #ShoyuShellGtkToplevel
   * @shoyu_shell_toplevel: A Shoyu Shell toplevel
   */
  shoyu_shell_gtk_toplevel_sigs[SIG_REALIZED] =
      g_signal_new("realized", SHOYU_SHELL_GTK_TYPE_TOPLEVEL, G_SIGNAL_RUN_LAST,
                   G_STRUCT_OFFSET(ShoyuShellGtkToplevelClass, realized), NULL,
                   NULL, NULL, G_TYPE_NONE, 1, G_TYPE_POINTER);

  /**
   * ShoyuShellGtkToplevel::unrealized:
   * @toplevel: a #ShoyuShellGtkToplevel
   */
  shoyu_shell_gtk_toplevel_sigs[SIG_UNREALIZED] = g_signal_new(
      "unrealized", SHOYU_SHELL_GTK_TYPE_TOPLEVEL, G_SIGNAL_RUN_LAST,
      G_STRUCT_OFFSET(ShoyuShellGtkToplevelClass, unrealized), NULL, NULL, NULL,
      G_TYPE_NONE, 0);

  /**
   * ShoyuShellGtkToplevel::frame:
   * @toplevel: a #ShoyuShellGtkToplevel
   */
  shoyu_shell_gtk_toplevel_sigs[SIG_FRAME] =
      g_signal_new("frame", SHOYU_SHELL_GTK_TYPE_TOPLEVEL, G_SIGNAL_RUN_LAST, 0,
                   NULL, NULL, NULL, G_TYPE_NONE, 0);
}

static void shoyu_shell_gtk_toplevel_init(ShoyuShellGtkToplevel *self) {
  self->is_invalidated = TRUE;
  self->texture = NULL;
  self->drm_format = 0;
  self->shm_format = 0;
}

void shoyu_shell_gtk_toplevel_realize(
    ShoyuShellGtkToplevel *self,
    struct shoyu_shell_toplevel *shoyu_shell_toplevel) {
  g_return_if_fail(SHOYU_SHELL_GTK_IS_TOPLEVEL(self));
  g_return_if_fail(self->shoyu_shell_toplevel == NULL && self->is_invalidated);

  self->shoyu_shell_toplevel = shoyu_shell_toplevel;
  self->is_invalidated = FALSE;

  shoyu_shell_toplevel_add_listener(self->shoyu_shell_toplevel,
                                    &shoyu_shell_toplevel_listener, self);

  g_signal_emit(self, shoyu_shell_gtk_toplevel_sigs[SIG_REALIZED], 0,
                shoyu_shell_toplevel);
}

void shoyu_shell_gtk_toplevel_unrealize(ShoyuShellGtkToplevel *self) {
  g_return_if_fail(SHOYU_SHELL_GTK_IS_TOPLEVEL(self));
  g_return_if_fail(self->shoyu_shell_toplevel != NULL && !self->is_invalidated);

  self->shoyu_shell_toplevel = NULL;
  self->is_invalidated = TRUE;

  g_signal_emit(self, shoyu_shell_gtk_toplevel_sigs[SIG_UNREALIZED], 0);
}

/**
 * shoyu_shell_gtk_toplevel_set_geometry:
 * @self: A #ShoyuShellGtkToplevel
 * @x: The x position
 * @y: The y position
 * @width: The width of the window
 * @height: The height of the window
 */
void shoyu_shell_gtk_toplevel_set_geometry(ShoyuShellGtkToplevel *self,
                                           uint32_t x, uint32_t y,
                                           uint32_t width, uint32_t height) {
  g_return_if_fail(SHOYU_SHELL_GTK_IS_TOPLEVEL(self));
  g_return_if_fail(self->shoyu_shell_toplevel != NULL && !self->is_invalidated);

  shoyu_shell_toplevel_set_geometry(self->shoyu_shell_toplevel, x, y, width,
                                    height);
}
