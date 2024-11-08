#pragma once

#include <shoyu/compositor.h>

typedef struct _ShoyuCompositorPrivate {
  ShoyuCompositor* self;

  GApplication* application;
  GList* outputs;

  struct wl_display* wl_display;
  struct wlr_backend* wlr_backend;
  struct wlr_renderer* wlr_renderer;

  struct wlr_allocator* allocator;
  struct wlr_compositor* compositor;

  struct wl_listener new_output;
} ShoyuCompositorPrivate;

#define SHOYU_COMPOSITOR(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), shoyu_compositor_get_type(), \
                              ShoyuCompositor))

#define SHOYU_COMPOSITOR_GET_PRIVATE(app)                        \
  ((ShoyuCompositorPrivate*)shoyu_compositor_get_instance_private( \
      SHOYU_COMPOSITOR(app)))

struct wlr_renderer* shoyu_compositor_get_wlr_renderer(ShoyuCompositor* self);
