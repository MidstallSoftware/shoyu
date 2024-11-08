#pragma once

#include <shoyu/application.h>

typedef struct _ShoyuApplicationPrivate {
  ShoyuCompositor* compositor;
  struct wl_display* wl_display;

  GSource* wl_source;
} ShoyuApplicationPrivate;

#define SHOYU_APPLICATION(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), shoyu_application_get_type(), \
                              ShoyuApplication))

#define SHOYU_APPLICATION_GET_PRIVATE(app)                        \
  ((ShoyuApplicationPrivate*)shoyu_application_get_instance_private( \
      SHOYU_APPLICATION(app)))
