#include "shell-toplevel-private.h"
#include <shoyu-shell-server-protocol.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/util/addon.h>

typedef struct {
    struct wl_resource *resource;
    struct wlr_addon addon;
    struct wlr_xdg_toplevel *wlr_xdg_toplevel;
    ShoyuShell *shell;
} ShellToplevel;

static const struct shoyu_shell_toplevel_interface shoyu_shell_toplevel_impl =
    {};

static void shoyu_shell_toplevel_destroy(ShellToplevel *self) {
  if (self == NULL)
    return;

  wlr_addon_finish(&self->addon);
  wl_resource_set_user_data(self->resource, NULL);
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
