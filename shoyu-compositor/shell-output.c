#include "compositor-private.h"
#include "output-private.h"
#include "shell-output-private.h"
#include "shell-private.h"

#include <shoyu-shell-server-protocol.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_output.h>
#include <wlr/util/addon.h>

typedef struct {
    struct wl_resource *resource;
    struct wlr_addon addon;
    struct wlr_output *wlr_output;
    ShoyuShell *shell;
} ShellOutput;

static void shoyu_shell_output_set_surface(struct wl_client *wl_client,
                                           struct wl_resource *resource,
                                           struct wl_resource *surface) {
  ShellOutput *self = wl_resource_get_user_data(resource);
  struct wlr_surface *wlr_surface = wlr_surface_from_resource(surface);

  ShoyuOutput *output =
      shoyu_compositor_get_output(self->shell->compositor, self->wlr_output);
  g_return_if_fail(output != NULL);

  shoyu_output_set_surface(output, wlr_surface);
}

static const struct shoyu_shell_output_interface shoyu_shell_output_impl = {
  .set_surface = shoyu_shell_output_set_surface,
};

static void shoyu_shell_output_destroy(ShellOutput *self) {
  if (self == NULL)
    return;

  wlr_addon_finish(&self->addon);
  wl_resource_set_user_data(self->resource, NULL);
  free(self);
}

static void shoyu_shell_output_resource_destroy(struct wl_resource *resource) {
  ShellOutput *self = wl_resource_get_user_data(resource);
  shoyu_shell_output_destroy(self);
}

static void shoyu_shell_output_addon_destroy(struct wlr_addon *addon) {
  ShellOutput *self = wl_container_of(addon, self, addon);
  shoyu_shell_output_destroy(self);
}

static const struct wlr_addon_interface shoyu_shell_output_addon_impl = {
  .name = "shoyu_shell_output",
  .destroy = shoyu_shell_output_addon_destroy,
};

void shoyu_shell_get_output(struct wl_client *wl_client,
                            struct wl_resource *shell_resource, uint32_t id,
                            struct wl_resource *output) {
  ShoyuShell *shell = wl_resource_get_user_data(shell_resource);
  struct wlr_output *wlr_output = wlr_output_from_resource(output);

  if (wlr_addon_find(&wlr_output->addons, shell,
                     &shoyu_shell_output_addon_impl) != NULL) {
    wl_resource_post_error(
        shell_resource, SHOYU_SHELL_ERROR_OUTPUT_ALREADY_CONSTRUCTED,
        "shoyu_shell_output already constructed for this output");
    return;
  }

  ShellOutput *shell_output = malloc(sizeof(ShellOutput));
  if (shell_output == NULL) {
    wl_resource_post_no_memory(shell_resource);
    return;
  }

  shell_output->wlr_output = wlr_output;
  shell_output->shell = SHOYU_SHELL(g_object_ref(shell));

  uint32_t version = wl_resource_get_version(shell_resource);
  shell_output->resource =
      wl_resource_create(wl_client, &shoyu_shell_output_interface, version, id);
  if (shell_output == NULL) {
    g_object_unref(shell_output->shell);
    free(shell_output);
    wl_resource_post_no_memory(shell_resource);
    return;
  }

  wl_resource_set_implementation(shell_output->resource,
                                 &shoyu_shell_output_impl, shell_output,
                                 shoyu_shell_output_resource_destroy);
  wlr_addon_init(&shell_output->addon, &wlr_output->addons, shell,
                 &shoyu_shell_output_addon_impl);
}
