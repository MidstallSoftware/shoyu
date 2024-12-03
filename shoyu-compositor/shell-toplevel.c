#include "compositor-private.h"
#include "shell-private.h"
#include "shell-toplevel-private.h"
#include <shoyu-shell-server-protocol.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/util/addon.h>

typedef struct {
    struct wl_resource *resource;
    gboolean has_addon_destroyed;
    struct wlr_addon addon;
    struct wlr_xdg_toplevel *wlr_xdg_toplevel;
    struct wl_listener surface_commit;
    struct wl_listener surface_destroy;
    struct wl_listener xdg_toplevel_destroy;
    ShoyuShell *shell;
} ShellToplevel;

typedef struct {
    ShellToplevel *toplevel;
    struct wlr_buffer *wlr_buffer;
    struct wl_resource *resource;
} ShellCapture;

static void shoyu_shell_toplevel_capture_capture(struct wl_client *client,
                                                 struct wl_resource *resource) {
  ShellCapture *self = wl_resource_get_user_data(resource);

  struct wlr_texture *texture =
      wlr_surface_get_texture(self->toplevel->wlr_xdg_toplevel->base->surface);
  if (!texture) {
    // TODO: post error
    return;
  }

  struct wlr_render_pass *pass = wlr_renderer_begin_buffer_pass(
      self->toplevel->shell->compositor->wlr_renderer, self->wlr_buffer, NULL);
  if (!pass) {
    // TODO: post error
    return;
  }

  wlr_render_pass_add_texture(pass, &(struct wlr_render_texture_options){
                                      .texture = texture,
                                      .blend_mode = WLR_RENDER_BLEND_MODE_NONE,
                                      .dst_box = {0, 0},
                                    });

  if (!wlr_render_pass_submit(pass)) {
    // TODO: post error
    return;
  }

  shoyu_shell_capture_send_done(resource);
}

static const struct shoyu_shell_capture_interface shoyu_shell_capture_impl = {
  .capture = shoyu_shell_toplevel_capture_capture,
};

static void shoyu_shell_toplevel_capture_destroy(struct wl_resource *resource) {
  ShellCapture *self = wl_resource_get_user_data(resource);
  free(self);
}

static void shoyu_shell_toplevel_capture(struct wl_client *client,
                                         struct wl_resource *resource,
                                         uint32_t capture_id,
                                         struct wl_resource *buffer_resource) {
  ShellToplevel *self = wl_resource_get_user_data(resource);

  ShellCapture *capture = g_new0(ShellCapture, 1);

  capture->toplevel = self;
  capture->wlr_buffer = wlr_buffer_try_from_resource(buffer_resource);
  if (!capture->wlr_buffer) {
    g_free(capture);
    // TODO: post error
    return;
  }

  capture->resource =
      wl_resource_create(client, &shoyu_shell_capture_interface,
                         wl_resource_get_version(resource), capture_id);
  wl_resource_set_implementation(capture->resource, &shoyu_shell_capture_impl,
                                 capture, shoyu_shell_toplevel_capture_destroy);
}

static const struct shoyu_shell_toplevel_interface shoyu_shell_toplevel_impl = {
  .capture = shoyu_shell_toplevel_capture,
};

static void shoyu_shell_toplevel_destroy(ShellToplevel *self) {
  if (self == NULL)
    return;

  if (!self->has_addon_destroyed) {
    wlr_addon_finish(&self->addon);
    self->has_addon_destroyed = TRUE;
  }

  wl_resource_set_user_data(self->resource, NULL);
  wl_list_remove(&self->surface_commit.link);
  wl_list_remove(&self->surface_destroy.link);
  wl_list_remove(&self->xdg_toplevel_destroy.link);
  shoyu_shell_toplevel_send_destroy(self->resource);
  free(self);
}

static void
shoyu_shell_toplevel_resource_destroy(struct wl_resource *resource) {
  ShellToplevel *self = wl_resource_get_user_data(resource);
  shoyu_shell_toplevel_destroy(self);
}

static void shoyu_shell_toplevel_addon_destroy(struct wlr_addon *addon) {
  ShellToplevel *self = wl_container_of(addon, self, addon);
  shoyu_shell_toplevel_destroy(self);
}

static const struct wlr_addon_interface shoyu_shell_toplevel_addon_impl = {
  .name = "shoyu_shell_toplevel",
  .destroy = shoyu_shell_toplevel_addon_destroy,
};

