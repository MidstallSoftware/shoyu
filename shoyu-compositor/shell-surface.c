#include "shell-surface-private.h"
#include <shoyu-shell-server-protocol.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/util/addon.h>

typedef struct {
  struct wl_resource* resource;
  struct wlr_addon addon;
  struct wlr_surface* wlr_surface;
} ShellSurface;

static const struct shoyu_shell_surface_interface shoyu_shell_surface_impl = {};

static void shoyu_shell_surface_destroy(ShellSurface* self) {
  if (self == NULL) return;

  wlr_addon_finish(&self->addon);
  wl_resource_set_user_data(self->resource, NULL);
  free(self);
}

static void shoyu_shell_surface_resource_destroy(struct wl_resource* resource) {
  ShellSurface* self = wl_resource_get_user_data(resource);
  shoyu_shell_surface_destroy(self);
}

static void shoyu_shell_surface_addon_destroy(struct wlr_addon* addon) {
  ShellSurface* self = wl_container_of(addon, self, addon);
  shoyu_shell_surface_destroy(self);
}

static const struct wlr_addon_interface shoyu_shell_surface_addon_impl = {
  .name = "shoyu_shell_surface",
  .destroy = shoyu_shell_surface_addon_destroy,
};

void shoyu_shell_get_surface(struct wl_client* wl_client, struct wl_resource* shell_resource, uint32_t id, struct wl_resource* surface) {
  ShoyuShell* shell = wl_resource_get_user_data(shell_resource);
  struct wlr_surface* wlr_surface = wlr_surface_from_resource(surface);

  if (wlr_addon_find(&wlr_surface->addons, shell, &shoyu_shell_surface_addon_impl) != NULL) {
    wl_resource_post_error(shell_resource, SHOYU_SHELL_ERROR_SURFACE_ALREADY_CONSTRUCTED, "shoyu_shell_surface already constructed for this surface");
    return;
  }

  ShellSurface* shell_surface = malloc(sizeof (ShellSurface));
  if (shell_surface == NULL) {
    wl_resource_post_no_memory(shell_resource);
    return;
  }

  shell_surface->wlr_surface = wlr_surface;

  uint32_t version = wl_resource_get_version(shell_resource);
  shell_surface->resource = wl_resource_create(wl_client, &shoyu_shell_surface_interface, version, id);
  if (shell_surface == NULL) {
    free(shell_surface);
    wl_resource_post_no_memory(shell_resource);
    return;
  }

  wl_resource_set_implementation(shell_surface->resource, &shoyu_shell_surface_impl, shell_surface, shoyu_shell_surface_resource_destroy);
  wlr_addon_init(&shell_surface->addon, &wlr_surface->addons, shell, &shoyu_shell_surface_addon_impl);
}
