#pragma once

#include "output.h"

#include <wlr/types/wlr_output.h>

struct _ShoyuOutput {
  GObject parent_instance;

  ShoyuCompositor* compositor;

  struct wlr_output* wlr_output;

  bool is_invalidated;

  struct wl_listener destroy;
  struct wl_listener frame;
  struct wl_listener request_state;
};

struct _ShoyuOutputClass {
  GObjectClass parent_class;

  void (*realized)(ShoyuOutput* self, struct wlr_output* wlr_output);
  void (*unrealized)(ShoyuOutput* self);
};

void shoyu_output_realize(ShoyuOutput* self, struct wlr_output* wlr_output);
void shoyu_output_unrealize(ShoyuOutput* self);