static void shoyu_shell_toplevel_surface_commit(struct wl_listener *listener,
                                                void *data) {
  ShellToplevel *self = wl_container_of(listener, self, surface_commit);

  if (wlr_surface_has_buffer(self->wlr_xdg_toplevel->base->surface)) {
    struct wlr_buffer *wlr_buffer =
        &self->wlr_xdg_toplevel->base->surface->buffer->base;

    struct wlr_dmabuf_attributes dmabuf_attribs;
    struct wlr_shm_attributes shm_attribs;
    if (wlr_buffer_get_dmabuf(wlr_buffer, &dmabuf_attribs)) {
      shoyu_shell_toplevel_send_drm_format(self->resource,
                                           dmabuf_attribs.format);
    } else if (wlr_buffer_get_shm(wlr_buffer, &shm_attribs)) {
      shoyu_shell_toplevel_send_shm_format(self->resource, shm_attribs.format);
    } else
      return;

    shoyu_shell_toplevel_send_frame(self->resource, wlr_buffer->width,
                                    wlr_buffer->height);
  }
}

static void shoyu_shell_toplevel_surface_destroy(struct wl_listener *listener,
                                                 void *data) {
  ShellToplevel *self = wl_container_of(listener, self, surface_destroy);
  shoyu_shell_toplevel_destroy(self);
}

static void
shoyu_shell_toplevel_xdg_toplevel_destroy(struct wl_listener *listener,
                                          void *data) {
  ShellToplevel *self = wl_container_of(listener, self, xdg_toplevel_destroy);
  shoyu_shell_toplevel_destroy(self);
}

void shoyu_shell_toplevel_create(struct wl_client *wl_client,
                                 struct wl_resource *shell_resource,
                                 struct wlr_xdg_toplevel *xdg_toplevel) {
  ShoyuShell *shell = wl_resource_get_user_data(shell_resource);

  if (wlr_addon_find(&xdg_toplevel->base->surface->addons, shell,
                     &shoyu_shell_toplevel_addon_impl) != NULL) {
    wl_resource_post_error(
        shell_resource, SHOYU_SHELL_ERROR_TOPLEVEL_ALREADY_CONSTRUCTED,
        "shoyu_shell_toplevel already constructed for this xdg_toplevel");
    return;
  }

  ShellToplevel *shell_toplevel = malloc(sizeof(ShellToplevel));
  if (shell_toplevel == NULL) {
    wl_resource_post_no_memory(shell_resource);
    return;
  }

  shell_toplevel->wlr_xdg_toplevel = xdg_toplevel;
  shell_toplevel->shell = SHOYU_SHELL(g_object_ref(shell));

  uint32_t version = wl_resource_get_version(shell_resource);
  shell_toplevel->resource = wl_resource_create(
      wl_client, &shoyu_shell_toplevel_interface, version, 0);
  if (shell_toplevel == NULL) {
    g_object_unref(shell_toplevel->shell);
    free(shell_toplevel);
    wl_resource_post_no_memory(shell_resource);
    return;
  }

  shell_toplevel->surface_commit.notify = shoyu_shell_toplevel_surface_commit;
  wl_signal_add(&shell_toplevel->wlr_xdg_toplevel->base->surface->events.commit,
                &shell_toplevel->surface_commit);

  shell_toplevel->surface_destroy.notify = shoyu_shell_toplevel_surface_destroy;
  wl_signal_add(
      &shell_toplevel->wlr_xdg_toplevel->base->surface->events.destroy,
      &shell_toplevel->surface_destroy);

  shell_toplevel->xdg_toplevel_destroy.notify =
      shoyu_shell_toplevel_xdg_toplevel_destroy;
  wl_signal_add(&shell_toplevel->wlr_xdg_toplevel->events.destroy,
                &shell_toplevel->xdg_toplevel_destroy);

  wl_resource_set_implementation(shell_toplevel->resource,
                                 &shoyu_shell_toplevel_impl, shell_toplevel,
                                 shoyu_shell_toplevel_resource_destroy);
  wlr_addon_init(&shell_toplevel->addon, &xdg_toplevel->base->surface->addons,
                 shell, &shoyu_shell_toplevel_addon_impl);

  shoyu_shell_send_toplevel_added(shell_resource, shell_toplevel->resource);
}

void shoyu_shell_toplevel_delete(ShoyuShell *shell,
                                 struct wlr_xdg_toplevel *xdg_toplevel) {
  struct wlr_addon *addon =
      wlr_addon_find(&xdg_toplevel->base->surface->addons, shell,
                     &shoyu_shell_toplevel_addon_impl);
  g_return_if_fail(addon != NULL);
  shoyu_shell_toplevel_addon_destroy(addon);
}
