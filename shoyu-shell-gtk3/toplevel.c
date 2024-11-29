#include "display-private.h"
#include "toplevel-private.h"
#include <drm_fourcc.h>

enum {
  PROP_0 = 0,
  PROP_DISPLAY,
  PROP_GL_CONTEXT,
  PROP_SURFACE,
  PROP_TEXTURE_ID,
  N_PROPERTIES,

  SIG_DESTROY = 0,
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

static void shoyu_shell_gtk_toplevel_drm_format(
    void *data, struct shoyu_shell_toplevel *shoyu_shell_toplevel,
    uint32_t drm_format) {
  ShoyuShellGtkToplevel *self = SHOYU_SHELL_GTK_TOPLEVEL(data);

  gboolean use_bo_map = self->gl_context != NULL
                            ? !gdk_gl_context_get_use_es(self->gl_context)
                            : TRUE;
  if (use_bo_map) {
    self->drm_format = DRM_FORMAT_ARGB8888;
  } else {
    self->drm_format = drm_format;
  }
}

static void shoyu_shell_gtk_toplevel_shm_format(
    void *data, struct shoyu_shell_toplevel *shoyu_shell_toplevel,
    uint32_t shm_format) {
  ShoyuShellGtkToplevel *toplevel = SHOYU_SHELL_GTK_TOPLEVEL(data);
  toplevel->shm_format = shm_format;
}

static void shoyu_shell_gtk_toplevel_frame(
    void *data, struct shoyu_shell_toplevel *shoyu_shell_toplevel,
    uint32_t width, uint32_t height) {
  ShoyuShellGtkToplevel *self = SHOYU_SHELL_GTK_TOPLEVEL(data);

  if (self->drm_format > 0) {
    g_assert(self->display->zwp_linux_dmabuf_v1 != NULL);

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

    struct zwp_linux_buffer_params_v1 *params =
        zwp_linux_dmabuf_v1_create_params(self->display->zwp_linux_dmabuf_v1);

    int n_planes = gbm_bo_get_plane_count(self->gbm_bo);
    for (int i = 0; i < n_planes; i++) {
      int fd = gbm_bo_get_fd_for_plane(self->gbm_bo, i);
      int offset = gbm_bo_get_offset(self->gbm_bo, i);
      int stride = gbm_bo_get_stride_for_plane(self->gbm_bo, i);

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

    shoyu_shell_toplevel_capture(self->shoyu_shell_toplevel, cbd.buffer);
    wl_display_roundtrip(wl_display);
    g_signal_emit(self, shoyu_shell_gtk_toplevel_sigs[SIG_FRAME], 0);

    gboolean use_bo_map = self->gl_context != NULL
                              ? !gdk_gl_context_get_use_es(self->gl_context)
                              : TRUE;

    if (use_bo_map) {
      uint32_t stride;
      void *bo_map = NULL;
      void *bo_data = gbm_bo_map(self->gbm_bo, 0, 0, width, height,
                                 GBM_BO_TRANSFER_READ, &stride, &bo_map);
      g_assert(bo_data != NULL);

      if (self->gl_context != NULL) {
        gdk_gl_context_make_current(self->gl_context);

        if (self->texture_id == 0) {
          glGenTextures(1, &self->texture_id);
        }

        glBindTexture(GL_TEXTURE_2D, self->texture_id);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, bo_data);

        gdk_gl_context_clear_current();
      } else {
        if (self->cairo_surface == NULL || needs_updating) {
          g_clear_pointer(&self->cairo_surface, cairo_surface_destroy);
          self->cairo_surface =
              cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
          g_assert(cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, width) ==
                   stride);
        }

        cairo_surface_flush(self->cairo_surface);
        memcpy(cairo_image_surface_get_data(self->cairo_surface), bo_data,
               stride * height);
        cairo_surface_mark_dirty(self->cairo_surface);

        g_object_notify_by_pspec(G_OBJECT(self),
                                 shoyu_shell_gtk_toplevel_props[PROP_SURFACE]);
      }

      gbm_bo_unmap(self->gbm_bo, bo_map);
    }
  }
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
      .frame = shoyu_shell_gtk_toplevel_frame,
      .destroy = shoyu_shell_gtk_toplevel_destroy,
};

static void shoyu_shell_gtk_toplevel_finalize(GObject *object) {
  ShoyuShellGtkToplevel *self = SHOYU_SHELL_GTK_TOPLEVEL(object);

  g_clear_object(&self->display);
  g_clear_object(&self->gl_context);
  g_clear_pointer(&self->cairo_surface, cairo_surface_destroy);

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
    case PROP_GL_CONTEXT:
      g_clear_object(&self->gl_context);
      self->gl_context = g_value_dup_object(value);
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
    case PROP_GL_CONTEXT:
      g_value_set_object(value, G_OBJECT(self->gl_context));
      break;
    case PROP_SURFACE:
      g_value_set_boxed(value, self->cairo_surface);
      break;
    case PROP_TEXTURE_ID:
      g_value_set_uint(value, self->texture_id);
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

  shoyu_shell_gtk_toplevel_props[PROP_GL_CONTEXT] =
      g_param_spec_object("gl-context", "Gdk GL Context",
                          "The GL Context to use for rendering with a texture.",
                          GDK_TYPE_GL_CONTEXT, G_PARAM_READWRITE);

  shoyu_shell_gtk_toplevel_props[PROP_SURFACE] = g_param_spec_boxed(
      "surface", "Cairo Surface", "The cairo surface containing the contents.",
      CAIRO_GOBJECT_TYPE_SURFACE, G_PARAM_READABLE);

  shoyu_shell_gtk_toplevel_props[PROP_TEXTURE_ID] = g_param_spec_uint(
      "texture-id", "GL Texture ID", "The GL texture containing the contents.",
      0, UINT_MAX, 0, G_PARAM_READABLE);

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
