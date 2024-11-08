#pragma once

#include <glib-object.h>
#include <gio/gio.h>
#include <gtk/gtk.h>
#include <wlr/backend.h>
#include <wayland-server-core.h>

#include "output.h"

G_BEGIN_DECLS;

G_DECLARE_DERIVABLE_TYPE(ShoyuCompositor, shoyu_compositor, SHOYU, COMPOSITOR, GObject);

struct _ShoyuCompositorClass {
  GObjectClass parent_class;

  /**
   * ShoyuCompositorClass:create_output:
   *
   * Returns: (transfer full) (nullable): a #ShoyuOutput
   */
  ShoyuOutput* (*create_output)(ShoyuCompositor* compositor, struct wlr_output* wlr_output);

  /*< private >*/
  gpointer padding[12];
};

struct wlr_backend* shoyu_compositor_get_wlr_backend(ShoyuCompositor* self);

ShoyuCompositor* shoyu_compositor_new(struct wl_display* wl_display);
ShoyuCompositor* shoyu_compositor_new_with_application(struct wl_display* wl_display, GApplication* application);
ShoyuCompositor* shoyu_compositor_new_with_wlr_backend(struct wl_display* wl_display, struct wlr_backend* wlr_backend);
ShoyuCompositor* shoyu_compositor_new_with_wlr_backend_with_application(struct wl_display* wl_display, struct wlr_backend* wlr_backend, GApplication* application);

GList* shoyu_compositor_get_outputs(ShoyuCompositor* self);

G_END_DECLS;
