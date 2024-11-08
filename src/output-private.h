#pragma once

#include <shoyu/output.h>

typedef struct _ShoyuOutputPrivate {
  ShoyuOutput* self;

  ShoyuCompositor* compositor;
  struct wlr_output* wlr_output;
  GtkWidget* view;

  gboolean is_destroying;

  struct wl_listener destroy;
  struct wl_listener frame;
} ShoyuOutputPrivate;

#define SHOYU_OUTPUT(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), shoyu_output_get_type(), \
                              ShoyuOutput))

#define SHOYU_OUTPUT_GET_PRIVATE(app)                        \
  ((ShoyuOutputPrivate*)shoyu_output_get_instance_private( \
      SHOYU_OUTPUT(app)))
